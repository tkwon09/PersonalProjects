import java.awt.Color;
import java.awt.Graphics;
import java.awt.Point;
import java.io.IOException;
import java.util.ArrayList;
import java.util.TreeSet;
import java.util.concurrent.Semaphore;
import java.lang.System;
import javax.swing.JFrame;

import com.sun.org.apache.xml.internal.resolver.helpers.Debug;

public class Graphics3D {

	private Game game;

	public Graphics3D(Game game) {
		this.game = game;
	}

	// This method takes all objects and converts the vertices into
	// world space, then into camera/screen space; this is a simplified version
	// that assumes that the camera is facing down the negative side of the z
	// axis
	// and does not pan, rotate, or change its viewing direction;
	// @params Graphics
	// @returns Graphics
	public Graphics render(Graphics graphics, String RENDER_MODE, double specularHardness, double specularIntensity, Semaphore lockLights, Semaphore lockObjects) throws InterruptedException {

		// create an ArrayList to store all Triangle3D in the world
		ArrayList<Triangle3D> AllTriangles = new ArrayList<Triangle3D>();

		// create the camera transformation matrix
		Matrix cMatrix = new Matrix(4);
		try {
			cMatrix = game.world().camera().getTransformMatrix();
		} catch (IOException e1) {
			e1.printStackTrace();
		}

		ArrayList<Object> objects = new ArrayList<Object>();

		do
		{
		}while (!lockObjects.tryAcquire());
		for (int i = 0; i < game.world().objects().size(); i++) {
			// remove all objects not in front of camera;
			Object curr = game.world().objects().get(i);
			if (curr.position().z() < -100 && curr.position().z() > -10000)
				objects.add(curr);
		}

		// this loop cycles though all objects;
		for (Object o : objects) {

			// create a new Triangle3D array to contain all of the transformed
			// triangles;
			Triangle3D[] tmesh = null;

			// tries to transform the object mesh, but if matrix multiplication
			// goes wrong, then it returns an error; this also converts the
			// object to world space;
			try {
				tmesh = o.transform();
			} catch (IOException e) {
				e.printStackTrace();
			}

			// now convert the object to view space; this requires a camera
			try {
				for (Triangle3D t : tmesh) {
					Vertex[] vertices = t.getVertices();
					for (Vertex v : vertices) {
						v.dimensions().components().mult(cMatrix);
					}
				}
			} catch (IOException e) {
				e.printStackTrace();
			}

			// add transformed Triangle3Ds into AllTriangles ArrayList if they
			// are facing front and if object is in front of camera;
			for (Triangle3D t : tmesh) {
				if (frontFacing(t)) {
					AllTriangles.add(t);
				}
			}
		}
		lockObjects.release();

		// sorts all of the Triangle3D by depth in increasing order;
		depthSort(AllTriangles);

		// adds all vertices into AllVertices ArrayList;
		ArrayList<Vertex> AllVertices = new ArrayList<Vertex>();

		for (Triangle3D t : AllTriangles) {
			for (Vertex v : t.getVertices())
				AllVertices.add(v);
		}

		// set Vertex colors based on light sources in the scene;
		computeVertexColors(AllVertices, specularHardness, specularIntensity, lockLights);

		// modifies a Vertex array of vertices into one projected into
		// 2D space; they are not yet in screen space;
		// usually this takes into account camera position, angle and
		// rotation;
		project(AllVertices);

		// GOURAUD SHADING
		if (RENDER_MODE.equals("SHADE"))
			gouraudShade(game.frame(), AllVertices, graphics);

		toScreen(AllVertices);

		// this method renders all triangles as wireframes;
		if (RENDER_MODE.equals("WIREFRAME"))
			renderWireFrame(AllVertices, graphics);

		return graphics;
	}

	// this method uses a projection formula to convert 3D triangles into
	// 2D triangles, compensating for the screen's coordinate system;
	// @params ArrayList<Triangle3D>
	// @returns ArrayList<Triangle2D>
	private void project(ArrayList<Vertex> vertices) {
		int i = 0;
		// keeps track of the iteration across vertices
		for (Vertex v : vertices) {
			int centery = game.frame().getHeight() / 2;

			// this is the distance from the camera to the projection plane
			double d = game.world().camera().dist();

			// calculate new distances based on screen ratio and viewport
			double dx = -(((d / (game.frame().getWidth() / game.frame()
					.getHeight())) + 1) * centery);
			double dy = -((d + 1) * centery);

			// projection formulas
			int sx = (int) (dx * v.x() / v.z());
			int sy = (int) (dy * v.y() / v.z());

			// preserve the depth value and color;
			Vertex vt = new Vertex(sx, sy, v.z(), v.color());

			vertices.set(i, vt);
			i++;
		}
	}

	private void toScreen(ArrayList<Vertex> vertices) {
		// get the screen info needed to move 0,0 to the center of the screen
		// and flip the y axis;
		int centerx = game.frame().getWidth() / 2;
		int centery = game.frame().getHeight() / 2;

		for (Vertex v : vertices) {
			float sx = v.x() + centerx;
			float sy = centery - v.y();
			v.set(0, sx);
			v.set(1, sy);
		}
	}

	// checks whether a given Triangle3D is facing towards the camera or not
	// and culls it if it does not;
	// @params Triangle3D
	// @returns boolean
	private boolean frontFacing(Triangle3D t) {

		// get the triangle's normal
		Vector3D tnormal = t.normal();

		// compute the see which way the face is facing in comparison
		// to the camera (located at 0,0,0)
		Vector3D cvector = new Vector3D(new Vertex(0, 0, (float) game.world().camera().position().z()), t.getVertex(0));
		double angle = tnormal.dotProduct(cvector);

		if (angle > 0)
			return false;
		return true;
	}

	// this method sorts all Triangle3D by their average z values in increasing
	// order,
	// as the camera is assumed to be facing down the negative side
	// of the z-axis at all times; assume all triangles' depths are
	// calculated;
	// @params ArrayList<Triangle3D>
	private void depthSort(ArrayList<Triangle3D> triangles) {

		TreeSet<Triangle3D> ttree = new TreeSet<Triangle3D>();

		for (Triangle3D t : triangles) {
			ttree.add(t);
		}

		// add sorted triangles to the original ArrayList
		triangles.clear();
		for (Triangle3D t : ttree) {
			triangles.add(t);
		}

	}

	// this method simply connects the dots of every three vertices;
	// @params ArrayList<Vertex>, Graphics
	private void renderWireFrame(ArrayList<Vertex> vertices, Graphics graphics) {
		// check vertices in groups of threes
		for (int i = 0; i < vertices.size() - 2; i += 3) {

			// assume that all triangles are defined in a counterclockwise
			// direction;
			Point p1 = new Point((int) vertices.get(i).x(), (int) vertices.get(
					i).y());
			Point p2 = new Point((int) vertices.get(i + 1).x(), (int) vertices
					.get(i + 1).y());
			Point p3 = new Point((int) vertices.get(i + 2).x(), (int) vertices
					.get(i + 2).y());

			graphics.setColor(vertices.get(i).color());
			graphics.drawLine(p1.x, p1.y, p2.x, p2.y);
			graphics.drawLine(p2.x, p2.y, p3.x, p3.y);
			graphics.drawLine(p3.x, p3.y, p1.x, p1.y);

		}
	}

	private void gouraudShade(JFrame frame, ArrayList<Vertex> vertices,
			Graphics graphics) {
		// check vertices in groups of threes
		for (int i = 0; i < vertices.size() - 2; i += 3) {

			// assume that all triangles are defined in a counterclockwise
			// direction;
			Vertex v1 = vertices.get(i);
			Vertex v2 = vertices.get(i + 1);
			Vertex v3 = vertices.get(i + 2);

			GouraudShader.shade(v1, v2, v3, graphics, frame);

		}
	}

	private void computeVertexColors(ArrayList<Vertex> vertices, double specularHardness, double specularIntensity, Semaphore lockLights) throws InterruptedException {
		ArrayList<Light> lights = game.world().lights();
		for (Vertex v : vertices) {

			double v_red = v.color().getRed() / 255.0;
			double v_green = v.color().getGreen() / 255.0;
			double v_blue = v.color().getBlue() / 255.0;
			
			double red_total = 0;
			double green_total = 0;
			double blue_total = 0;
			
			do
			{
			}while (!lockLights.tryAcquire());
			for (int i = 0; i < lights.size(); i++)
			{
				Light l = lights.get(i);
				
				// AMBIENT LIGHTING
				double a_red = game.world().ambient() * v_red * l.color().getRed() / 255.0;
				double a_green = game.world().ambient() * v_green * l.color().getGreen() / 255.0;
				double a_blue = game.world().ambient() * v_blue * l.color().getBlue() / 255.0;
				
				// DIFFUSE LIGHTING
				Vector3D vertexToLight = new Vector3D(v, l.position()).normal();
			    double diffuseFactor = Vector3D.dotProduct(v.normal(), vertexToLight); //DiffuseFactor ranges from 0 to 1
			    if (diffuseFactor < 0)
			    	diffuseFactor = 0; //Surfaces more than 90 degrees are in total shadow 
			    
			    double d_red = l.color().getRed() / 255.0 * v_red * diffuseFactor;
			    double d_green = l.color().getGreen() / 255.0 * v_green * diffuseFactor;
			    double d_blue = l.color().getBlue() / 255.0 * v_blue * diffuseFactor;
			    
				// SPECULAR LIGHTING
			    double s_red = 0;
			    double s_green = 0;
			    double s_blue = 0;
			    if (diffuseFactor > 0)
			    {
			    	try
			    	{
				    	Vector3D vertexToCamera = new Vector3D(v, game.world().camera().position()).normal();
				    	Vector3D lightToVertex = Vector3D.scale(vertexToLight, -1, -1, -1);
			    		Vector3D reflect = Matrix.mult(lightToVertex, Matrix.reflectMatrix(v.normal().get(0), v.normal().get(1), v.normal().get(2)));
				    	double specularFactor = Vector3D.dotProduct(vertexToCamera, reflect);
				    	if(specularFactor < 0)
				    		specularFactor = 0;
				    	specularFactor = Math.pow(specularFactor, specularHardness);
				    	
					    s_red = l.color().getRed() / 255.0 * specularIntensity * specularFactor;
					    s_green = l.color().getGreen() / 255.0 * specularIntensity * specularFactor;
					    s_blue = l.color().getBlue() / 255.0 * specularIntensity * specularFactor;
			    	}
			    	catch(IOException exception)
			    	{
			    	}
			    }
			    
			    // ATTENUATION
		        double distanceToLight = new Vector3D(v, l.position()).magnitude();
		        double attenuationFactor = 1.0 / (1.0 + (distanceToLight / 255) * (distanceToLight / 255));
			    
			    /*
			    // FRESNEL EFFECT
			    
			    // Calc fresnel factor. We must first find the dot product of the surface
			    // normal and the vector from the camera. This ranges from [-1..1] 
			    // so we shift it up to [0..2] by adding one, before raising it to the power of FresAmount.
			    double fresnelTerm = Math.pow(Vector3D.dotProduct(vnormal, directionToViewer) + 1, 1);
			    
			    if (fresnelTerm > 1)
			    	fresnelTerm = 1;  //Ensure that the factor stays within the limits.
			    
			    //All the non-reflective terms: Ambient light, Diffuse Light and Specular Light
			    double nonReflectiveRed = red*ambAmount + red*diffuseFactor*lcolor.getRed() + lcolor.getRed()*specularFactor;
			    double nonReflectiveGreen = green*ambAmount + green*diffuseFactor*lcolor.getGreen() + lcolor.getGreen()*specularFactor;
			    double nonReflectiveBlue = blue*ambAmount + blue*diffuseFactor*lcolor.getBlue() + lcolor.getBlue()*specularFactor;

			    //Just one reflective term. Here you would normally put the reflectivity or environment maps.
			    double FresCol = 255;

			    //We combine the reflective and non-reflective terms using the fresnel factor.    
			    red = nonReflectiveRed;//*(1-FresnelTerm) + FresCol*FresnelTerm; 
			    green = nonReflectiveGreen;//*(1-FresnelTerm) + FresCol*FresnelTerm;
			    blue = nonReflectiveBlue;//*(1-FresnelTerm) + FresCol*FresnelTerm;
			    */
		        
			    red_total += a_red + attenuationFactor * (d_red + s_red);
			    green_total += a_green + attenuationFactor * (d_green + s_green);
			    blue_total += a_blue + attenuationFactor * (d_blue + s_blue);
			}
			
			// clip colors;
			if (red_total > 1)
				red_total = 1;
			if (green_total > 1)
				green_total = 1;
			if (blue_total > 1)
				blue_total = 1;
			
			v.setColor(new Color(
					(int) (red_total * 255),
					(int) (green_total * 255),
					(int) (blue_total * 255)));
			lockLights.release();
		}
	}
}

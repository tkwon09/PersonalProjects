import java.awt.Color;
import java.util.Scanner;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;

public class Object {
	
	private Triangle3D[] mesh;
	private Vertex position;
	private Vector3D scale;
	private Vector3D orientation;
	private World world;
	
	public Object(Triangle3D[] mesh){
		this.mesh = mesh;
	}
	
	public Triangle3D[] mesh(){return this.mesh;}
	public Vertex position(){return this.position;}
	public Vector3D scale(){return this.scale;}
	public Vector3D orientation(){return this.orientation;}
	public World world(){return this.world;}
	
	// This method tells this object to add itself to the specified world at the specified
	// position with the specified orientation;
	// @params World, Vertex, Vector3D
	public void addSelfToWorld(World world, Vertex position, Vector3D orientation) {
		this.world = world;
		this.position = position;
		// This vector must have a w value of 0;
		this.orientation = orientation;
		this.scale = new Vector3D(1, 1, 1);
	}
	
	// This method tells this object to remove itself from whichever world it may be in;
	public void removeSelfFromWorld() {
		this.world = null;
		this.orientation = null;
		this.position = null;
	}
	
	// This method tells this object to translate by a certain number of units;
	// @params double x,y,z
	public void translate(float x, float y, float z){
		position.set(0, position.x()+x);
		position.set(1, position.y()+y);
		position.set(2, position.z()+z);
	}
	
	public void setscale(float x, float y, float z){
		this.scale.set(0, x);
		this.scale.set(1, y);
		this.scale.set(2, z);
	}
	
	// This method tells this object to rotate by a certain number of degrees;
	// @params double x,y,z
	public void rotate(double x, double y, double z){
		// convert degrees to radians
		float xrad = (float)(x*(Math.PI/180));
		float yrad = (float)(y*(Math.PI/180));
		float zrad = (float)(z*(Math.PI/180));
		
		// get the new rotation value
		float xinc = orientation.get(0)+xrad;
		float yinc = orientation.get(1)+yrad;
		float zinc = orientation.get(2)+zrad;
		
		// check if each orientation angle is greater than 2pi 
		while (xinc>2*Math.PI)
			xinc -= 2*Math.PI;
		while (yinc>2*Math.PI)
			yinc -= 2*Math.PI;
		while (zinc>2*Math.PI)
			zinc -= 2*Math.PI;
		
		// update the orientation vector
		orientation.set(0, xinc);
		orientation.set(1, yinc);
		orientation.set(2, zinc);
		
	}
	
	// This method calls the transform matrix for this
	// object and returns a Triangle3D array of transformed
	// triangles;
	// @returns Matrix
	public Triangle3D[] transform() throws IOException{
		
		// first a transform matrix is created using the
		// position and orientation of the object
		Matrix transform = Object.getTransformMatrix(this);
		
		// a temporary Triangle3D array is created, allowing
		// for the tranformation of vertices and a retention of
		// the vertices in object space;
		Triangle3D[] temp = new Triangle3D[mesh.length];
		
		// this loop cycles through each Triangle3D in the
		// object's mesh;
		for(int i=0; i<mesh.length; i++){
			Triangle3D t = mesh[i];
			// an empty array of transformed vertices is created;
			// this allows for the original vertices to stay intact
			Vertex[] tvertices = new Vertex[3];
			
			// this loop cycles through every vertex in the current
			// Triangle3D of the object mesh;
			for(int j=0; j<t.getVertices().length; j++){
				
				// transformation
				Vector3D dim = t.getVertex(j).dimensions();
				Vector3D tdim = Matrix.mult(dim, transform);
				
				Vector3D vnormal = t.getVertex(j).normal();
				Vector3D tvnormal = Matrix.mult(vnormal, transform);
				tvnormal.normalize();
				
				// add into the array of transformed vertices
				Vertex result = new Vertex(tdim.get(0),tdim.get(1),tdim.get(2), t.getVertex(j).color());
				result.setNormal(tvnormal);
				tvertices[j] = result;
			
			}
			// at this point all spots in the transformed vertex
			// array should be filled and a new Triangle3D is created
			// with the transformed points; this is added into the
			// temporary Triangle3D array;
			Triangle3D nt = new Triangle3D(tvertices);
			nt.calculateDepth();
			temp[i] = nt;
		}
		// return the Triangle3D array of transformed triangles
		return temp;
	}
	
	// This is a static method that returns a matrix used to
	// transform an object from object coordinates to world
	// coordinates; Each vertex of the object should be
	// multiplied by the returned matrix; It throws an
	// exception in case the matrices are not the correct
	// size to multiply;
	// @params Object
	// @returns Matrix
	public static Matrix getTransformMatrix(Object o) throws IOException{
		return Matrix.constructMatrix(o.scale(), o.orientation(), o.position());
	}
	
	// This is a static method that computes a vertex normal; the
	// vertex passed in must be in an object;
	// @params Vertex, Triangle3D[]
	private static void setVertexNormal(Vertex v, Triangle3D[] mesh) {

		ArrayList<Vector3D> tnormals = new ArrayList<Vector3D>();

		for (Triangle3D t : mesh) {
			boolean IS_IN_TRIANGLE = false;
			Vertex[] tvertices = t.getVertices();

			for (Vertex tv : tvertices) {
				if (v.x() == tv.x() && v.y() == tv.y() && v.z() == tv.z()) {
					IS_IN_TRIANGLE = true;
				}
			}
			if (IS_IN_TRIANGLE) {
				Vector3D tnormal = t.normal();
				tnormal.normalize();
				tnormals.add(tnormal);
			}
		}
		Vector3D vnormal = new Vector3D(0, 0, 0, 0);
		float nx = vnormal.get(0);
		float ny = vnormal.get(1);
		float nz = vnormal.get(2);
		for (Vector3D tnormal : tnormals) {
			nx += tnormal.get(0);
			ny += tnormal.get(1);
			nz += tnormal.get(2);
		}
		vnormal.set(0, nx);
		vnormal.set(1, ny);
		vnormal.set(2, nz);
		for (int i = 0; i < 3; i++) {
			vnormal.set(i, vnormal.get(i) / tnormals.size());
		}
		vnormal.normalize();
		vnormal.set(3, 0);
		v.setNormal(vnormal);
	}
	
	// This is a static method that returns a cube primitive of a side
	// length at a certain position;
	// @params int, Vertex
	// @returns Object
	public static Object cube(int side, Color c){
		
		Vertex[] v = new Vertex[8];
		Triangle3D[] t = new Triangle3D[12];
		
		v[0] = new Vertex(-(side/2),(side/2),(side/2), c);
		v[1] = new Vertex(-(side/2),-(side/2),(side/2), c);
		v[2] = new Vertex((side/2),-(side/2),(side/2), c);
		v[3] = new Vertex((side/2),(side/2),(side/2), c);
		v[4] = new Vertex((side/2),(side/2),-(side/2), c);
		v[5] = new Vertex((side/2),-(side/2),-(side/2), c);
		v[6] = new Vertex(-(side/2),-(side/2),-(side/2), c);
		v[7] = new Vertex(-(side/2),(side/2),-(side/2), c);
		
		t[0] = new Triangle3D(v[0],v[1],v[2]);
		t[1] = new Triangle3D(v[0],v[2],v[3]);
		t[2] = new Triangle3D(v[3],v[2],v[5]);
		t[3] = new Triangle3D(v[3],v[5],v[4]);
		t[4] = new Triangle3D(v[4],v[5],v[6]);
		t[5] = new Triangle3D(v[4],v[6],v[7]);
		t[6] = new Triangle3D(v[7],v[6],v[1]);
		t[7] = new Triangle3D(v[7],v[1],v[0]);
		t[8] = new Triangle3D(v[7],v[0],v[3]);
		t[9] = new Triangle3D(v[7],v[3],v[4]);
		t[10] = new Triangle3D(v[1],v[6],v[5]);
		t[11] = new Triangle3D(v[1],v[5],v[2]);
		
		for(Triangle3D triangle : t){
			for(Vertex curr : triangle.getVertices()){
				Object.setVertexNormal(curr, t);
			}
		}
		
		return new Object(t);
	}
	
	public static Object star(int size, Color c){
		
		double angle = Math.PI/2;
		double increment = (2*Math.PI)/10;
		
		Vertex[] v = new Vertex[12];
		Triangle3D[] t = new Triangle3D[20];
		double halfsize = (size*2)/3;
		
		v[0] = new Vertex(0,0,30,c);
		v[1] = new Vertex(0,0,-30,c);
		v[2] = new Vertex((int)(size*Math.cos(angle)),(int)(size*Math.sin(angle)),0,c);
		angle += increment;
		v[3] = new Vertex((int)(halfsize*Math.cos(angle)),(int)(halfsize*Math.sin(angle)),0,c);
		angle += increment;
		v[4] = new Vertex((int)(size*Math.cos(angle)),(int)(size*Math.sin(angle)),0,c);
		angle += increment;
		v[5] = new Vertex((int)(halfsize*Math.cos(angle)),(int)(halfsize*Math.sin(angle)),0,c);
		angle += increment;
		v[6] = new Vertex((int)(size*Math.cos(angle)),(int)(size*Math.sin(angle)),0,c);
		angle += increment;
		v[7] = new Vertex((int)(halfsize*Math.cos(angle)),(int)(halfsize*Math.sin(angle)),0,c);
		angle += increment;
		v[8] = new Vertex((int)(size*Math.cos(angle)),(int)(size*Math.sin(angle)),0,c);
		angle += increment;
		v[9] = new Vertex((int)(halfsize*Math.cos(angle)),(int)(halfsize*Math.sin(angle)),0,c);
		angle += increment;
		v[10] = new Vertex((int)(size*Math.cos(angle)),(int)(size*Math.sin(angle)),0,c);
		angle += increment;
		v[11] = new Vertex((int)(halfsize*Math.cos(angle)),(int)(halfsize*Math.sin(angle)),0,c);
		
		t[0] = new Triangle3D(v[11],v[2],v[0]);
		t[1] = new Triangle3D(v[2],v[3],v[0]);
		t[2] = new Triangle3D(v[3],v[4],v[0]);
		t[3] = new Triangle3D(v[4],v[5],v[0]);
		t[4] = new Triangle3D(v[5],v[6],v[0]);
		t[5] = new Triangle3D(v[6],v[7],v[0]);
		t[6] = new Triangle3D(v[7],v[8],v[0]);
		t[7] = new Triangle3D(v[8],v[9],v[0]);
		t[8] = new Triangle3D(v[9],v[10],v[0]);
		t[9] = new Triangle3D(v[10],v[11],v[0]);
		
		t[10] = new Triangle3D(v[2],v[11],v[1]);
		t[11] = new Triangle3D(v[3],v[2],v[1]);
		t[12] = new Triangle3D(v[4],v[3],v[1]);
		t[13] = new Triangle3D(v[5],v[4],v[1]);
		t[14] = new Triangle3D(v[6],v[5],v[1]);
		t[15] = new Triangle3D(v[7],v[6],v[1]);
		t[16] = new Triangle3D(v[8],v[7],v[1]);
		t[17] = new Triangle3D(v[9],v[8],v[1]);
		t[18] = new Triangle3D(v[10],v[9],v[1]);
		t[19] = new Triangle3D(v[11],v[10],v[1]);
		
		for(Triangle3D triangle : t){
			for(Vertex curr : triangle.getVertices()){
				Object.setVertexNormal(curr, t);
			}
		}
		
		return new Object(t);
	}
	/*
	public static plane(int height, int width, int heightdivision, int widthdivision, Color color){
		Vertex[] v = new Vertex[(heightdivision+1)*(widthdivision+1)];
		Triangle3D[] t = new Triangle3D[2*(heightdivision*widthdivision)];
		
		float widthincrement = width/widthdivision;
		float heightincrement = height/heightdivision;
		
		float currwidth = 0;
		float currheight = 0;
		int vertexindex = 0;
		
		for(int r=0; r<widthdivision+1; r++){
			for(int c=0; c<heightdivision+1; c++){
				v[vertexindex] = new Vertex(currwidth, currheight , 0, color);
				vertexindex++;
				currwidth += widthincrement;
			}
			currheight += heightincrement;
		}
		
		
		
	}
	*/
	public static Object object(File file, String HAND, Color color){
		
		ArrayList<Vertex> vertices = new ArrayList<Vertex>();
		ArrayList<Triangle3D> triangles = new ArrayList<Triangle3D>();
		ArrayList<Vector3D> vnormals = new ArrayList<Vector3D>();

		Scanner sc = null;
		Color currColor = color;
		
		try {
			sc = new Scanner(file);
		} catch (FileNotFoundException e) {e.printStackTrace();}
		while(sc.hasNext()){
			String curr = sc.next();
			if(curr.equals("v")){
				float x = (float)sc.nextDouble();
				float y = (float)sc.nextDouble();
				float z = (float)sc.nextDouble();
				vertices.add(new Vertex(x, y, z, currColor));
			} else if(curr.equals("#")){
				sc.nextLine();
			} else if(curr.equals("f")){
				ArrayList<Vertex> tv = new ArrayList<Vertex>();
				while(sc.hasNextInt()){
					tv.add(vertices.get(sc.nextInt()-1));
				}
				if(tv.size()==3){
					Triangle3D temp = null;
					if(HAND.equals("CC")){
						temp = new Triangle3D(tv.get(0), tv.get(1), tv.get(2));
						for(Vertex v : temp.getVertices())
							v.setColor(currColor);
					}else if(HAND.equals("C")){
						temp = new Triangle3D(tv.get(2), tv.get(1), tv.get(0));
						for(Vertex v : temp.getVertices())
							v.setColor(currColor);
					}
					triangles.add(temp);
				} else if(tv.size()==4){
					Triangle3D temp1 = null;
					Triangle3D temp2 = null;
					if(HAND.equals("CC")){
						temp1 = new Triangle3D(tv.get(0), tv.get(1), tv.get(2));
						temp2 = new Triangle3D(tv.get(0), tv.get(2), tv.get(3));
						for(Vertex v : temp1.getVertices())
							v.setColor(currColor);
						for(Vertex v : temp2.getVertices())
							v.setColor(currColor);
					}else if(HAND.equals("C")){
						temp1 = new Triangle3D(tv.get(0), tv.get(3), tv.get(2));
						temp2 = new Triangle3D(tv.get(0), tv.get(2), tv.get(1));
						for(Vertex v : temp1.getVertices())
							v.setColor(currColor);
						for(Vertex v : temp2.getVertices())
							v.setColor(currColor);
					}
					triangles.add(temp1);
					triangles.add(temp2);
				}
			} else if(curr.equals("vt")){
				sc.nextLine();
			} else if(curr.equals("vn")){
				vnormals.add(new Vector3D(sc.nextFloat(), sc.nextFloat(), sc.nextFloat(), 0));
			} else if(curr.equals("g")){
				sc.nextLine();
			}else if(curr.equals("s")){
				sc.nextLine();
			} else if(curr.equals("c")){
				currColor = new Color(sc.nextInt(), sc.nextInt(),sc. nextInt());
			}
		}
		
		if(vnormals.size()>0){
			int index=0;
			for(Vertex v : vertices){
				v.setNormal(vnormals.get(index));
				index++;
			}
		}
		
		Triangle3D[] t = new Triangle3D[triangles.size()];
		for(int i=0; i<triangles.size(); i++){
			t[i] = triangles.get(i);
		}
		
		if(vnormals.size()==0){
			for(Triangle3D triangle : t){
				for(Vertex v : triangle.getVertices()){
					Object.setVertexNormal(v, t);
				}
			}
		}
		
		return new Object(t);
	}
	
public static Object squareobject(File file, Color color){
		
		ArrayList<Vertex> vertices = new ArrayList<Vertex>();
		ArrayList<Triangle3D> triangles = new ArrayList<Triangle3D>();
		ArrayList<Vector3D> vnormals = new ArrayList<Vector3D>();

		Scanner sc = null;
		try {
			sc = new Scanner(file);
		} catch (FileNotFoundException e) {e.printStackTrace();}
		while(sc.hasNext()){
			String curr = sc.next();
			if(curr.equals("v")){
				float x = (float)sc.nextDouble();
				float y = (float)sc.nextDouble();
				float z = (float)sc.nextDouble();
				vertices.add(new Vertex(x, y, z, color));
			} else if(curr.equals("#")){
				sc.nextLine();
			} else if(curr.equals("f")){
				
				int v1 = sc.nextInt()-1;
				sc.next();
				sc.next();
				int v2 = sc.nextInt()-1;
				sc.next();
				sc.next();
				int v3 = sc.nextInt()-1;
				sc.next();
				sc.next();
				int v4 = sc.nextInt()-1;
				sc.next();
				sc.next();
				
				Triangle3D temp1 = new Triangle3D(vertices.get(v1), vertices.get(v2), vertices.get(v3));
				Triangle3D temp2 = new Triangle3D(vertices.get(v1), vertices.get(v3), vertices.get(v4));
				triangles.add(temp1);
				triangles.add(temp2);
			} else if(curr.equals("vt")){
				sc.nextLine();
			} else if(curr.equals("vn")){
				vnormals.add(new Vector3D(sc.nextFloat(), sc.nextFloat(), sc.nextFloat(), 0));
			} else if(curr.equals("g")){
				sc.nextLine();
			}
		}
		if(vnormals.size()>0){
			int index=0;
			for(Vertex v : vertices){
				v.setNormal(vnormals.get(index));
				index++;
			}
		}
		
		Triangle3D[] t = new Triangle3D[triangles.size()];
		for(int i=0; i<triangles.size(); i++){
			t[i] = triangles.get(i);
		}
		
		if(vnormals.size()==0){
			for(Triangle3D triangle : t){
				for(Vertex v : triangle.getVertices()){
					Object.setVertexNormal(v, t);
				}
			}
		}
		
		return new Object(t);
	}
	
}

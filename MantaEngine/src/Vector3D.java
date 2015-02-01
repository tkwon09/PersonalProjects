import java.io.IOException;


public class Vector3D {
	
	private Matrix components;
	
	// this constructor creates a Vector3D
	// that measures magnitude, as w=1;
	// @params x, y, z
	// @returns Vector3D with components of
	//			x, y, z, and w
	public Vector3D(float x, float y, float z){
		this.components = new Matrix(4,1);
		this.components.setCell(0, 0, x);
		this.components.setCell(1, 0, y);
		this.components.setCell(2, 0, z);
		this.components.setCell(3, 0, 1);
	}
	
	// this constructor creates a Vector3D
	// that measures direction or magnitude
	// depending on whether w is 0 or 1;
	// @params x, y, z
	public Vector3D(float x, float y, float z, int w){
		this.components = new Matrix(4,1);
		this.components.setCell(0, 0, x);
		this.components.setCell(1, 0, y);
		this.components.setCell(2, 0, z);
		this.components.setCell(3, 0, w);
	}
	
	// this constructor creates a Vector3D
	// from point v to q;
	// @params Vertex v, Vertex q
	public Vector3D(Vertex v, Vertex q){
		this.components = new Matrix(4,1);
		this.components.setCell(0, 0, q.x()-v.x());
		this.components.setCell(1, 0, q.y()-v.y());
		this.components.setCell(2, 0, q.z()-v.z());
		this.components.setCell(3, 0, 1);
	}
	
	// this constructor creates a Vector3D
	// that contains the matrix;
	// @params M
	// 		*M must be a 4x1 matrix
	public Vector3D(Matrix M){
		this.components = M;
	}
	
	// this constructor creates a Vector3D
	// that contains all zeroes;
	// @params i
	// 		*i must be 4
	public Vector3D(int i){
		this.components = new Matrix(i,1);
	}
	
	public Matrix components(){return this.components;}
	public void setComponents(Matrix components){this.components = components;}
	public float get(int i) {return (float)this.components.cell(i, 0);}
	public void set(int i, float f) {this.components.setCell(i, 0, f);}
	
	// returns the length of the vector by the
	// Pythagorean Theorem;
	// @returns double
	public double magnitude(){
		float x = get(0);
		float y = get(1);
		float z = get(2);
		return Math.sqrt((x*x)+(y*y)+(z*z));
	}
	
	// returns the dot product of this vector
	// and another vector;
	// the scalar returned is the magnitudes of
	// both vector multiplied by the cosine of
	// the angle between the vectors;
	// @params Vector3D v
	// @returns double
	public double dotProduct(Vector3D v){
		return Vector3D.dotProduct(this, v);
	}
	
	// static version of the dotproduct method;
	// @params Vector3D v, q
	// @returns double
	public static double dotProduct(Vector3D v, Vector3D q){
		return v.get(0)*q.get(0)+v.get(1)*q.get(1)+v.get(2)*q.get(2);
	}
	
	// returns the angle this vector and another vector;
	public double angleBetweenVectors(Vector3D v){
		return Vector3D.angleBetweenVectors(this, v);
	}
	
	// static version of the angleBetweenVectors method;
	// @params Vector3D v, q
	// @returns double
	public static double angleBetweenVectors(Vector3D v, Vector3D q){
		Vector3D vnormal = v.normal();
		Vector3D qnormal = q.normal();
		return Math.acos(dotProduct(vnormal, qnormal));
	}
	
	// returns the cross product of this and another
	// vector; the vector returned is perpendicular
	// to the plane that the two vectors lie on;
	// the direction of the resulting vector is determined
	// by the order of the vectors; it is anticommutative
	// @params Vector3D v
	// @returns Vector3D
	public Vector3D crossProduct(Vector3D v){
		return crossProduct(this, v);
	}
	
	// static version of the cross product method;
	// @params Vector3D v, q
	// @returns Vector3D
	public static Vector3D crossProduct(Vector3D v, Vector3D q){
		double x = v.get(1)*q.get(2)-v.get(2)*q.get(1);
		double y = v.get(2)*q.get(0)-v.get(0)*q.get(2);
		double z = v.get(0)*q.get(1)-v.get(1)*q.get(0);
		return new Vector3D((float)x, (float)y, (float)z);
	}

	// this method turns this Vector3D into a normalized
	// Vector3D;
	public void normalize(){
		this.setComponents(this.normal().components());
	}
	
	// returns the normalized version of this vector;
	// this means that the vector has a magnitude of 1;
	// @returns Vector3D
	public Vector3D normal(){
		float x = get(0);
		float y = get(1);
		float z = get(2);
		double mag = magnitude();
		
		Vector3D n = new Vector3D(x,y,z,(int)get(3));
		
		n.set(0, (float)(x/mag));
		n.set(1, (float)(y/mag));
		n.set(2, (float)(z/mag));
		return n;
	}
	
	public static Vector3D scale(Vector3D v, double magx, double magy, double magz){
		float x = v.get(0);
		float y = v.get(1);
		float z = v.get(2);
		
		Vector3D result = new Vector3D((float)(x*magx), (float)(y*magy), (float)(z*magz));
		return result;
	}
	
	public static Vector3D translate(Vector3D v, double magx, double magy, double magz){
		float x = v.get(0);
		float y = v.get(1);
		float z = v.get(2);
		
		Vector3D result = new Vector3D((float)(x+magx), (float)(y+magy), (float)(z+magz));
		return result;
	}
	
	public Vector3D rotate(double rotx, double roty, double rotz){
		return Vector3D.rotate(this, rotx, roty, rotz);
	}
	
	public static Vector3D rotate(Vector3D v, double rotx, double roty, double rotz){
		Vector3D result = null;
		try {
		Matrix identity = new Matrix(4);
		Matrix xrot = Matrix.XrotMatrix(rotx);
		Matrix yrot = Matrix.YrotMatrix(roty);
		Matrix zrot = Matrix.ZrotMatrix(rotz);
		
		Matrix M1 = Matrix.mult(identity, xrot);
		Matrix M2 = Matrix.mult(M1, yrot);
		Matrix M3 = Matrix.mult(M2, zrot);
		result = Matrix.mult(M3, v);
		} catch (IOException e) {e.printStackTrace();}
		return result;
	}
	
	// returns a Vertex that is a certain ratio along the
	// Vector
	// 2params double
	public Vertex vertexAlongVector(double ratio){
		float x = (float) (ratio*get(0));
		float y = (float) (ratio*get(1));
		float z = (float) (ratio*get(2));
		return new Vertex(x, y, z);
	}
	
}

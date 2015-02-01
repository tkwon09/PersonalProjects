
public class Triangle3D implements Comparable<Triangle3D>{
	
	private Vertex[] vertices;
	private float depth;

	public Triangle3D(Vertex v1, Vertex v2, Vertex v3){
		Vertex[] vertices = new Vertex[3];
		vertices[0] = v1;
		vertices[1] = v2;
		vertices[2] = v3;
		this.vertices = vertices;
	}
	
	public Triangle3D(Vertex[] v){this.vertices = v;}
	public Vertex[] getVertices(){return this.vertices;}
	public Vertex getVertex(int i){return this.vertices[i];}
	public float depth(){return this.depth;}
	
	public Vector3D normal(){
		return surfaceNormal(getVertex(0), getVertex(1), getVertex(2));
	}
	
	public static Vector3D surfaceNormal(Vertex v1, Vertex v2, Vertex v3){
		Vector3D Edge1 = new Vector3D(v1, v2);
		Vector3D Edge2 = new Vector3D(v1, v3);
		Vector3D FaceNormal = Edge1.crossProduct(Edge2);
		return FaceNormal;
	}
	
	// calculates the centroid of the triangle and returns
	// the triangle's depth;
	public void calculateDepth(){
		float depth = 0;
		for(Vertex v : getVertices()){
			depth += v.z();
		}
		depth /= 3;
		this.depth = depth;
	}
	
	// compares triangles by depth;
	public int compareTo(Triangle3D t) {
		if (this.depth > t.depth())
			return 1;
		else
			return -1;
	}

}

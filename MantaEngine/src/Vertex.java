import java.awt.Color;

public class Vertex{
	
	// stores position data in a Vector3D
	private Vector3D dimensions;
	private Vector3D normal;
	private Color color;
	
	// constructor creates a Vertex using three floats;
	// vector used is a magnitude vector
	// @params float x, y, z
	public Vertex(float x, float y, float z){
		this.dimensions = new Vector3D(x, y, z);
	}
	
	// constructor creates a Vertex using three floats and a color;
	// vector used is a magnitude vector
	// @params float x, y, z; Color
	public Vertex(float x, float y, float z, Color color){
		this.dimensions = new Vector3D(x, y, z);
		this.color = color;
	}
	
	public float get(int i){return dimensions.get(i);}
	public float x(){return get(0);}
	public float y(){return get(1);}
	public float z(){return get(2);}
	public void set(int i, float f){dimensions.set(i, f);}
	public Vector3D dimensions(){return dimensions;}
	public void setNormal(Vector3D normal){this.normal = normal;}
	public Vector3D normal(){return normal;}
	public Color color(){return this.color;}
	public void setColor(Color c){this.color = c;}

}

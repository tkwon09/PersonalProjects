import java.awt.Color;


public class Light {
	
	private Vertex position;
	private Color color;
	private int strength;
	private World world;
	
	public Light(Vertex position, Color color, int strength){
		this.position = position;
		this.strength = strength;
		this.color = color;
	}
	
	public Vertex position(){return this.position;}
	public int strength(){return this.strength;}
	public Color color(){return this.color;}
	public World world(){return this.world;}
	
	public void translate(float x, float y, float z){
		position.set(0, position.x()+x);
		position.set(1, position.y()+y);
		position.set(2, position.z()+z);
	}
	
	public void addSelfToWorld(World world) {
		this.world = world;
	}

	public void removeSelfFromWorld() {
		this.world = null;
	}

}


import java.awt.Color;
import java.util.ArrayList;

public class World {

	private ArrayList<Object> objects;
	private ArrayList<Light> lights;
	private Camera camera;
	private float ambient;
	
	public World(){
		this.objects = new ArrayList<Object>();
		this.lights = new ArrayList<Light>();
	}
	
	public ArrayList<Object> objects(){return this.objects;}
	public ArrayList<Light> lights(){return this.lights;}
	public void setCamera(Camera camera){this.camera=camera;}
	public Camera camera(){return this.camera;}
	public void setAmbient(float amount){this.ambient = amount;}
	public float ambient(){return this.ambient;}
	
	public void addObject(Object object, Vertex position, Vector3D orientation){
		object.addSelfToWorld(this, position, orientation);
		this.objects.add(object);
	}
	
	public void removeObject(Object object){
		if(object.world() == this){
			object.removeSelfFromWorld();
			this.objects.remove(object);
		}
	}
	
	public void addLight(Light light){
		light.addSelfToWorld(this);
		this.lights.add(light);
	}
	
	public void removeLight(Light light){
		if(light.world() == this){
			light.removeSelfFromWorld();
			this.lights.remove(light);
		}
	}
	
}

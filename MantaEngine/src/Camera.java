import java.io.IOException;


public class Camera {
	
	private Vector3D direction;
	private Vector3D rotation;
	private Vertex position;
	private double fov;
	private double dist;
	
	public Camera(Vertex position, Vertex target){
		this.position = position;
		lookAt(target);
	}
	
	public Vector3D direction(){return this.direction;}
	public Vector3D rotation(){return this.rotation;}
	public Vertex position(){return this.position;}
	public double fov(){return this.fov;}
	public void setfov(double fov){this.fov = fov;}
	public void calcDist(){this.dist = 1/Math.tan(fov());}
	public double dist(){return this.dist;}
	
	public void lookAt(Vertex v){setDirection(new Vector3D(this.position, v));}
	
	private void setDirection(Vector3D direction){
		
		this.direction = direction;
		
	    float yaw = -(float)(Math.atan2(direction.get(2), direction.get(0)) - Math.atan2(-1, 0));
	    while (yaw < 0)
	    	yaw += 2 * Math.PI;

	    float p = (float)(Math.sqrt(direction.get(2) * direction.get(2) + direction.get(0) * direction.get(0)) * Math.sin(direction.get(2)));
	    float pitch = (float)((Math.atan2(p, direction.get(1)) - Math.atan2(-1, 0)));
	    while (pitch < 0)
	    	pitch += 2 * Math.PI;

	    float roll = 0;

	    this.setRotation(yaw, pitch, roll);
	}
	
	private void setRotation(float yaw, float pitch, float roll){
		rotation = new Vector3D(yaw, pitch, roll, 0);
	}
	
	public Matrix getTransformMatrix() throws IOException{
		return Camera.getTransformMatrix(this);
	}
	
	public static Matrix getTransformMatrix(Camera camera) throws IOException{
		return Matrix.constructMatrix(new Vector3D(1, 1, 1), camera.rotation(), camera.position()).inverse();
	}

}

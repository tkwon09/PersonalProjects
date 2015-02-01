import java.awt.Image;


public class ImageContainer {
	
	private Image image;
	private Vertex position;
	
	public ImageContainer(Image image, Vertex position){
		this.image = image;
		this.position = position;
	}
	
	public Image image(){return this.image;}
	public int height(){return this.image.getHeight(null);}
	public int width(){return this.image.getWidth(null);}
	public float x(){return this.position.x();}
	public float y(){return this.position.y();}
	public float z(){return this.position.z();}

}

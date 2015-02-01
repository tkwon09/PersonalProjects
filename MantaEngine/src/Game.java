import java.awt.Color;
import java.awt.Graphics;
import java.awt.Toolkit;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.io.File;
import java.util.ArrayList;

import javax.swing.JFrame;
import javax.swing.KeyStroke;

public class Game implements KeyListener{

	public boolean IS_RUNNING;
	private Graphics3D graphics3D;
	private Graphics2D graphics2D;
	private String RENDER;
	private World world;
	private JFrame frame;
	private ArrayList<ImageContainer> images;
	private double velx;
	private double vely;
	private double velz;
	private double accelx;
	private double accely;
	private double accelz;
	
	public Game(JFrame frame){
		IS_RUNNING = true;
		this.frame = frame;
		frame.addKeyListener(this);
		initialize();
	}
	
	private void initialize() {
		// create Graphics3D
		this.graphics3D = new Graphics3D(this);
		this.RENDER = "WIREFRAME";
		
		// create and define world
		this.world = new World();
		this.world.setCamera(new Camera(new Vertex(0,0,0), new Vertex(0,0,0)));
		this.world.camera().setfov(Math.PI/4);
		this.world.camera().calcDist();
		this.world.setAmbient((float).5);
		
		// add all 2D images to arraylist
		this.images = new ArrayList<ImageContainer>();
		this.images.add(new ImageContainer(Toolkit.getDefaultToolkit().getImage("resources/images/HUD.png"), new Vertex(0,0,0)));
		
		// create Graphics2D
		this.graphics2D = new Graphics2D(this);
		
		
		/*
		 * 
		 */
		this.velx = 0;
		this.vely = 0;
		this.velz = 0;
		
		this.accelx = .2;
		this.accely = .2;
		this.accelz = .2;
		
		// create and add world objects
		Light light1 = new Light(new Vertex(0, 300, -200),Color.WHITE, 400);
		File teapot = new File("resources/objects/teapot.obj");
		File magnolia = new File("resources/objects/magnolia.obj");
		Object object = Object.object(teapot, "C", Color.MAGENTA);
		//Object object = Object.star(50, Color.YELLOW);
		world.addLight(light1);
		world.addObject(object, new Vertex(0,0,-300), new Vector3D(0,0,0));
		
	}
	
	public World world(){return this.world;}
	public JFrame frame(){return this.frame;}
	public ArrayList<ImageContainer> images(){return this.images;}
	
	// Method to tell the game to update logic based on input
	public void updateLogic(){
		Object object = world.objects().get(0);
		object.rotate((float)velx, (float)vely, (float)velz);
	}
	
	// Tells embedded Graphics3D and Graphics2D to render
	public Graphics render(Graphics graphics){
		graphics3D.render(graphics, RENDER);
		//graphics2D.render(graphics);
		return graphics;
	}
	
	public void keyPressed(KeyEvent key) {
		if(key.getKeyCode()==KeyEvent.VK_UP){
			velx -= accelx;
		}
		if(key.getKeyCode()==KeyEvent.VK_DOWN){
			velx += accelx;
		}
		if(key.getKeyCode()==KeyEvent.VK_LEFT){
			vely -= accely;
		}
		if(key.getKeyCode()==KeyEvent.VK_RIGHT){
			vely += accely;
		}
		if(key.getKeyCode()==KeyEvent.VK_COMMA){
			velz += accelz;
		}
		if(key.getKeyCode()==KeyEvent.VK_PERIOD){
			velz -= accelz;
		}
	}
	public void keyReleased(KeyEvent key) {
		// TODO Auto-generated method stub
		
	}
	public void keyTyped(KeyEvent key) {
		// TODO Auto-generated method stub
		
	}
	
}

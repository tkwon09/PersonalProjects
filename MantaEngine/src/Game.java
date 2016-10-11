import java.awt.Color;
import java.awt.Graphics;
import java.awt.Toolkit;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.io.File;
import java.util.ArrayList;
import java.util.concurrent.Semaphore;

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
	private double accelx;
	private double accely;
	private ArrayList<Object> objects;
	private int selectedObj;
	private Semaphore lockLights;
	private Semaphore lockObjects;
	double specularHardness;
	double specularIntensity;
	
	private boolean toggle1 = true;
	private boolean toggle2 = false;
	private boolean toggle3 = false;
	private boolean toggle4 = false;
	private boolean toggle5 = false;
	private boolean toggle6 = false;

	private Light light1 = new Light(new Vertex(100, 300, -200),Color.WHITE);
	private Light light2 = new Light(new Vertex(50, -50, -250),Color.YELLOW);
	private Light light3 = new Light(new Vertex(-200, 50, -350),Color.CYAN.darker().darker());
	private Light light4 = new Light(new Vertex(100, -100, -250),Color.ORANGE.darker().darker());
	private Light light5 = new Light(new Vertex(0, -80, -300),Color.RED);
	private Light light6 = new Light(new Vertex(0, 80, -400),Color.GREEN);
	
	public Game(JFrame frame){
		IS_RUNNING = true;
		this.frame = frame;
		frame.addKeyListener(this);
		lockLights = new Semaphore(1);
		lockObjects = new Semaphore(1);
		objects = new ArrayList<Object>();
		selectedObj = 1;
		specularHardness = 3;
		specularIntensity = 2;
		initialize();
	}
	
	private void initialize() {
		// create Graphics3D
		this.graphics3D = new Graphics3D(this);
		this.RENDER = "SHADE";
		
		// create and define world
		this.world = new World();
		this.world.setCamera(new Camera(new Vertex(0,0,0), new Vertex(0,0,0)));
		this.world.camera().setfov(Math.PI/4);
		this.world.camera().calcDist();
		this.world.setAmbient((float)0.5);

		frame.setBackground(new Color(world.ambient(), world.ambient(), world.ambient()));
		
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
		
		this.accelx = 0;
		this.accely = 0;
		
		// create and add world objects
		File teapot = new File("resources/objects/teapot.obj");
		File magnolia = new File("resources/objects/magnolia.obj");
		objects.add(Object.object(teapot, "C", Color.MAGENTA));
		objects.add(Object.object(magnolia, "CC", null));
		objects.add(Object.star(50, Color.YELLOW));
		objects.add(Object.plane(200, 200, 30, 30, Color.WHITE));
		world.addLight(light1);
		world.addObject(objects.get(0), new Vertex(0,0,-300), new Vector3D(0,0,0));
		
	}
	
	public World world(){return this.world;}
	public JFrame frame(){return this.frame;}
	public ArrayList<ImageContainer> images(){return this.images;}
	
	// Method to tell the game to update logic based on input
	public void updateLogic(){
		do
		{
		}while (!lockObjects.tryAcquire());
		Object object = world.objects().get(0);
		this.velx += this.accelx;
		this.vely += this.accely;
		if (velx > 5)
		{
			velx = 5;
		}
		if (velx < -5)
		{
			velx = -5;
		}
		if (vely > 5)
		{
			vely = 5;
		}
		if (vely < -5)
		{
			vely = -5;
		}
		
		object.rotate((float)velx, (float)vely, 0);
		lockObjects.release();
	}
	
	// Tells embedded Graphics3D and Graphics2D to render
	public Graphics render(Graphics graphics) throws InterruptedException{
		graphics3D.render(graphics, RENDER, specularHardness, specularIntensity, lockLights, lockObjects);
		//graphics2D.render(graphics);
		return graphics;
	}
	
	public void keyPressed(KeyEvent key) {
		if(key.getKeyCode()==KeyEvent.VK_SPACE){
			velx = 0;
			vely = 0;
			accelx = 0;
			accely = 0;
		}
		if(key.getKeyCode()==KeyEvent.VK_UP){
			this.accelx = 0.4;
		}
		if(key.getKeyCode()==KeyEvent.VK_DOWN){
			this.accelx = -0.4;
		}
		if(key.getKeyCode()==KeyEvent.VK_LEFT){
			this.accely = -0.4;
		}
		if(key.getKeyCode()==KeyEvent.VK_RIGHT){
			this.accely = 0.4;
		}
		if(key.getKeyCode()==KeyEvent.VK_1){
			do
			{
			}while (!lockLights.tryAcquire());
			if (toggle1)
			{
				world.removeLight(light1);
				toggle1 = false;
			}
			else
			{
				world.addLight(light1);
				toggle1 = true;
			}
			lockLights.release();
		}
		if(key.getKeyCode()==KeyEvent.VK_2){
			do
			{
			}while (!lockLights.tryAcquire());
			if (toggle2)
			{
				world.removeLight(light2);
				toggle2 = false;
			}
			else
			{
				world.addLight(light2);
				toggle2 = true;
			}
			lockLights.release();
		}
		if(key.getKeyCode()==KeyEvent.VK_3){
			do
			{
			}while (!lockLights.tryAcquire());
			if (toggle3)
			{
				world.removeLight(light3);
				toggle3 = false;
			}
			else
			{
				world.addLight(light3);
				toggle3 = true;
			}
			lockLights.release();
		}
		if(key.getKeyCode()==KeyEvent.VK_4){
			do
			{
			}while (!lockLights.tryAcquire());
			if (toggle4)
			{
				world.removeLight(light4);
				toggle4 = false;
			}
			else
			{
				world.addLight(light4);
				toggle4 = true;
			}
			lockLights.release();
		}
		if(key.getKeyCode()==KeyEvent.VK_5){
			do
			{
			}while (!lockLights.tryAcquire());
			if (toggle5)
			{
				world.removeLight(light5);
				toggle5 = false;
			}
			else
			{
				world.addLight(light5);
				toggle5 = true;
			}
			lockLights.release();
		}
		if(key.getKeyCode()==KeyEvent.VK_6){
			do
			{
			}while (!lockLights.tryAcquire());
			if (toggle6)
			{
				world.removeLight(light6);
				toggle6 = false;
			}
			else
			{
				world.addLight(light6);
				toggle6 = true;
			}
			lockLights.release();
		}
		if (key.getKeyCode()==KeyEvent.VK_W)
		{
			this.RENDER = "WIREFRAME";
		}
		if (key.getKeyCode()==KeyEvent.VK_S)
		{
			this.RENDER = "SHADE";
		}
		if (key.getKeyCode()==KeyEvent.VK_EQUALS)
		{
			float amb = world.ambient() + 0.1f;
			if (amb > 1)
			{
				amb = 1;
			}
			world.setAmbient(amb);
			frame.setBackground(new Color(amb, amb, amb));
		}
		if (key.getKeyCode()==KeyEvent.VK_MINUS)
		{
			float amb = world.ambient() - 0.1f;
			if (amb < 0)
			{
				amb = 0;
			}
			world.setAmbient(amb);
			frame.setBackground(new Color(amb, amb, amb));
		}
		if (key.getKeyCode()==KeyEvent.VK_S)
		{
			this.RENDER = "SHADE";
		}
		if (key.getKeyCode()==KeyEvent.VK_Q)
		{
			this.specularHardness += 1;
			if (this.specularHardness > 12)
			{
				this.specularHardness = 12;
			}
		}
		if (key.getKeyCode()==KeyEvent.VK_A)
		{
			this.specularHardness -= 1;
			if (this.specularHardness < 0)
			{
				this.specularHardness = 0;
			}
		}
		if (key.getKeyCode()==KeyEvent.VK_E)
		{
			this.specularIntensity += 1;
			if (this.specularIntensity > 12)
			{
				this.specularIntensity = 12;
			}
		}
		if (key.getKeyCode()==KeyEvent.VK_D)
		{
			this.specularIntensity -= 1;
			if (this.specularIntensity < 0)
			{
				this.specularIntensity = 0;
			}
		}
		if (key.getKeyCode()==KeyEvent.VK_SHIFT)
		{
			do
			{
			}while (!lockObjects.tryAcquire());
			for (Object obj : objects)
			{
				world.removeObject(obj);
			}
			world.addObject(objects.get(selectedObj), new Vertex(0,0,-300), new Vector3D(0,0,0));
			selectedObj++;
			if (selectedObj >= objects.size())
			{
				selectedObj = 0;
			}
			lockObjects.release();
		}
	}
	public void keyReleased(KeyEvent key) {
		if(key.getKeyCode()==KeyEvent.VK_UP){
			this.accelx = 0;
		}
		if(key.getKeyCode()==KeyEvent.VK_DOWN){
			this.accelx = 0;
		}
		if(key.getKeyCode()==KeyEvent.VK_LEFT){
			this.accely = 0;
		}
		if(key.getKeyCode()==KeyEvent.VK_RIGHT){
			this.accely = 0;
		}
	}
	public void keyTyped(KeyEvent key) {
		// TODO Auto-generated method stub
		
	}
	
}

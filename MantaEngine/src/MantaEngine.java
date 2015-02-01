
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Toolkit;
import java.awt.image.BufferStrategy;
import javax.swing.JFrame;

public class MantaEngine {

	/**
	 * @param args
	 */
	private JFrame frame;
	private Game game;
	private BufferStrategy bufferStrategy;
	private int delay;
	
	public static void main(String[] args) {
		new MantaEngine().run();
	}

	public void run(){
		initialize();
		loop();
	}
	
	public void initialize(){
		frame = new JFrame("MantaEngine");
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		frame.setBackground(Color.BLACK);
		frame.setUndecorated(false);
		frame.setIconImage(Toolkit.getDefaultToolkit().getImage("resources/icon.png"));
		frame.setSize(800, 600);
		frame.setVisible(true);
		frame.createBufferStrategy(2);
		bufferStrategy = frame.getBufferStrategy();
		
		game = new Game(frame);
		
		delay = 10;
	}
	
	public void loop(){
		while(game.IS_RUNNING){
			try {Thread.sleep(delay);} catch (InterruptedException e) {e.printStackTrace();}
			game.updateLogic();
			updateFrame();
		}
	}
	public void updateFrame(){
		Graphics graphics = bufferStrategy.getDrawGraphics();
		graphics.clearRect(0, 0, frame.getWidth(), frame.getHeight());
		graphics = game.render(graphics);
		graphics.dispose();
		bufferStrategy.show();
		Toolkit.getDefaultToolkit().sync();
	}
	
}

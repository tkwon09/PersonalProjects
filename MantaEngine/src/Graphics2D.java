import java.awt.Graphics;

public class Graphics2D {
	
	private Game game;
	
	public Graphics2D(Game game){
		this.game = game;
	}
	
	public void render(Graphics graphics){
		for(ImageContainer image : game.images()){
			graphics.drawImage(image.image(), (int)image.x(), (int)image.y(), game.frame().getWidth(), game.frame().getHeight(), null);
		}
	}

}

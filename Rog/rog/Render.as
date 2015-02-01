package rog {
	
	import flash.display.*;
	
	/**	Render
	 *	Draws all drawables onto a canvas from the perspective of a camera
	 */
	public class Render {

		private var game:Game;

		// Constructor
		public function Render(game:Game) {
			this.game = game;
		}
		
		private function constrainCamera() {
			var minCam:int = -(game.getGameState().getMapState().getTileSize()*2);
			var maxCamX:int = (game.getGameState().getMapState().getWidth() + 2)*game.getGameState().getMapState().getTileSize();
			var maxCamY:int = (game.getGameState().getMapState().getHeight() + 2)*game.getGameState().getMapState().getTileSize();
			var farEdgeX:int = game.getCameraState().getBorderX() + game.getCameraState().getWidth();
			var farEdgeY:int = game.getCameraState().getBorderY() + game.getCameraState().getHeight();
			var mapHalfX:int = game.getGameState().getMapState().getWidth()*game.getGameState().getMapState().getTileSize()/2;
			var mapHalfY:int = game.getGameState().getMapState().getHeight()*game.getGameState().getMapState().getTileSize()/2;
			// Whole map can fit on camera
			if (maxCamX - minCam < game.getCameraState().getWidth()) {
				game.getCameraState().doShift((maxCamX - minCam)/2 - game.getCameraState().getWindowX() + minCam, 0);
			} else if (game.getCameraState().getBorderX() < minCam) {
				game.getCameraState().doShift(minCam - game.getCameraState().getBorderX(), 0);
			} else if (farEdgeX > maxCamX) {
				game.getCameraState().doShift(maxCamX - farEdgeX, 0);
			}
			if (maxCamY - minCam < game.getCameraState().getHeight()) {
				game.getCameraState().doShift(0, (maxCamY - minCam)/2 - game.getCameraState().getWindowY() + minCam);
			} else if (game.getCameraState().getBorderY() < minCam) {
				game.getCameraState().doShift(0, minCam - game.getCameraState().getBorderY());
			} else if (farEdgeY > maxCamY) {
				game.getCameraState().doShift(0, maxCamY - farEdgeY);
			}
		}

		/*
		 *  Draw elements of the game onto the Graphics instance
		 */
		public function render(g:Graphics):void {
			constrainCamera();
			var drawables:Vector.<Drawable> = game.getDrawables();
			for (var i:int = drawables.length - 1; i >= 0; i--) {
				drawables[i].draw(g, game.getCameraState());
			}
		}
	}
	
}

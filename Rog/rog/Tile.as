package rog {
	import flash.display.Graphics;
	import flash.display.BitmapData;
	import flash.geom.Matrix;
	
	/**	Tile
	 *	Represents a single tile on the game map
	 */
	public class Tile implements Positionable, Drawable {

		private var xPos:int;
		private var yPos:int;
		private var size:int;
		private var type:String;
		private var spriteSheet:SpriteSheet;
		
		// Constructor
		public function Tile(xPos:int, yPos:int, tileSize:int, spriteSheet:SpriteSheet, type:String) {
			this.xPos = xPos;
			this.yPos = yPos;
			this.size = tileSize;
			this.type = type;
			this.spriteSheet = spriteSheet;
		}
		
		/*
		 *	Implementation of Positionable interface
		 */
		public function getX():int {
			return xPos;
		}
		public function getY():int {
			return yPos;
		}
		public function getWindowX():int {
			return xPos*size;
		}
		public function getWindowY():int {
			return yPos*size;
		}
		public function getWidth():int {
			return size;
		}
		public function getHeight():int {
			return size;
		}
		public function doMove(newX:int, newY:int):Boolean {
			return false;
		}
		
		/*
		 *  Implementation of Drawable interface.
		 */
		public function draw(g:Graphics, c:CameraState):void {
			if (c.isOnCamera(this)) {
				var frame:BitmapData = spriteSheet.getSpriteFrame(type, 0, size, false);
				var xyMatrix:Matrix = new Matrix();
				xyMatrix.tx = c.getRelativeX(this);
				xyMatrix.ty = c.getRelativeY(this);
				g.lineStyle(0, 0, 0.0);
				g.beginBitmapFill(frame, xyMatrix, false, false);
				g.drawRect(xyMatrix.tx, xyMatrix.ty, frame.width, frame.height);
				g.endFill();
			}
		}
	}
	
}

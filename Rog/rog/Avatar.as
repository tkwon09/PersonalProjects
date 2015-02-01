package rog {
	import flash.display.Graphics;
	import flash.geom.Matrix;
	import flash.display.BitmapData;
	
	/**	Avatar
	 *	Represents the drawable portion of an entity that lives in a map
	 */
	public class Avatar implements Positionable, Drawable {

		private var name:String;
		private var xPos:int;
		private var yPos:int;
		private var xWindow:int;
		private var yWindow:int;
		private var spriteSheet:SpriteSheet;
		private var ai:AI;
		private var map:MapState;
		private var currFrame:BitmapData;
		private var flipped:Boolean;
		private var xOffset:int;
		private var index:uint;
		private var attributes:Attributed;

		// Constructor
		public function Avatar(map:MapState, spriteSheet:SpriteSheet, ai:AI, index:uint, name:String, attributes:Attributed, xPos:int, yPos:int, xWindow:int, yWindow:int) {
			this.name = name;
			this.xPos = xPos;
			this.yPos = yPos;
			this.xWindow = xWindow;
			this.yWindow = yWindow;
			this.spriteSheet = spriteSheet;
			this.ai = ai;
			this.map = map;
			this.flipped = false;
			this.xOffset = 0;
			this.index = index;
			this.attributes = attributes;
		}
		
		/*
		 *	Return attributes
		 */
		public function getAttributes():Attributed {
			return attributes;
		}
		
		public function getAI():AI {
			return ai;
		}
		
		/*
		 *	Set the avatar's rendered image to a particular frame of a particular animation cycle. Also flips the avatar if needed.
		 */
		public function setCurrentFrame(animationName:String, frameNum:int, flipHorizontal:Boolean, xOffset:int):void {
			flipped = flipHorizontal;
			this.xOffset = xOffset;
			this.currFrame = spriteSheet.getSpriteFrame(animationName, frameNum, map.getTileSize(), flipHorizontal);
		}
		
		/*
		 *	Get the default X movements for an animation cycle on this avatar's sprite sheet
		 */
		public function getXSteps(cycle:String):Vector.<int> {
			return spriteSheet.getXSteps(cycle);
		}
		
		/*
		 *	Get the default Y movements for an animation cycle on this avatar's sprite sheet
		 */
		public function getYSteps(cycle:String):Vector.<int> {
			return spriteSheet.getYSteps(cycle);
		}
		
		/*
		 *	Get the maximum frame count of an animation cycle on this avatar's sprite sheet
		 */
		public function getCycleFrameCount(cycle:String):int {
			return spriteSheet.getCycleFrameCount(cycle);
		}
		
		/*
		 *	Get the frame width of an animation cycle on this avatar's sprite sheet
		 */
		public function getCycleWidth(cycle:String):int {
			return spriteSheet.getWidthInfo(cycle);
		}
		
		/*
		 *	Get the frame height of an animation cycle on this avatar's sprite sheet
		 */
		public function getCycleHeight(cycle:String):int {
			return spriteSheet.getHeightInfo(cycle);
		}
		
		/*
		 *	Check if this avatar is flipped along the x-axis
		 */
		public function getFlipped():Boolean {
			return flipped;
		}
		
		/* 
		 *  Implementation of Drawable interface
		 */
		public function draw(g:Graphics, c:CameraState):void {
			if (c.isOnCamera(this)) {
				var xyMatrix:Matrix = new Matrix();
				xyMatrix.tx = c.getRelativeX(this) + xOffset;
				xyMatrix.ty = c.getRelativeY(this);
				g.lineStyle(0, 0, 0.0);
				g.beginBitmapFill(currFrame, xyMatrix, false, false);
				g.drawRect(xyMatrix.tx, xyMatrix.ty, currFrame.width, currFrame.height);
				g.endFill();
			}
		}
		
		/* 
		 *  Implementation of Positionable interface
		 */
		public function getX():int {
			return xPos;
		}
		public function getY():int {
			return yPos;
		}
		public function getWindowX():int {
			return xWindow;
		}
		public function getWindowY():int {
			return yWindow;
		}
		public function getWidth():int {
			return map.getTileSize();
		}
		public function getHeight():int {
			return map.getTileSize();
		}
		
		public function setWindowX(newX:int):void {
			this.xWindow = newX;
		}
		public function setWindowY(newY:int):void {
			this.yWindow = newY;
		}
		// Moves avatar's map coordinates, not its window position
		public function doMove(newX:int, newY:int):Boolean {
			if (map.isOnMap(xPos + newX, yPos + newY) && map.getAvatarAt(xPos + newX, yPos + newY) == null) {
				xPos += newX;
				yPos += newY;
				return true;
			}
			return false;
		}
		
		/*
		 *	Move the avatar's window position
		 */
		public function moveBy(newX:Number, newY:Number):void {
			xWindow += newX;
			yWindow += newY;
		}

		/*
		 *	Tests whether this avatar is equal to another one.
		 *	Currently implemented as a map coordinate check.
		 */
		public function equalTo(comp:Avatar):Boolean {
			return (this.index == comp.index);
		}
		
		/*
		 *	Get the name of the avatar
		 */
		public function getName():String {
			return name;
		}
	}
	
}

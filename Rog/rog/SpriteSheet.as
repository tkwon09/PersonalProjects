package rog {
	
	import flash.display.BitmapData;
	import flash.utils.ByteArray;
	import flash.geom.Rectangle;
	import flash.geom.Matrix;
	
	/**	SpriteSheet
	 *	Base class for classes linked to library sprite sheets. These subclasses need to implement certain methods.
	 *	Treat this class like an abstract class. Unfortunately, Actionscript 3 doesn't support abstract classes.
	 */
	public class SpriteSheet extends BitmapData {
		
		protected var widthInfo:Vector.<int>;
		protected var heightInfo:Vector.<int>;
		
		// Constructor
		public function SpriteSheet(widthInfo:Vector.<int>, heightInfo:Vector.<int>) {
			this.widthInfo = widthInfo;
			this.heightInfo = heightInfo;
			super(0, 0);
		}
		
		/*
		 *  Get the X steps for an animation cycle
		 *	@WARNING: This method should be overridden when implementing a sprite sheet!
		 */
		public function getXSteps(cycle:String):Vector.<int> {
			return new Vector.<int>();
		}
		
		/*
		 *  Get the Y steps for an animation cycle
		 *	@WARNING: This method should be overridden when implementing a sprite sheet!
		 */
		public function getYSteps(cycle:String):Vector.<int> {
			return new Vector.<int>();
		}
		
		/*
		 *  Get the frame count for an animation cycle
		 *	@WARNING: This method should be overridden when implementing a sprite sheet!
		 */
		public function getCycleFrameCount(cycle:String):int {
			return -1;
		}
		
		/*
		 *  Get the frame width for an animation cycle
		 *	@WARNING: This method should be overridden when implementing a sprite sheet!
		 */
		public function getWidthInfo(cycle:String):int {
			return -1;
		}
		
		/*
		 *  Get the frame height for an animation cycle
		 *	@WARNING: This method should be overridden when implementing a sprite sheet!
		 */
		public function getHeightInfo(cycle:String):int {
			return -1;
		}
		
		protected function getX(index:int, y:int):int {
			return widthInfo[y]*index;
		}
		
		protected function getY(index:int):int {
			var total:int = 0;
			for (var i:int = 0; i < index; i++)
				total += heightInfo[i];
			return total;
		}

		/*
		 *  Get a frame from a particular cycle on the sprite sheet
		 *	@WARNING: This method should be overridden when implementing a sprite sheet!
		 */
		public function getSpriteFrame(cycle:String, frame:int, scaleTo:int, flipHorizontal:Boolean):BitmapData {
			return null;
		}
		
		/*
		 *  Cuts a frame out of the sprite sheet. Can only be used by subclasses.
		 */
		protected function cutFrame(xPos:int, yPos:int, frameWidth:int, frameHeight:int, scaleTo:int, flipHorizontal:Boolean):BitmapData {
			// Cut out the frame from the bitmap data
			var bytes:ByteArray = this.getPixels(new Rectangle(xPos, yPos, frameWidth, frameHeight));
			// Set the starting index of the data returned
			bytes.position = 0;
			//trace(xPos, yPos, frameWidth, frameHeight, (bytes.length - bytes.position)/4);
			// Create the new bitmap data to return
			var unscaledBitmapData:BitmapData = new BitmapData(frameWidth, frameHeight, true, 0x00000000);
			unscaledBitmapData.setPixels(new Rectangle(0, 0, frameWidth, frameHeight), bytes);
			var scaleMatrix:Matrix;
			// If flipped, flip the bitmap horizontally
			if (flipHorizontal) {
				scaleMatrix = new Matrix(-1, 0, 0, 1, frameWidth, 0);
			} else {
				scaleMatrix = new Matrix();
			}
			// Scale the frame to the specified size while preserving size ratio
			var scalar:Number = (Number(scaleTo)/Number(frameWidth));
			if (frameHeight < frameWidth) {
				scalar = (Number(scaleTo)/Number(frameHeight));
			}
			var scaleToX = scaleTo*scalar;
			var scaleToY = scaleTo*scalar;
			scaleMatrix.scale(scalar, scalar);
			var scaledBitmapData:BitmapData = new BitmapData(scaleToX, scaleToY, true, 0x00000000);
			scaledBitmapData.draw(unscaledBitmapData, scaleMatrix);
			// Return the final bitmap data
			return scaledBitmapData;
		}
	}
	
}

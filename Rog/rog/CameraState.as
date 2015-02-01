package rog {
	
	/**	CameraState
	 *	Contains information about the camera state at any given time
	 */
	public class CameraState implements Positionable {

		private var focus:Positionable;
		private var xWindow:int;
		private var yWindow:int;
		private var camWidth:int;
		private var camHeight:int;
		private var oldX:int;
		private var oldY:int;

		// Constructor
		public function CameraState(focus:Positionable, camWidth:int, camHeight:int) {
			this.focus = focus;
			this.xWindow = focus.getWindowX();
			this.yWindow = focus.getWindowY();
			this.camWidth = camWidth;
			this.camHeight = camHeight;
		}
		
		/*
		 *	Implementation of Positionable interface
		 */
		public function getX():int {
			return focus.getX();
		}
		public function getY():int {
			return focus.getY();
		}
		public function getWindowX():int {
			return xWindow;
		}
		public function getWindowY():int {
			return yWindow;
		}
		public function getWidth():int {
			return camWidth;
		}
		public function getHeight():int {
			return camHeight;
		}
		
		public function getBorderX():int {
			return xWindow - (camWidth/2) + (focus.getWidth()/2);
		}
		public function getBorderY():int {
			return yWindow - (camHeight/2) + (focus.getHeight()/2);
		}
		
		/* 
		 *  Convert a Positionable's window coordinates to coordinates on the camera
		 */
		public function getRelativeX(obj:Positionable):int {
			return obj.getWindowX() - xWindow + (camWidth/2) - (obj.getWidth()/2);
		}
		public function getRelativeY(obj:Positionable):int {
			return obj.getWindowY() - yWindow + (camHeight/2) - (obj.getHeight()/2);
		}
		
		public function changeFocus(newFocus:Positionable):void {
			this.focus = newFocus;
		}
		
		/* 
		 *  Move the camera by a set amount
		 */
		public function doShift(xDiff:int, yDiff:int):void {
			xWindow += xDiff;
			yWindow += yDiff;
		}
		
		/* 
		 *  Move the camera based on historical movement values
		 */
		public function doMove(newX:int, newY:int):Boolean {
			var deltaX:int = newX - oldX;
			var deltaY:int = newY - oldY;
			if (oldX == 0) {
				oldX = newX;
				oldY = newY;
			} else {
				xWindow -= deltaX;
				yWindow -= deltaY;
				oldX = newX;
				oldY = newY;
			}
			return true;
		}
		
		/* 
		 *  Reset a camera move
		 */
		public function resetMove():void {
			oldX = 0;
			oldY = 0;
		}
		
		/* 
		 *  Check if a positionable is inside the camera's field of view
		 */
		public function isOnCamera(obj:Positionable):Boolean {
			var objX:int = getRelativeX(obj);
			var objY:int = getRelativeY(obj);
			var objWidth:int = obj.getWidth();
			var objHeight:int = obj.getHeight();
			return ((objX + objWidth) >= 0 && objX <= camWidth && objY <= camHeight && (objY + objHeight) >= 0);
		}

	}
	
}

package rog {
	
	/**	GameActionMoveCamera
	 *	Moves the camera on the map to a certain x, y window coordinate
	 */
	public class GameActionMoveCamera implements GameAction {

		private var newX:int;
		private var newY:int;

		// Constructor
		public function GameActionMoveCamera(newX:int, newY:int) {
			this.newX = newX;
			this.newY = newY;
		}
		
		/*
		 *	Implementation of GameAction interface
		 */
		public function doAction(g:Game):void {
			g.getCameraState().doShift(newX - g.getCameraState().getWindowX(), newY - g.getCameraState().getWindowY());
		}
	}
	
}

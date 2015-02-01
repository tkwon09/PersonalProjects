package rog {
	
	/**	GameActionFocusCamera
	 *	Moves the camera on the map to a certain x, y window coordinate
	 */
	public class GameActionFocusCamera implements GameAction {

		private var avatar:Avatar;
		private var shiftWindowX:int;
		private var shiftWindowY:int;
		private var immediate:Boolean;

		// Constructor
		public function GameActionFocusCamera(avatar:Avatar, immediate:Boolean = true, shiftWindowX:int = 0, shiftWindowY:int = 0) {
			this.avatar = avatar;
			this.shiftWindowX = shiftWindowX;
			this.shiftWindowY = shiftWindowY;
			this.immediate = immediate;
		}
		
		/*
		 *	Implementation of GameAction interface
		 */
		public function doAction(g:Game):void {
			// Tell camera to focus on the new avatar
			g.getCameraState().changeFocus(avatar);
			// If the new focus is off-camera or the focus is scheduled as immediate, center the camera on the new focus
			if (!g.getCameraState().isOnCamera(avatar) || immediate) {
				var actions:Vector.<GameAction> = new Vector.<GameAction>();
				var maxFrames:int = 5;
				// Calculate amount to shift camera by per frame
				var xSegment:int = (g.getGameState().getPlayerAvatar().getWindowX() - g.getCameraState().getWindowX() + shiftWindowX)/maxFrames;
				var ySegment:int = (g.getGameState().getPlayerAvatar().getWindowY() - g.getCameraState().getWindowY() + shiftWindowY)/maxFrames;
				// Schedule camera movement actions
				for (var i:int = maxFrames; i >= 0; i--) {
					actions.push(new GameActionMoveCamera(g.getCameraState().getWindowX() + (xSegment*i), g.getCameraState().getWindowY() + (ySegment*i)));
				}
				g.shoveActions(actions);
			}
		}
	}
	
}

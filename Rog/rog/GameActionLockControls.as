package rog {
	
	/**	GameActionLockControls
	 *	Lock the game's controls
	 */
	public class GameActionLockControls implements GameAction {

		private var lock:Boolean;

		// Constructor
		public function GameActionLockControls(lock:Boolean) {
			this.lock = lock;
		}
		
		/*
		 *	Implementation of GameAction interface
		 */
		public function doAction(g:Game):void {
			g.setLockControls(lock);
		}

	}
	
}

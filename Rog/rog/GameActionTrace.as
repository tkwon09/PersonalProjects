package rog {
	
	/**	GameActionTrace
	 *	Print a message to the console.
	 */
	public class GameActionTrace implements GameAction {

		private var message:Object;
		
		// Contructor
		public function GameActionTrace(message:Object) {
			this.message = message
		}
		
		/*
		 *	Implementation of GameAction interface
		 */
		public function doAction(g:Game):void {
			trace(message);
		}
	}
	
}

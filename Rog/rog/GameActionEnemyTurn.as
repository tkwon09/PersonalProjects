package rog {
	
	/**	GameActionEnemyTurn
	 *	Lock the game's controls
	 */
	public class GameActionEnemyTurn implements GameAction {

		// Constructor
		public function GameActionEnemyTurn() {
		}
		
		/*
		 *	Implementation of GameAction interface
		 */
		public function doAction(g:Game):void {
			var nextMoves:Vector.<GameAction> = g.getAIController().getNextMoves();
			g.queueActions(nextMoves);
		}

	}
	
}

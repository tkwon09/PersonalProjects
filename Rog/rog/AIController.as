package rog {
	
	public class AIController {

		private var g:Game;

		public function AIController(g:Game) {
			this.g = g;
		}
		
		public function getNextMoves():Vector.<GameAction> {
			var actions:Vector.<GameAction> = new Vector.<GameAction>();
			var avatars:Vector.<Avatar> = g.getGameState().getMapState().getAvatars();
			var tempMapState:MapState = g.getGameState().getMapState().duplicate();
			for (var i:int = 0; i < avatars.length; i++) {
				if (!avatars[i].equalTo(g.getGameState().getPlayerAvatar())) {
					var nextMove:Vector.<int> = avatars[i].getAI().getNextMove(avatars[i], g.getGameState().getPlayerAvatar(), tempMapState);
					if (nextMove != null) {
						actions.push(new GameActionMoveAvatar(avatars[i], nextMove[0], nextMove[1], true));
					}
				}
			}
			return actions;
		}

	}
	
}

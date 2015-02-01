package rog {
	
	import flash.events.*
	
	/**	KeyboardController
	 *	Processes keystrokes and queues up game actions for the game to perform
	 */
	public class KeyboardController {

		private var game:Game;
		private var locked:Boolean;

		// Contstructor
		public function KeyboardController(game:Game) {
			this.game = game;
			this.locked = false;
		}
		
		/*
		 *	Get lock status
		 */
		public function getLock():Boolean {
			return locked;
		}
		
		/*
		 *	Set lock status
		 */
		public function setLock(lock:Boolean):void {
			this.locked = lock;
		}
		
		/*
		 *	Process a keyboard event to tell the game how to handle it
		 */
		public function process(e:KeyboardEvent):Vector.<GameAction> {
			var gameActionArray = new Vector.<GameAction>();
			switch(e.keyCode) {
				
				// Q = Move player Up-Left
				case(81) : {
					gameActionArray.push(new GameActionMoveAvatar(game.getGameState().getPlayerAvatar(), -1, -1, true, true));
					gameActionArray.push(new GameActionEnemyTurn());
					break;
				}

				// W = Move player Up
				case(87) : {
					gameActionArray.push(new GameActionMoveAvatar(game.getGameState().getPlayerAvatar(), 0, -1, true, true));
					gameActionArray.push(new GameActionEnemyTurn());
					break;
				}

				// E = Move player Up-Right
				case(69) : {
					gameActionArray.push(new GameActionMoveAvatar(game.getGameState().getPlayerAvatar(), 1, -1, true, true));
					gameActionArray.push(new GameActionEnemyTurn());
					break;
				}

				// A = Move player Left
				case(65) : {
					gameActionArray.push(new GameActionMoveAvatar(game.getGameState().getPlayerAvatar(), -1, 0, true, true));
					gameActionArray.push(new GameActionEnemyTurn());
					break;
				}

				// D = Move player Right
				case(68) : {
					gameActionArray.push(new GameActionMoveAvatar(game.getGameState().getPlayerAvatar(), 1, 0, true, true));
					gameActionArray.push(new GameActionEnemyTurn());
					break;
				}

				// Z = Move player Down-Left
				case(90) : {
					gameActionArray.push(new GameActionMoveAvatar(game.getGameState().getPlayerAvatar(), -1, 1, true, true));
					gameActionArray.push(new GameActionEnemyTurn());
					break;
				}

				// X = Move player Down
				case(88) : {
					gameActionArray.push(new GameActionMoveAvatar(game.getGameState().getPlayerAvatar(), 0, 1, true, true));
					gameActionArray.push(new GameActionEnemyTurn());
					break;
				}

				// X = Move player Down-Right
				case(67) : {
					gameActionArray.push(new GameActionMoveAvatar(game.getGameState().getPlayerAvatar(), 1, 1, true, true));
					gameActionArray.push(new GameActionEnemyTurn());
					break;
				}
				
				case(83) : {
					gameActionArray.push(new GameActionFocusCamera(game.getGameState().getPlayerAvatar()));
					break;
				}
				
				// Unrecognize = print keyCode to console
				default : {
					gameActionArray.push(new GameActionTrace(e.keyCode));
					break;
				}
			}
			return gameActionArray;
		}

	}
	
}

package rog {
	import flash.display.Sprite;
	import flash.events.*;
	
	/**	Game
	 *	Contains code to delegate player inputs and manages various state containers
	 */
	public class Game {
		
		private var gameState:GameState;
		private var cameraState:CameraState;
		private var controller:KeyboardController;
		private var nextDragEvent:MouseEvent;
		private var keyboardEventStack:Vector.<KeyboardEvent>;
		private var keyDownMap:Object;
		private var actionQueue:Vector.<GameAction>;
		private var spriteMap:Object;
		private var animator:GameAnimator;
		private var aiMap:Object;
		private var aiController:AIController;
		
		// Constructor
		public function Game(width:int, height:int) {
			loadSprites();
			loadAIs();
			animator = new GameAnimator();
			gameState = new GameState(this);
			cameraState = new CameraState(gameState.getPlayerAvatar(), width, height);
			controller = new KeyboardController(this);
			keyboardEventStack = new Vector.<KeyboardEvent>();
			actionQueue = new Vector.<GameAction>();
			keyDownMap = new Object();
			aiController = new AIController(this);
		}
		
		/* 
		 *  Load sprite sheets.
		 */
		private function loadSprites():void {
			spriteMap = new Object();
			spriteMap['player'] = new sprite_player();
			spriteMap['tile'] = new sprite_tile();
			spriteMap['bungie'] = new sprite_enemy_bungie();
		}
		
		/* 
		 *  Load enemy AI.
		 */
		private function loadAIs():void {
			aiMap = new Object();
			aiMap['bungie'] = new ai_enemy_bungie();
		}
		
		/* 
		 *  Returns the requested sprite sheet.
		 */
		public function getSpriteSheet(name:String):SpriteSheet {
			return spriteMap[name];
		}
		
		/* 
		 *  Returns the requested ai.
		 */
		public function getAI(name:String):AI {
			return aiMap[name];
		}
		
		/* 
		 *  Returns the current state of the game.
		 */
		public function getGameState():GameState {
			return gameState;
		}
		
		/* 
		 *  Returns the the animator.
		 */
		public function getGameAnimator():GameAnimator {
			return animator;
		}
		
		/* 
		 *  Returns the the AI Controller.
		 */
		public function getAIController():AIController {
			return aiController;
		}
		
		/* 
		 *  Returns the current state of the camera.
		 */
		public function getCameraState():CameraState {
			return cameraState;
		}
		
		/* 
		 *  Updates the game logic.
		 */
		public function update():void {
			animator.doAnimations();
			controller.setLock(animator.getLocked());
			this.doEvents();
			this.doActions();
		}
		
		/* 
		 *  Performs queued up events
		 */
		private function doEvents():void {
			// Move the camera if the player is dragging with the mouse
			if (nextDragEvent != null) {
				cameraState.doMove(nextDragEvent.localX, nextDragEvent.localY);
			} else {
				cameraState.resetMove();
			}
			
			// Perform queue of keyboard events
			var testOverlap:Object = new Object();
			while (keyboardEventStack.length > 0) {
				var currKeyboardEvent:KeyboardEvent = keyboardEventStack.pop();
				// If the key is released, remove it from the map and add it to the local map
				if (currKeyboardEvent.type == KeyboardEvent.KEY_UP) {
					delete keyDownMap[currKeyboardEvent.keyCode];
					testOverlap[currKeyboardEvent.keyCode] = true;
				// If there are keys that are released and pressed down in the same event stack, don't process the keypress
				} else if (!keyDownMap.hasOwnProperty(currKeyboardEvent.keyCode) && !testOverlap.hasOwnProperty(currKeyboardEvent.keyCode) && currKeyboardEvent.type == KeyboardEvent.KEY_DOWN) {
					var actions:Vector.<GameAction> = controller.process(currKeyboardEvent);
					// Queue actions
					if (!controller.getLock()) {
						queueActions(actions);
					}
					// Record keypress
					keyDownMap[currKeyboardEvent.keyCode] = true;
				}
			}
		}
		
		/* 
		 *  Perform next action in the action queue
		 */
		private function doActions():void {
			if (actionQueue.length > 0) {
				actionQueue.shift().doAction(this);
			}
		}
		
		/* 
		 *  Sorts events by event type
		 */
		public function notify(e:Event):void {
			switch (e.type) {
				case MouseEvent.MOUSE_DOWN : {
					if (nextDragEvent == null) {
						nextDragEvent = MouseEvent(e);
					}
					break;
				}
				case MouseEvent.MOUSE_MOVE : {
					if (nextDragEvent != null) {
						nextDragEvent = MouseEvent(e);
					}
					break;
				}
				case MouseEvent.MOUSE_UP : {
						nextDragEvent = null;
					break;
				}
				case MouseEvent.MOUSE_OUT : {
						nextDragEvent = null;
					break;
				}
				case KeyboardEvent.KEY_DOWN : {
					if (keyboardEventStack.every(hasOnlyKeyUp)) {
						keyboardEventStack.push(KeyboardEvent(e));
					}
					break;
				}
				case KeyboardEvent.KEY_UP : {
					keyboardEventStack.push(KeyboardEvent(e));
					break;
				}
			}
		}
		
		/*
		 *	Helper function. Checks if a vector of keyboard events only contains KEY_UP events
		 */
		private function hasOnlyKeyUp(e:KeyboardEvent, index:int, all:Vector.<KeyboardEvent>):Boolean {
			return e.type == KeyboardEvent.KEY_UP;
		}
		
		/* 
		 *  Add game actions to the end of the actionQueue
		 */
		public function queueActions(gameActions:Vector.<GameAction>):void {
			for (var i:int = 0; i < gameActions.length; i++) {
				actionQueue.push(gameActions[i]);
			}
		}
		
		/* 
		 *  Add game actions to the front of the actionQueue
		 */
		public function shoveActions(gameActions:Vector.<GameAction>):void {
			for (var i:int = 0; i < gameActions.length; i++) {
				actionQueue.unshift(gameActions[i]);
			}
		}
		
		/* 
		 *  Returns drawables.
		 */
		public function getDrawables():Vector.<Drawable> {
			return gameState.getDrawables();
		}
	}
	
}

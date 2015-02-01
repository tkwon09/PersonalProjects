package {
	
	import rog.*;
	import flash.events.*;
	import flash.display.*;
	
	[SWF(width='640', height='480', backgroundColor='#292C2C', frameRate='30')]
	
	/**	Main
	 *	sets up the game and its components. Contains the main game loop
	 */
	public class Main extends MovieClip {
		
		private var canvas:Sprite;
		private var render:Render;
		private var game:Game;
		
		// Constructor
		public function Main() {
			canvas = new Sprite();
			this.addChild(canvas);
			resetCanvas();
			
			game = new Game(640, 480);
			render = new Render(game);
			stage.addEventListener(Event.ENTER_FRAME, update, false, 0, true);
			this.addEventListener(MouseEvent.MOUSE_DOWN, notify, false, 0, true);
			this.addEventListener(MouseEvent.MOUSE_MOVE, notify, false, 0, true);
			this.addEventListener(MouseEvent.MOUSE_UP, notify, false, 0, true);
			this.addEventListener(MouseEvent.MOUSE_OUT, notify, false, 0, true);
			stage.addEventListener(KeyboardEvent.KEY_DOWN, notify, false, 0, true);
			stage.addEventListener(KeyboardEvent.KEY_UP, notify, false, 0, true);
			stage.showDefaultContextMenu = false;
			stage.quality = 'low';
		}
		
		private function resetCanvas():void {
			canvas.graphics.clear();
			canvas.graphics.beginFill(0x292C2C);
			canvas.graphics.drawRect(0, 0, 640, 480);
			canvas.graphics.endFill();
		}
		
		// Mouse listener
		public function notify(e:Event):void {
			game.notify(e);
		}
		
		// Main game loop
		public function update(e:Event):void {
			// Update game state
			game.update();
			
			// Draw game
			resetCanvas();
			render.render(canvas.graphics);
		}
	}
	
}

package rog {
	
	/**	GameState
	 *	Contains the state of the game at any given moment
	 */
	public class GameState {
		
		private var playerState:PlayerState;
		private var playerAvatar:Avatar;
		private var mapState:MapState;
		
		// Constructor
		public function GameState(g:Game) {
			buildDemo(g);
		}
		
		private function buildDemo(g:Game) {
			var tileSize:int = 30;
			var mapWidth:int = 20 - Math.random()*10;
			var mapHeight:int = 20 - Math.random()*10;
			this.mapState = new MapState(mapWidth, mapHeight, tileSize, g.getSpriteSheet('tile'));
			this.playerState = new PlayerState('Jojo');
			this.playerAvatar = this.mapState.addAvatarAt(1, 1, playerState.getName(), g.getSpriteSheet('player'), null, new Attributed(25, 10, 10, 5), g.getGameAnimator());
			var count:int = 0;
			var reduc:int = Math.random()*5;
			while (count < 10 - reduc) {
				var xPos:int = Math.random()*mapWidth;
				var yPos:int = Math.random()*mapHeight;
				if (this.mapState.getAvatarAt(xPos, yPos) == null) {
					this.mapState.addAvatarAt(xPos, yPos, 'enemy bungie' + count, g.getSpriteSheet('bungie'), g.getAI('bungie'), new Attributed(15, 5, 7, 5), g.getGameAnimator());
					count++;
				}
			}
		}
		
		/*
		 *	Returns reference to player state
		 */
		public function getPlayerState():PlayerState {
			return this.playerState;
		}
		
		/*
		 *	Returns reference to player avatar
		 */
		public function getPlayerAvatar():Avatar {
			return this.playerAvatar;
		}
		
		/*
		 *	Returns reference to map state
		 */
		public function getMapState():MapState {
			return this.mapState;
		}
		
		/*
		 *	Returns drawables ordered by z-index
		 */
		public function getDrawables():Vector.<Drawable> {
			var drawables = new Vector.<Drawable>();
			drawables.push(mapState);
			return drawables;
		}
	}
	
}

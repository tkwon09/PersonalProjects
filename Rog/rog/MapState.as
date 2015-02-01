package rog {
	
	import flash.display.*;
	
	/**	MapState
	 *	Contains all information about the state of the map at any given moment
	 */
	public class MapState implements Drawable {

		private var mapWidth:int;
		private var mapHeight:int;
		private var tileSize:int;
		private var tileGrid:Vector.<Vector.<Tile>>;
		private var avatarVector:Vector.<Avatar>;
		private var tileSpriteSheet:SpriteSheet;

		// Constructor
		public function MapState(mapWidth:int, mapHeight:int, tileSize:int, spriteSheet:SpriteSheet) {
			this.mapWidth = mapWidth;
			this.mapHeight = mapHeight;
			this.tileSize = tileSize;
			tileGrid = new Vector.<Vector.<Tile>>();
			avatarVector = new Vector.<Avatar>;
			this.tileSpriteSheet = spriteSheet;
			
			// Create an empty grid of tiles.
			for (var i:int = 0; i < mapWidth; i++) {
				tileGrid.push(new Vector.<Tile>());
				for (var j:int = 0; j < mapHeight; j++) {
					var tile:String = 'grass_1'
					if (j == 4 || j == 5) {
						tile = 'grass_stone_1';
						if (j % 2 == 0)
							tile = 'grass_stone_2';
					} else if (Math.random() > 0.5) {
						tile = 'grass_2';
					}
					tileGrid[i].push(new Tile(i, j, tileSize, spriteSheet, tile));
				}
			}
		}
		
		/*
		 *  Returns the tile size
		 */
		public function getTileSize():int {
			return tileSize;
		}
		
		public function getWidth():int {
			return mapWidth;
		}
		
		public function getHeight():int {
			return mapHeight;
		}
		
		/*
		 *  Returns the tile at some map coordinates
		 */
		public function getTileAt(xPos:int, yPos:int):Tile {
			return tileGrid[xPos][yPos];
		}

		/*
		 *  Returns the avatar at some map coordinates
		 */
		public function getAvatarAt(xPos:int, yPos:int):Avatar {
			for (var i:int = 0; i < avatarVector.length; i++) {
				if (avatarVector[i].getX() == xPos && avatarVector[i].getY() == yPos) {
					return avatarVector[i];
				}
			}
			return null;
		}
		
		public function getAvatars():Vector.<Avatar> {
			return avatarVector;
		}
		
		/*
		 *  Adds an avatar at specified map coordinates if it's empty. Constrains the Avatar to default tile size.
		 */
		public function addAvatarAt(xPos:int, yPos:int, name:String, spriteSheet:SpriteSheet, ai:AI, attributes:Attributed, animator:GameAnimator, animationCycle:String = 'idle_default'):Avatar {
			if (getAvatarAt(xPos, yPos) == null) {
				var index:uint = avatarVector.length;
				var avatar:Avatar = new Avatar(this, spriteSheet, ai, index, name, attributes, xPos, yPos, xPos*tileSize, yPos*tileSize);
				avatarVector.push(avatar);
				var vect:Vector.<int> = new Vector.<int>(0);
				if (animator != null) {
					animator.scheduleConcurrentAvatarAnimation(avatar, animationCycle, 0, false, vect, vect);
				}
				return avatar;
			}
			return null;
		}
		
		/*
		 *  Removes an avatar at specified map coordinates.
		 */
		public function removeAvatarAt(xPos:int, yPos:int):void {
			for (var i:int = 0; i < avatarVector.length; i++) {
				if (avatarVector[i].getX() == xPos && avatarVector[i].getY() == yPos) {
					avatarVector.splice(i, 1);
				}
			}
		}

		/*
		 *  Checks collision with edge of map
		 */
		public function isOnMap(xTest:int, yTest:int):Boolean {
			return xTest >= 0 && yTest >= 0 && xTest < tileGrid.length && yTest < tileGrid[xTest].length;
		}
		
		/*
		 *  Implementation of Drawable interface.
		 */
		public function draw(g:Graphics, camera:CameraState):void {
			for (var i:int = 0; i < tileGrid.length; i++) {
				for (var j:int = 0; j < tileGrid[i].length; j++) {
					tileGrid[i][j].draw(g, camera);
				}
			}
			for (i = 0; i < avatarVector.length; i++) {
				avatarVector[i].draw(g, camera);
			}
		}
		
		public function duplicate():MapState {
			var duplicateMapState:MapState = new MapState(mapWidth, mapHeight, tileSize, tileSpriteSheet);
			for (var i:int = 0; i < avatarVector.length; i++) {
				duplicateMapState.addAvatarAt(avatarVector[i].getX(), avatarVector[i].getY(), avatarVector[i].getName(), null, avatarVector[i].getAI(), avatarVector[i].getAttributes(), null, null);
			}
			return duplicateMapState;
		}
	}
	
}

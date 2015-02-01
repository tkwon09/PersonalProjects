package  {
	
	import rog.SpriteSheet;
	import flash.display.BitmapData;
	
	/**	sprite_enemy_bungie
	 *	Connecting class for player sprite sheet
	 */
	public class sprite_enemy_bungie extends rog.SpriteSheet {
		
		// Constructor
		public function sprite_enemy_bungie() {
			var setWidthInfo:Vector.<int> = new <int>[15, 20];
			var setHeightInfo:Vector.<int> = new <int>[15, 15];
			super(setWidthInfo, setHeightInfo);
		}
		
		/*
		 *  Get the X steps for an animation cycle
		 */
		override public function getXSteps(cycle:String):Vector.<int> {
			var v:Vector.<int> = new Vector.<int>();
			switch (cycle) {
				case('idle_default') : {
					v = new <int>[0];
					break;
				}
				case('walk') : {
					v = new <int>[0];
					break;
				}
				case('attack') : {
					v = new <int>[0];
					break;
				}
				case('damage') : {
					v = new <int>[0];
					break;
				}
			}
			return v;
		}
		
		/*
		 *  Get the Y steps for an animation cycle
		 */
		override public function getYSteps(cycle:String):Vector.<int> {
			var v:Vector.<int> = new Vector.<int>();
			switch (cycle) {
				case('idle_default') : {
					v = new <int>[0];
					break;
				}
				case('walk') : {
					v = new <int>[0];
					break;
				}
				case('attack') : {
					v = new <int>[0];
					break;
				}
				case('damage') : {
					v = new <int>[0];
					break;
				}
			}
			return v;
		}
		
		/*
		 *	Store frame count for each animation cycle
		 */
		override public function getCycleFrameCount(cycle:String):int {
			switch (cycle) {
				case('idle_default') : {
					return 12;
				}
				case('walk') : {
					return 8;
				}
				case('damage') : {
					return 7;
				}
				case('attack') : {
					return 9;
				}
			}
			return -1;
		}
		
		/*
		 *  Get the frame width for an animation cycle
		 */
		override public function getWidthInfo(cycle:String):int {
			switch (cycle) {
				case('idle_default') : {
					return widthInfo[0];
				}
				case('walk') : {
					return widthInfo[0];
				}
				case('attack') : {
					return widthInfo[1];
				}
				case('damage') : {
					return widthInfo[0];
				}
			}
			return -1;
		}
		
		/*
		 *  Get the frame height for an animation cycle
		 */
		override public function getHeightInfo(cycle:String):int {
			switch (cycle) {
				case('idle_default') : {
					return heightInfo[0];
				}
				case('walk') : {
					return heightInfo[0];
				}
				case('attack') : {
					return heightInfo[1];
				}
				case('damage') : {
					return heightInfo[0];
				}
			}
			return -1;
		}
		
		/*
		 *	Store coordinates of each frame in an animation cycle.
		 */
		override public function getSpriteFrame(cycle:String, frame:int, scaleTo:int, flipHorizontal:Boolean):BitmapData {
			
			switch (cycle) {
				
				// IDLE
				case('idle_default') : {
					switch(frame) {
						case(0) : {
							return cutFrame(getX(0, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(1) : {
							return cutFrame(getX(0, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(2) : {
							return cutFrame(getX(0, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(3) : {
							return cutFrame(getX(0, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(4) : {
							return cutFrame(getX(0, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(5) : {
							return cutFrame(getX(0, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(6) : {
							return cutFrame(getX(1, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(7) : {
							return cutFrame(getX(1, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(8) : {
							return cutFrame(getX(1, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(9) : {
							return cutFrame(getX(1, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(10) : {
							return cutFrame(getX(1, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(11) : {
							return cutFrame(getX(1, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
					}
					break;
				}
						
				// WALK
				case ('walk') : {
					switch(frame) {
						case(0) : {
							return cutFrame(getX(2, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(1) : {
							return cutFrame(getX(2, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(2) : {
							return cutFrame(getX(0, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(3) : {
							return cutFrame(getX(0, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(4) : {
							return cutFrame(getX(2, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, !flipHorizontal);
						}
						case(5) : {
							return cutFrame(getX(2, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, !flipHorizontal);
						}
						case(6) : {
							return cutFrame(getX(0, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(7) : {
							return cutFrame(getX(0, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
					}
					break;
				}
				
				// DAMAGE
				case ('damage') : {
					switch(frame) {
						case(0) : {
							return cutFrame(getX(4, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(1) : {
							return cutFrame(getX(4, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(2) : {
							return cutFrame(getX(3, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(3) : {
							return cutFrame(getX(3, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(4) : {
							return cutFrame(getX(4, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(5) : {
							return cutFrame(getX(4, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(6) : {
							return cutFrame(getX(3, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(7) : {
							return cutFrame(getX(3, 0), getY(0), widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
					}
					break;
				}
				
				// ATTACK
				case ('attack') : {
					switch(frame) {
						case(0) : {
							return cutFrame(getX(0, 1), getY(1), widthInfo[1], heightInfo[1], scaleTo, flipHorizontal);
						}
						case(1) : {
							return cutFrame(getX(1, 1), getY(1), widthInfo[1], heightInfo[1], scaleTo, flipHorizontal);
						}
						case(2) : {
							return cutFrame(getX(2, 1), getY(1), widthInfo[1], heightInfo[1], scaleTo, flipHorizontal);
						}
						case(3) : {
							return cutFrame(getX(3, 1), getY(1), widthInfo[1], heightInfo[1], scaleTo, flipHorizontal);
						}
						case(4) : {
							return cutFrame(getX(3, 1), getY(1), widthInfo[1], heightInfo[1], scaleTo, flipHorizontal);
						}
						case(5) : {
							return cutFrame(getX(4, 1), getY(1), widthInfo[1], heightInfo[1], scaleTo, flipHorizontal);
						}
						case(6) : {
							return cutFrame(getX(4, 1), getY(1), widthInfo[1], heightInfo[1], scaleTo, flipHorizontal);
						}
						case(7) : {
							return cutFrame(getX(5, 1), getY(1), widthInfo[1], heightInfo[1], scaleTo, flipHorizontal);
						}
						case(8) : {
							return cutFrame(getX(5, 1), getY(1), widthInfo[1], heightInfo[1], scaleTo, flipHorizontal);
						}
					}
					break;
				}
			}
			return null;
		}
		
	}
	
}

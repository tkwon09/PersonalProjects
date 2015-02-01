package  {
	
	import rog.SpriteSheet;
	import flash.display.BitmapData;
	
	/**	sprite_player
	 *	Connecting class for player sprite sheet
	 */
	public class sprite_player extends rog.SpriteSheet {
		
		// Constructor
		public function sprite_player() {
			var setWidthInfo:Vector.<int> = new <int>[15, 25];
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
					v = new <int>[-18, 0, 0, 6, 6, 3, 3, 0, 0, 0];
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
					v = new <int>[0, 0, 0, -6, -3, 3, 6, 0, 0, 0];
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
				case('attack') : {
					return 10;
				}
				case('damage') : {
					return 8;
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
				
				// PLAYER ANIMATION: IDLE
				case('idle_default') : {
					switch(frame) {
						case(0) : {
							return cutFrame(0, 0, widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(1) : {
							return cutFrame(0, 0, widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(2) : {
							return cutFrame(0, 0, widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(3) : {
							return cutFrame(0, 0, widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(4) : {
							return cutFrame(0, 0, widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(5) : {
							return cutFrame(0, 0, widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(6) : {
							return cutFrame(getX(1, 0), 0, widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(7) : {
							return cutFrame(getX(1, 0), 0, widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(8) : {
							return cutFrame(getX(1, 0), 0, widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(9) : {
							return cutFrame(getX(1, 0), 0, widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(10) : {
							return cutFrame(getX(1, 0), 0, widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(11) : {
							return cutFrame(getX(1, 0), 0, widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
					}
					break;
				}
						
				// PLAYER ANIMATION: WALK
				case ('walk') : {
					switch(frame) {
						case(0) : {
							if (flipHorizontal) {
								return cutFrame(getX(3, 0), 0, widthInfo[0], heightInfo[0], scaleTo, true);
							}
							return cutFrame(getX(2, 0), 0, widthInfo[0], heightInfo[0], scaleTo, false);
						}
						case(1) : {
							if (flipHorizontal) {
								return cutFrame(getX(3, 0), 0, widthInfo[0], heightInfo[0], scaleTo, true);
							}
							return cutFrame(getX(2, 0), 0, widthInfo[0], heightInfo[0], scaleTo, false);
						}
						case(2) : {
							return cutFrame(0, 0, widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(3) : {
							return cutFrame(0, 0, widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(4) : {
							if (flipHorizontal) {
								return cutFrame(getX(2, 0), 0, widthInfo[0], heightInfo[0], scaleTo, true);
							}
							return cutFrame(getX(3, 0), 0, widthInfo[0], heightInfo[0], scaleTo, false);
						}
						case(5) : {
							if (flipHorizontal) {
								return cutFrame(getX(2, 0), 0, widthInfo[0], heightInfo[0], scaleTo, true);
							}
							return cutFrame(getX(3, 0), 0, widthInfo[0], heightInfo[0], scaleTo, false);
						}
						case(6) : {
							return cutFrame(0, 0, widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(7) : {
							return cutFrame(0, 0, widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
					}
					break;
				}
				
				// PLAYER ANIMATION: DAMAGE
				case ('damage') : {
					switch(frame) {
						case(0) : {
							return cutFrame(getX(4, 0), 0, widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(1) : {
							return cutFrame(getX(4, 0), 0, widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(2) : {
							return cutFrame(getX(5, 0), 0, widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(3) : {
							return cutFrame(getX(5, 0), 0, widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(4) : {
							return cutFrame(getX(4, 0), 0, widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(5) : {
							return cutFrame(getX(4, 0), 0, widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(6) : {
							return cutFrame(getX(5, 0), 0, widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
						case(7) : {
							return cutFrame(getX(5, 0), 0, widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
						}
					}
					break;
				}
				
				// PLAYER ANIMATION: ATTACK
				case ('attack') : {
					switch(frame) {
						case(0) : {
							return cutFrame(getX(0, 1), getY(1), widthInfo[1], heightInfo[1], scaleTo, flipHorizontal);
						}
						case(1) : {
							return cutFrame(getX(0, 1), getY(1), widthInfo[1], heightInfo[1], scaleTo, flipHorizontal);
						}
						case(2) : {
							return cutFrame(getX(0, 1), getY(1), widthInfo[1], heightInfo[1], scaleTo, flipHorizontal);
						}
						case(3) : {
							return cutFrame(getX(1, 1), getY(1), widthInfo[1], heightInfo[1], scaleTo, flipHorizontal);
						}
						case(4) : {
							return cutFrame(getX(1, 1), getY(1), widthInfo[1], heightInfo[1], scaleTo, flipHorizontal);
						}
						case(5) : {
							return cutFrame(getX(1, 1), getY(1), widthInfo[1], heightInfo[1], scaleTo, flipHorizontal);
						}
						case(6) : {
							return cutFrame(getX(2, 1), getY(1), widthInfo[1], heightInfo[1], scaleTo, flipHorizontal);
						}
						case(7) : {
							return cutFrame(getX(2, 1), getY(1), widthInfo[1], heightInfo[1], scaleTo, flipHorizontal);
						}
						case(8) : {
							return cutFrame(getX(2, 1), getY(1), widthInfo[1], heightInfo[1], scaleTo, flipHorizontal);
						}
						case(9) : {
							return cutFrame(getX(2, 1), getY(1), widthInfo[1], heightInfo[1], scaleTo, flipHorizontal);
						}
					}
					break;
				}
			}
			return null;
		}
		
	}
	
}

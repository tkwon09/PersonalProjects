package  {
	
	import rog.SpriteSheet;
	import flash.display.BitmapData;
	
	public class sprite_tile extends rog.SpriteSheet {
		
		public function sprite_tile() {
			var setWidthInfo:Vector.<int> = new <int>[15, 15];
			var setHeightInfo:Vector.<int> = new <int>[15, 15];
			super(setWidthInfo, setHeightInfo);
		}
		
		override public function getCycleFrameCount(cycle:String):int {
			switch (cycle) {
				case('ceramic_1', 'ceramic_2') : {
					return 1;
				}
				case('grass_1', 'grass_2') : {
					return 1;
				}
			}
			return -1;
		}
		
		override public function getSpriteFrame(cycle:String, frame:int, scaleTo:int, flipHorizontal:Boolean):BitmapData {
			
			switch (cycle) {
				
				// CERAMIC
				case('ceramic_1') : {
					return cutFrame(getX(0, 0), 0, widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
				}
				case('ceramic_2') : {
					return cutFrame(getX(1, 0), 0, widthInfo[0], heightInfo[0], scaleTo, flipHorizontal);
				}
				// GRASS
				case('grass_1') : {
					return cutFrame(getX(0, 1), getY(1), widthInfo[1], heightInfo[1], scaleTo, flipHorizontal);
				}
				case('grass_2') : {
					return cutFrame(getX(1, 1), getY(1), widthInfo[1], heightInfo[1], scaleTo, flipHorizontal);
				}
				case('grass_stone_1') : {
					return cutFrame(getX(2, 1), getY(1), widthInfo[1], heightInfo[1], scaleTo, flipHorizontal);
				}
				case('grass_stone_2') : {
					return cutFrame(getX(3, 1), getY(1), widthInfo[1], heightInfo[1], scaleTo, flipHorizontal);
				}
			}
			return null;
		}
		
	}
	
}

package rog {
	
	import rog.Avatar;
	
	public class AI {
		
		protected var sightRange;
		
		public function AI() {
		}
		
		public function getNextMove(avatar:Avatar, player:Avatar, mapState:MapState):Vector.<int, int> {
			return null;
		}
		
		protected function testInSightRadius(avatar:Avatar, test:Avatar) {
			var xPos:int = avatar.getX();
			var yPos:int = avatar.getY();
			var xTest = test.getX();
			var yTest = test.getY();
			var xDiff = xTest - xPos;
			var yDiff = yTest - yPos;
			return (xDiff*xDiff + yDiff*yDiff <= sightRange*sightRange);
		}
	}
	
}

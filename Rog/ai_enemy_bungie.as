package  {
	
	import rog.AI;
	import rog.MapState;
	import rog.Avatar;
	
	public class ai_enemy_bungie extends AI {
		
		public function ai_enemy_bungie() {
			sightRange = 5;
		}
		
		/*
		 *  Implementation of AI interface
		 */
		override public function getNextMove(avatar:Avatar, player:Avatar, tempMapState:MapState):Vector.<int, int> {
			var testMove:Vector.<int> = null;
			if (testInSightRadius(avatar, player)) {
				var xDiff = player.getX() - avatar.getX();
				var yDiff = player.getY() - avatar.getY();
				var xStep = xDiff / Math.abs(xDiff);
				var yStep = yDiff / Math.abs(yDiff);
				testMove = new <int>[xStep, yStep];
				var duplicate:Avatar = tempMapState.getAvatarAt(avatar.getX(), avatar.getY());
				var conflict:Avatar = tempMapState.getAvatarAt(avatar.getX() + testMove[0], avatar.getX() + testMove[1]);
				if (conflict != null && !conflict.equalTo(player)) {
					
				} else {
					duplicate.doMove(xStep, yStep);
				}
			}
			return testMove;
		}
	}
	
}

package rog {
	
	/**	Positionable
	 *	Interface for objects that occupy a spot on the game grid. Also controls sprite size and position.
	 */
	public interface Positionable {
		function getX():int
		function getY():int
		function getWindowX():int
		function getWindowY():int
		function getWidth():int
		function getHeight():int
		function doMove(newX:int, newY:int):Boolean
	}
	
}

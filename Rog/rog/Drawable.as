package rog {
	
	import flash.display.*;
	
	/**	Drawable
	 *	Interface for objects that are drawn in the specified camera
	 */
	public interface Drawable {
		function draw(g:Graphics, camera:CameraState):void
	}
	
}

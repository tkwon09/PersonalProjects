package rog {
	
	/**	GameAnimationSequence
	 *	Contains information about an animation sequence.
	 *	@WARNING: Intended for use by a GameAnimator ONLY. Should be rewritten as a protected class.
	 */
	public class GameAnimationSequence {

		private var avatar:Avatar;
		private var cycle:String;
		private var maxFrames:int;
		private var frameCount:int;
		private var currFrame:int;
		private var xMoveIndex:int;
		private var yMoveIndex:int;
		private var lock:Boolean;
		private var loop:Boolean;
		private var flipHorizontal:Boolean;
		private var xOffset:int;
		private var xMove:Vector.<int>;
		private var yMove:Vector.<int>;

		// Constructor
		public function GameAnimationSequence(avatar:Avatar, cycle:String, maxFrames:int, lock:Boolean, xMove:Vector.<int>, yMove:Vector.<int>, flipHorizontal:Boolean, xOffset:int) {
			this.avatar = avatar;
			this.cycle = cycle;
			this.maxFrames = maxFrames;
			this.loop = maxFrames <= 0;
			this.lock = lock;
			this.flipHorizontal = flipHorizontal;
			this.xOffset = xOffset;
			this.xMove = xMove;
			this.yMove = yMove;
			currFrame = 0;
			frameCount = 0;
			xMoveIndex = 0;
			yMoveIndex = 0;
		}
		
		/*
		 *	Move one frame in the animation sequence if the sequence is not complete.
		 */
		public function step():void {
			if (!isComplete()) {
				avatar.setCurrentFrame(cycle, currFrame, flipHorizontal, xOffset);
				if (xMove.length > 0) {
					if (flipHorizontal) {
						avatar.moveBy((-1)*(xMove[xMoveIndex]), 0);
					} else {
						avatar.moveBy(xMove[xMoveIndex], 0);
					}
				}
				if (yMove.length > 0)
					avatar.moveBy(0, yMove[yMoveIndex]);
				currFrame++;
				frameCount++;
				xMoveIndex++;
				yMoveIndex++;
				if (currFrame >= avatar.getCycleFrameCount(cycle) && (loop || frameCount < maxFrames)) {
					currFrame = 0;
					if (loop) {
						frameCount = 0;
					}
				}
				if (xMoveIndex >= xMove.length)
					xMoveIndex = 0;
				if (yMoveIndex >= yMove.length)
					yMoveIndex = 0;
			}
		}
		
		/*
		 *	Helper function. Checks animation sequence overlap by comparing avatars
		 */
		public function checkOverlap(seq:GameAnimationSequence) {
			return this.avatar.equalTo(seq.avatar);
		}
		
		/*
		 *	Returns whether this animation sequence is a locking sequence or not. Loops cannot lock
		 */
		public function isLock():Boolean {
			if (loop) {
				return false;
			}
			return lock;
		}
		
		/*
		 *	Returns whether this animation sequence loops or not
		 */
		public function isLoop():Boolean {
			return loop;
		}
		
		/*
		 *	Returns whether this animation sequence is complete or not. Loops never complete
		 */
		public function isComplete():Boolean {
			if (loop) {
				return false;
			}
			return frameCount >= maxFrames;
		}
		
		/*
		 *	Debug
		 */
		public function doTrace():void {
			trace(avatar.getX(), avatar.getY(), cycle, maxFrames);
		}

	}
	
}

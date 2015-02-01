package rog {
	
	/**	GameAnimator
	 *	Manages concurrent and sequential animation sequences. Handles scheduling and frame-stepping.
	 */
	public class GameAnimator {

		private var animationList:Vector.<GameAnimationSequence>;
		private var scheduledAnimations:Vector.<Vector.<GameAnimationSequence>>;
		private var currContext;

		// Constructor
		public function GameAnimator() {
			animationList = new Vector.<GameAnimationSequence>();
			scheduledAnimations = new Vector.<Vector.<GameAnimationSequence>>();
			currContext = -1;
		}
		
		/*
		 *	Schedules an animation sequence. If an Avatar is already being animated, schedule the animation to play after it's complete.
		 */
		public function scheduleConcurrentAvatarAnimation(avatar:Avatar, cycle:String, maxFrames:int, lock:Boolean, xMove:Vector.<int>, yMove:Vector.<int>, flipHorizontal:Boolean = false, xOffset:int = 0):void {
			var seq:GameAnimationSequence = new GameAnimationSequence(avatar, cycle, maxFrames, lock, xMove, yMove, flipHorizontal, xOffset);
			animationList.unshift(seq);
			// If there's a conflict, schedule animation for later
			if (!animationList.every(checkNoConflicts)) {
				animationList.shift();
				if (scheduledAnimations.length == 0) {
					createNewContext();
				} else {
					removeOverlappingLoops(seq, scheduledAnimations[currContext]);
				}
				// Find the next overlapping sequence and schedule it after
				scheduledAnimations[currContext].unshift(seq);
				while (!scheduledAnimations[currContext].every(checkNoConflicts)) {
					scheduledAnimations[currContext].shift();
					createNewContext();
					scheduledAnimations[currContext].unshift(seq);
				}
				/*	DEBUG
				trace(cycle);
				trace('context ACTIVE')
				animationList.forEach(traceSequences);
				for (var i:int = 0; i <= currContext; i++) {
					trace('context ' + i);
					scheduledAnimations[i].forEach(traceSequences);
				}
				trace('-----------------------------------------------');
				*/
			}
		}
		
		public function scheduleSequentialAvatarAnimations(avatars:Vector.<Avatar>, cycles:Vector.<String>, maxFrames:Vector.<int>, locks:Vector.<Boolean>, xMoves:Vector.<Vector.<int>>, yMoves:Vector.<Vector.<int>>, flipHorizontals:Vector.<Boolean>, xOffsets:Vector.<int>) {
			var oldContext:int = currContext;
			if (oldContext < 0) {
				oldContext = 0;
			}
			//trace(avatars.length, cycles.length, maxFrames.length, locks.length, xMoves.length, yMoves.length, flipHorizontals.length, xOffsets.length);	DEBUG
			for (var i:int = 0; i < avatars.length; i++) {
				scheduleConcurrentAvatarAnimation(avatars[i], cycles[i], maxFrames[i], locks[i], xMoves[i], yMoves[i], flipHorizontals[i], xOffsets[i]);
			}
			currContext = oldContext;
		}
		
		public function createNewContext():void {
			currContext++;
			if (currContext >= scheduledAnimations.length) {
				scheduledAnimations.push(new Vector.<GameAnimationSequence>());
			}
		}
		
		/*
		 *	Check whether any of the current animations lock the game.
		 */
		public function getLocked():Boolean {
			for (var i:int = 0; i < scheduledAnimations.length; i++) {
				if (!scheduledAnimations[i].every(testNoLock)) {
					return true;
				}
			}
			return !animationList.every(testNoLock);
		}
		
		/*
		 *	Tell current animation sequences to progress one step.
		 *		First remove all complete animation sequences. Then, step each sequence.
		 *		If the animation list is empty, or there are only loops in the current animation list, and there are scheduled animation sequences...
		 *			Transfer the oldest scheduled animation to the active animation list.
		 *			If a loop sequence already exists for the Avatar, replace it.
		 */
		public function doAnimations():void {
			/*	DEBUG
			animationList.forEach(traceSequences);
			trace(' ');
			for (var i:int = 0; i < scheduledAnimations.length; i++) {
				scheduledAnimations[i].forEach(traceSequences);
			}
			trace('-------------------------------------');
			*/
			
			// remove completed animations
			animationList = animationList.filter(filterComplete);
			// step each animation sequence
			animationList.forEach(processSequence);
			// if the animationList contains only loops or is empty
			if (animationList.length == 0 || animationList.every(testOnlyLoop)) {
				// while there are scheduled animations
				if (scheduledAnimations.length > 0) {
					// add on the next scheduled animations. replace any loops
					var newAnimations:Vector.<GameAnimationSequence> = scheduledAnimations.shift();
					while (newAnimations.length > 0) {
						var currScheduled:GameAnimationSequence = newAnimations.pop();
						removeOverlappingLoops(currScheduled, animationList);
						animationList.push(currScheduled);
					}
					if (currContext >= scheduledAnimations.length) {
						currContext--;
					}
				}
			}
		}
		
		/*
		 *	Support method. Removes loops from a vector of animation sequences that overlap with the given animation sequence
		 */
		private function removeOverlappingLoops(seq:GameAnimationSequence, all:Vector.<GameAnimationSequence>):void {
			for (var i:int = 0; i < all.length; i++) {
				if (all[i].isLoop() && all[i].checkOverlap(seq)) {
					all.splice(i, 1);
					i--;
				}
			}
		}
		
		/*
		 *	Debug
		 */
		private function traceSequences(seq:GameAnimationSequence, index:int, all:Vector.<GameAnimationSequence>):void {
			seq.doTrace();
		}
		
		/*
		 *	Helper function. Progresses the current animation sequence by one step.
		 */
		private function processSequence(seq:GameAnimationSequence, index:int, all:Vector.<GameAnimationSequence>):void {
			seq.step();
		}
		
		/*
		 *	Helper function. If the sequence is complete, flag it for removal.
		 */
		private function filterComplete(seq:GameAnimationSequence, index:int, all:Vector.<GameAnimationSequence>):Boolean {
			return !seq.isComplete();
		}
		
		/*
		 *	Helper function. Check if sequence should lock controls. If it's complete, ignore the locking attribute.
		 */
		private function testNoLock(seq:GameAnimationSequence, index:int, all:Vector.<GameAnimationSequence>):Boolean {
			return !(seq.isLock() && !seq.isComplete());
		}
		
		/*
		 *	Helper function. Check if the current sequence is a loop.
		 */
		private function testOnlyLoop(seq:GameAnimationSequence, index:int, all:Vector.<GameAnimationSequence>):Boolean {
			return seq.isLoop();
		}
		
		/*
		 *	Helper function. Check whether a sequence is in conflict with the first sequence in the vector.
		 */
		private function checkNoConflicts(seq:GameAnimationSequence, index:int, all:Vector.<GameAnimationSequence>):Boolean {
			if (index == 0) {
				return true;
			}
			return !seq.checkOverlap(all[0]);
		}
	}
	
}

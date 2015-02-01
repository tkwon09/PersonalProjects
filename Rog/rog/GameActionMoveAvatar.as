package rog {
	
	/**	GameActionMovePlayer
	 *	Moves the player avatar on the map, then schedules a player move animation.
	 */
	public class GameActionMoveAvatar implements GameAction {

		private var avatar:Avatar;
		private var newX:int;
		private var newY:int;
		private var focus:Boolean;
		private var scheduleSequential:Boolean;

		// Constructor
		public function GameActionMoveAvatar(avatar:Avatar, newX:int, newY:int, scheduleSequential:Boolean = false, focus:Boolean = false) {
			this.avatar = avatar;
			this.newX = newX;
			this.newY = newY;
			this.focus = focus;
			this.scheduleSequential = scheduleSequential;
		}
		
		/*
		 *	Implementation of GameAction interface
		 */
		public function doAction(g:Game):void {

			// Schedule a camera focus action so the camera refocuses on the player
			if (focus) {
				var tileSize:int = g.getGameState().getMapState().getTileSize();
				var actions:Vector.<GameAction> = new Vector.<GameAction>();
				actions.push(new GameActionFocusCamera(avatar, true, newX*tileSize, newY*tileSize));
				g.shoveActions(actions);
			}
			
			// Flip if needed
			var flip:Boolean = avatar.getFlipped();
			if (newX < 0) {
				flip = true;
			} else if (newX > 0) {
				flip = false;
			}
			
			// Move the avatar's map coordinates. If successful, schedule animations
			if (avatar.doMove(newX, newY)) {
				scheduleMoveAnimations(avatar, g, flip);
			// See if there's another avatar in the way and attack it
			} else {
				var target:Avatar = g.getGameState().getMapState().getAvatarAt(avatar.getX() + newX, avatar.getY() + newY);
				if (target != null) {
					scheduleAttackAnimations(avatar, target, g, flip);
				}
			}
		}
		
		/*
		 *	Support function. Schedules player walk animation.
		 */
		private function scheduleMoveAnimations(player:Avatar, g:Game, flip:Boolean):void {
			
			// Set number of frames to play animation for
			var maxFrames:int = 10;
			
			// Calculate amount to shift avatar's window position per frame
			var tileSize:int = g.getGameState().getMapState().getTileSize();
			var xStep:int = newX*tileSize/maxFrames;
			var yStep:int = newY*tileSize/maxFrames;
			
			// Schedule a walk cycle, then schedule the idle animation on loop.
			var xSteps:Vector.<int> = new Vector.<int>();
			for (var i:int = 0; i < maxFrames; i++) {
				if (flip) {
					xSteps.push((-1)*xStep);
				} else {
					xSteps.push(xStep);
				}
			}
			var ySteps:Vector.<int> = new Vector.<int>();
			for (i = 0; i < maxFrames; i++)
				ySteps.push(yStep);
			
			// Creating vectors for scheduled sequential animations
			var avatars:Vector.<Avatar> = new <Avatar>[avatar, avatar];
			var cycles:Vector.<String> = new <String>['walk', 'idle_default'];
			var maxFrameCounts:Vector.<int> = new <int>[maxFrames, 0];
			var locks:Vector.<Boolean> = new <Boolean>[true, false];
			var xStepsVect:Vector.<Vector.<int>> = new <Vector.<int>>[xSteps, new <int>[0]];
			var yStepsVect:Vector.<Vector.<int>> = new <Vector.<int>>[ySteps, new <int>[0]];
			var flips:Vector.<Boolean> = new <Boolean>[flip, flip];
			var xOffsets:Vector.<int> = new <int>[0, 0];
			
			// If not on camera, shorten animation to 1 frame.
			if (!avatar.equalTo(g.getGameState().getPlayerAvatar()) && !g.getCameraState().isOnCamera(avatar)) {
				maxFrames[0] = 1;
				xStepsVect[0] = new <int>[newX];
				yStepsVect[0] = new <int>[newY];
			}
			// Schedule animations according to scheduling criteria
			if (scheduleSequential) {
				g.getGameAnimator().scheduleSequentialAvatarAnimations(avatars, cycles, maxFrameCounts, locks, xStepsVect, yStepsVect, flips, xOffsets);
			} else {
				for (i = 0; i < avatars.length; i++) {
					g.getGameAnimator().scheduleConcurrentAvatarAnimation(avatars[i], cycles[i], maxFrameCounts[i], locks[i], xStepsVect[i], yStepsVect[i], flips[i]);
				}
			}
		}
		
		/*
		 *	Support function. Schedules attack animation and target's damage animation
		 */
		private function scheduleAttackAnimations(attacker:Avatar, target:Avatar, g:Game, flip:Boolean):void {
			
			// Do the damage
			var originalHealth:int = target.getAttributes().getHealth();
			var damageDealt:int = target.getAttributes().doDamage(attacker.getAttributes(), 'physical');
			var message:String = attacker.getName() + ' deals ' + damageDealt + ' physical damage to ' + target.getName() + '! ' + originalHealth + ' -> ' + target.getAttributes().getHealth();
			
			// Simple death thingy
			if (target.getAttributes().getHealth() == 0)
				g.getGameState().getMapState().removeAvatarAt(target.getX(), target.getY());
				
			g.queueActions(new <GameAction>[new GameActionTrace(message)]);
			
			// Calculate offset
			var xOffset:int = 0;
			if (newX < 0) {
				xOffset = (-1)*attacker.getCycleWidth('attack');
			}
			
			// Set up scheduled sequential animations
			var avatarsAttacker:Vector.<Avatar> = new <Avatar>[attacker, attacker];
			var cyclesAttacker:Vector.<String> = new <String>['attack', 'idle_default'];
			var maxFramesAttacker:Vector.<int> = new <int>[attacker.getCycleFrameCount('attack'), 0];
			var locksAttacker:Vector.<Boolean> = new <Boolean>[true, false];
			var xStepsAttacker:Vector.<Vector.<int>> = new <Vector.<int>>[attacker.getXSteps('attack'), attacker.getXSteps('idle_default')];
			var yStepsAttacker:Vector.<Vector.<int>> = new <Vector.<int>>[attacker.getYSteps('attack'), attacker.getYSteps('idle_default')];
			var flipsAttacker:Vector.<Boolean> = new <Boolean>[flip, flip];
			var xOffsetsAttacker:Vector.<int> = new <int>[xOffset, 0];
			
			var avatarsTarget:Vector.<Avatar> = new <Avatar>[target, target];
			var cyclesTarget:Vector.<String> = new <String>['damage', 'idle_default'];
			var maxFramesTarget:Vector.<int> = new <int>[target.getCycleFrameCount('damage'), 0];
			var locksTarget:Vector.<Boolean> = new <Boolean>[true, false];
			var xStepsTarget:Vector.<Vector.<int>> = new <Vector.<int>>[target.getXSteps('damage'), target.getXSteps('idle_default')];
			var yStepsTarget:Vector.<Vector.<int>> = new <Vector.<int>>[target.getYSteps('damage'), target.getYSteps('idle_default')];
			var flipsTarget:Vector.<Boolean> = new <Boolean>[!flip, !flip];
			var xOffsetsTarget:Vector.<int> = new <int>[0, 0];
			
			// Schedule animations based on scheduling criteria
			if (scheduleSequential) {
				g.getGameAnimator().scheduleSequentialAvatarAnimations(avatarsAttacker, cyclesAttacker, maxFramesAttacker, locksAttacker, xStepsAttacker, yStepsAttacker, flipsAttacker, xOffsetsAttacker);
				g.getGameAnimator().scheduleSequentialAvatarAnimations(avatarsTarget, cyclesTarget, maxFramesTarget, locksTarget, xStepsTarget, yStepsTarget, flipsTarget, xOffsetsTarget);
			} else {
				for (var i:int = 0; i < avatarsAttacker.length; i++) {
					g.getGameAnimator().scheduleConcurrentAvatarAnimation(avatarsAttacker[i], cyclesAttacker[i], maxFramesAttacker[i], locksAttacker[i], xStepsAttacker[i], yStepsAttacker[i], flipsAttacker[i], xOffsetsAttacker[i]);
					g.getGameAnimator().scheduleConcurrentAvatarAnimation(avatarsTarget[i], cyclesTarget[i], maxFramesTarget[i], locksTarget[i], xStepsTarget[i], yStepsTarget[i], flipsTarget[i], xOffsetsTarget[i]);
				}
			}
			
		}
	}
	
}

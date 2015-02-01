package rog {
	
	/**	PlayerState
	 *	Contains all information about player at any given moment
	 */
	public class PlayerState {

		private var nameString:String;
		private var equipment:Array;

		// Constructor
		public function PlayerState(nameString:String) {
			this.nameString = nameString;
			this.equipment = new Array();
		}
		
		public function getName():String {
			return nameString;
		}
	}
	
}

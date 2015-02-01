
public class ScanLine{
	
	private Vertex v1;
	private Vertex v2;
	
	public ScanLine(Vertex v1, Vertex v2){
		this.v1 = v1;
		this.v2 = v2;
	}
	
	public Vertex v1(){return this.v1;}
	public Vertex v2(){return this.v2;}
	public float xdist(){return v2.x()-v1.x();}
	// returns slope of line; if it is vertical the slope is adjusted
	// to near vertical
	public float slope(){
		float slope = (v2.x() - v1.x()) / (v2.y() - v1.y());
		if(slope<99999999&&slope>-99999999)
			return slope;
		else if(slope>99999999)
			return 99999999;
		else
			return -99999999;

	}

}

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

/*public class ScanLine{
	
	private Vertex v1;
	private Vertex v2;
	private float inverse_slope;
	private float[] color_increment;
	
	public ScanLine(Vertex v1, Vertex v2){
		this.v1 = v1;
		this.v2 = v2;
		float rise = v2.y() - v1.y();
		float run = v2.x() - v1.x();
		if (rise == 0)
		{
			if (run > 0)
			{
				inverse_slope = Float.MIN_VALUE;
			}
			else if (run < 0)
			{
				inverse_slope = Float.MAX_VALUE;
			}
			else
			{
				inverse_slope = 0;
			}
		}
		else
		{
			inverse_slope = run / rise;
		}
		float scale = (float) (1f / (new Vector3D(v1, v2).magnitude()));
		color_increment = new float[]{
				(float) ((v2.color().getRed() - v1.color().getRed()) * scale),
				(float) ((v2.color().getGreen() - v1.color().getGreen()) * scale),
				(float) ((v2.color().getBlue() - v1.color().getBlue()) * scale)
				};
	}
	
	public Vertex v1(){return this.v1;}
	public Vertex v2(){return this.v2;}
	public float inverseSlope(){ return inverse_slope; }
	public float[] colorIncrement(){ return color_increment; }
	
	public float[] getStartColorValues()
	{
		float[] colorValues = new float[3];
		v1.color().getRGBColorComponents(colorValues);
		return colorValues;
	}
	
	public float[] getEndColorValues()
	{
		float[] colorValues = new float[3];
		v2.color().getRGBColorComponents(colorValues);
		return colorValues;
	}
}
*/
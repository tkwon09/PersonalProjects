import java.awt.Color;
import java.awt.Graphics;

import javax.swing.JFrame;

public class GouraudShader {
	
	public GouraudShader(){
	}
	
	public static void shade(Vertex v1, Vertex v2, Vertex v3, Graphics graphics, JFrame frame){
		
		// pick the highest vertex out of the three and then wind the
		// points counter clockwise;
		Vertex A;
		Vertex B;
		Vertex C;
		
		if(v1.y()>=v2.y() && v1.y()>v3.y()){
			A = v1;
			B = v2;
			C = v3;
		} else if(v2.y()>=v1.y() && v2.y()>v3.y()){
			A = v2;
			B = v3;
			C = v1;
		} else{
			A = v3;
			B = v1;
			C = v2;
		}
		
		// create scan lines
		ScanLine line1 = new ScanLine(A, B);
		ScanLine line2 = new ScanLine(A, C);
		
		// initialize the x values of both the left and right lines and color value
		int xL = (int)A.x();
		int xR = xL;
		Color Lcolor = A.color();
		Color Rcolor = Lcolor;
		Color incolor = Lcolor;
		
		// scanning loop
		for(int y=(int)line1.v1().y(); y>=line1.v2().y(); y--){
			
			// if the end of line 1 is reached before the end of line 2
			// is reached, then line 1 is now from B to C;
			if(y==line1.v2().y() && y>=line2.v2().y()){
				line1 = new ScanLine(B, C);
				Lcolor = B.color();
			
				// if the end of line 2 is reached before the end of line 1
				// is reached, then line 2 is now from C to B;
			} else if(y<line2.v2().y()){
				line2 = new ScanLine(C, B);
				Rcolor = C.color();
			}
			
			float[] incolorvals = new float[3];
			float[] Lcolorvals = new float[3];
			float[] Rcolorvals = new float[3];
			Lcolor.getRGBColorComponents(incolorvals);
			Lcolor.getRGBColorComponents(Lcolorvals);
			Rcolor.getRGBColorComponents(Rcolorvals);

			int xdiff = xR-xL;
			
			float[] incolordiffvals = new float[3];
			
			for(int ci=0; ci<3; ci++){
				incolordiffvals[ci] = Rcolorvals[ci]-Lcolorvals[ci];
			}
			
			// loop through each point between the lines;
			for(int x=xL; x<xR; x++){
				for(int ci=0; ci<3; ci++){
					incolorvals[ci] += incolordiffvals[ci]/xdiff;
				
					if(incolorvals[ci]>1)
						incolorvals[ci] = 1;
					else if(incolorvals[ci]<0)
						incolorvals[ci] = 0;
				}
				incolor = new Color(incolorvals[0], incolorvals[1], incolorvals[2]);
				
				graphics.setColor(incolor);
				int sx = x+frame.getWidth()/2;
				int sy = (frame.getHeight()/2)-y;
				graphics.drawLine(sx, sy, sx, sy);
			}
			
			// increment the left x and right x;
			xL = (int)(line1.v2().x()+(line1.slope()*(y-line1.v2().y())));
			xR = (int)(line2.v2().x()+(line2.slope()*(y-line2.v2().y())));
			
			// create float arrays to contain color values for each line
			// and vertex
			float[] Lcolorvalues = new float[3];
			float[] Rcolorvalues = new float[3];
			float[] L1V1colorvalues = new float[3];
			float[] L1V2colorvalues = new float[3];
			float[] L2V1colorvalues = new float[3];
			float[] L2V2colorvalues = new float[3];
			Lcolor.getRGBColorComponents(Lcolorvalues);
			Rcolor.getRGBColorComponents(Rcolorvalues);
			line1.v1().color().getRGBColorComponents(L1V1colorvalues);
			line1.v2().color().getRGBColorComponents(L1V2colorvalues);
			line2.v1().color().getRGBColorComponents(L2V1colorvalues);
			line2.v2().color().getRGBColorComponents(L2V2colorvalues);
			
			// calculate y difference
			float Lydiff = line1.v1().y()-line1.v2().y();
			float Rydiff = line2.v1().y()-line2.v2().y();
			
			// loop through all color values
			for(int ci=0; ci<3; ci++){
				// get the current left line color
				float currLcolorval = Lcolorvalues[ci];
				// get the current right line color 
				float currRcolorval = Rcolorvalues[ci];
				// get Line 1 Vertex 1 color
				float L1V1colorval = L1V1colorvalues[ci];
				// get Line 1 Vertex 2 color
				float L1V2colorval = L1V2colorvalues[ci];
				// get Line 2 Vertex 1 color
				float L2V1colorval = L2V1colorvalues[ci];
				// get Line 2 Vertex 2 color
				float L2V2colorval = L2V2colorvalues[ci];
				
				// calculate left line color difference
				float Lcolorvaldiff = L1V2colorval-L1V1colorval;
				// calculate right line color difference
				float Rcolorvaldiff = L2V2colorval-L2V1colorval;
				
				// calculate left line color slope
				float Lcolorslope = Lcolorvaldiff/Lydiff;
				// calculate right line color slope
				float Rcolorslope = Rcolorvaldiff/Rydiff;
				
				// increment left line color
				currLcolorval += Lcolorslope;
				// increment right line color
				currRcolorval += Rcolorslope;
				
				// clip left line color values
				if(currLcolorval>1)
					currLcolorval = 1;
				else if(currLcolorval<0)
					currLcolorval = 0;
				// clip right line color values
				if(currRcolorval>1)
					currRcolorval = 1;
				else if(currRcolorval<0)
					currRcolorval = 0;
				
				// set colors
				Lcolorvalues[ci] = currLcolorval;
				Rcolorvalues[ci] = currRcolorval;
			}
			// set line colors
			Lcolor = new Color(Lcolorvalues[0], Lcolorvalues[1], Lcolorvalues[2]);
			Rcolor = new Color(Rcolorvalues[0], Rcolorvalues[1], Rcolorvalues[2]);
		}
	}
}

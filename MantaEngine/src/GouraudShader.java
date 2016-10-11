import java.awt.Color;
import java.awt.Graphics;
import java.util.ArrayList;
import java.util.Comparator;

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
		
		if (v1.y() >= v2.y() && v1.y() > v3.y()){
			A = v1;
			B = v2;
			C = v3;
		} else if (v2.y() >= v1.y() && v2.y() > v3.y()){
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
		for(int y = (int)line1.v1().y(); y >= line1.v2().y(); y--){
			
			// if the end of line 1 is reached before the end of line 2
			// is reached, then line 1 is now from B to C;
			if (y <= line1.v2().y() && y >= line2.v2().y()){
				line1 = new ScanLine(B, C);
				Lcolor = B.color();
			
			// if the end of line 2 is reached before the end of line 1
			// is reached, then line 2 is now from C to B;
			} else if (y < line2.v2().y()){
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
	
	/*public static void shade(Vertex v1, Vertex v2, Vertex v3, Graphics graphics, JFrame frame){
		
		// pick the highest vertex out of the three and then wind the
		// points counter clockwise;
		Vertex A;
		Vertex B;
		Vertex C;
		
		if (v1.y() > v2.y())
		{
			if (v1.y() > v3.y())
			{
				if (v2.y() > v3.y())
				{
					A = v1;
					B = v2;
					C = v3;
				}
				else
				{
					A = v1;
					B = v3;
					C = v2;
				}
			}
			else
			{
				A = v3;
				B = v1;
				C = v2;
			}
		}
		else
		{
			if (v1.y() > v3.y())
			{
				A = v2;
				B = v1;
				C = v3;
			}
			else
			{
				if (v2.y() > v3.y())
				{
					A = v2;
					B = v3;
					C = v1;
				}
				else
				{
					A = v3;
					B = v2;
					C = v1;
				}
			}
		}
		// If A.y == B.y and A.x > B.x, swap A and B
		if (A.y() == B.y() && A.x() > B.x())
		{
			Vertex B_t = B;
			B = A;
			A = B_t;
		}
		// If B.x < C.x, go A->B, A->C. otherwise A->C, A->B
		ScanLine line1 = new ScanLine(A, B);
		ScanLine line2 = new ScanLine(A, C);
		if (B.x() > C.x())
		{
			ScanLine temp = line1;
			line1 = line2;
			line2 = temp;
		}
		
		// initialize the x values of both the left and right lines and color value
		int xL = (int)line1.v1().x();
		int xR = (int)line2.v1().x();
		float[] Lcolor = line1.getStartColorValues();
		float[] Rcolor = line2.getStartColorValues();
		float[] incolor;
		
		// scanning loop
		for (int y = (int)line1.v1().y(); y >= line1.v2().y(); y--)
		{
			// if the end of line 1 is reached before the end of line 2,
			// set line 1 to B -> C
			if (y == line1.v2().y() && y >= line2.v2().y())
			{
				line1 = new ScanLine(B, C);
				Lcolor = line1.getStartColorValues();
			}
			// if the end of line 2 is reached before the end of line 2,
			// set line 1 to B -> C
			if (y == line2.v2().y() && y >= line1.v2().y())
			{
				line2 = new ScanLine(B, C);
				Rcolor = line2.getStartColorValues();
			}
			
			incolor = new float[]{Lcolor[0], Lcolor[1], Lcolor[2]};
			float[] incolordiff;
			if ((xR - xL) == 0)
			{
				incolordiff = new float[]{Rcolor[0], Rcolor[1], Rcolor[2]};
			}
			else
			{
				incolordiff = new float[]{
						(Rcolor[0] - Lcolor[0]) / (xR - xL),
						(Rcolor[1] - Lcolor[1]) / (xR - xL),
						(Rcolor[2] - Lcolor[2]) / (xR - xL)
								};
			}
			
			// loop through each point between the lines;
			for(int x = xL; x < xR; x++){
				incolor[0] = incolor[0] + incolordiff[0];
				incolor[1] = incolor[1] + incolordiff[1];
				incolor[2] = incolor[2] + incolordiff[2];
				
				if (incolor[0] < 0)
				{
					incolor[0] = 0;
				}
				if (incolor[0] > 1)
				{
					incolor[0] = 1;
				}
				if (incolor[1] < 0)
				{
					incolor[1] = 0;
				}
				if (incolor[1] > 1)
				{
					incolor[1] = 1;
				}
				if (incolor[2] < 0)
				{
					incolor[2] = 0;
				}
				if (incolor[2] > 1)
				{
					incolor[2] = 1;
				}
				
				graphics.setColor(new Color(incolor[0], incolor[1], incolor[2]));
				int sx = x + frame.getWidth() >> 1;
				int sy = (frame.getHeight() >> 1) - y;
				graphics.drawLine(sx, sy, sx, sy);
			}
			
			// increment the left x and right x;
			xL = (int) (xL + line1.inverseSlope());
			xR = (int) (xR + line2.inverseSlope());
			
			// increment the left color and right color
			Lcolor[0] = Lcolor[0] + line1.colorIncrement()[0];
			Lcolor[1] = Lcolor[1] + line1.colorIncrement()[1];
			Lcolor[2] = Lcolor[2] + line1.colorIncrement()[2];
			
			Rcolor[0] = Rcolor[0] + line2.colorIncrement()[0];
			Rcolor[1] = Rcolor[1] + line2.colorIncrement()[1];
			Rcolor[2] = Rcolor[2] + line2.colorIncrement()[2];
		}
	}*/
}

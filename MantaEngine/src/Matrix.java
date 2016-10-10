
import java.io.IOException;
import java.io.PrintStream;

public class Matrix {

	private float[][] data;

	// create a square identity matrix of specified dimension
	public Matrix(int dimension){
		data = new float[dimension][dimension];
		for (int i = 0; i < dimension; i++)
			for (int j = 0; j < dimension; j++)
				data[i][j] = ((i==j) ? 1 : 0);
	}

	// create a matrix of any dimensions containing all zeroes
	public Matrix(int rows, int cols){
		data = new float[rows][cols];
		for (int i = 0; i < rows; i++)
			for (int j = 0; j < cols; j++)
				data[i][j] = 0;
	}

	public Matrix(Matrix M){
		data = new float[M.rows()][M.cols()];
		try {copy(M);} catch (IOException e) {System.out.println("Invalid matrix");}
	}

	public void copy(Matrix M) throws IOException{
		if (M.rows() != rows() || M.cols() != cols())
			throw new IOException();
		subcopy(M);
	}

	public void subcopy(Matrix M) throws IOException{
		for (int i = 0; i < M.rows(); i++)
			for (int j = 0; j < M.cols(); j++)
				setCell(i, j, M.cell(i, j));
	}

	public int cols(){return (data.length > 0) ? data[0].length : 0;}
	public int rows(){return data.length;}
	// return the cell at the ith row and jth column
	public float cell(int i, int j){return data[i][j];}
	// set the value of the cell at the ith row and jth column
	public void setCell(int i, int j, float value){data[i][j] = value;}

	
	// MATRIX ADDITION
	public Matrix add(Matrix M2) throws IOException{
		if (rows() != M2.rows() || cols() != M2.cols())
			throw new IOException();

		Matrix result = new Matrix(rows(), cols());
		for (int i = 0; i < rows(); i++)
			for (int j = 0; j < cols(); j++)
				result.setCell(i, j, cell(i, j) + M2.cell(i, j));
		return result;
	}

	public static Matrix add(Matrix M1, Matrix M2) throws IOException{ return M1.add(M2); }

	
	// MATRIX SUBTRACTION
	public Matrix subtract(Matrix M2) throws IOException{
		if (rows() != M2.rows() || cols() != M2.cols())
			throw new IOException();

		Matrix result = new Matrix(rows(), cols());
		for (int i = 0; i < rows(); i++)
			for (int j = 0; j < cols(); j++)
				result.setCell(i, j, cell(i, j) - M2.cell(i, j));
		return result;
	}

	public static Matrix subtract(Matrix M1, Matrix M2) throws IOException{ return M1.subtract(M2); }

	
	// SCALAR MULTIPLICATION
	public Matrix mult(double a){
		Matrix result = new Matrix(rows(), cols());
		try {result.copy(this);} catch (IOException e) {System.out.println("Invalid matrix.");}
		result.selfMult(a);
		return result;
	}

	public void selfMult(double a){
		for (int i = 0; i < rows(); i++)
			for (int j = 0; j < cols(); j++)
				data[i][j] *= a;
	}

	
	// VECTOR MULTIPLICATION
	
	// Each component of the resulting vector is the summation of each corresponding
	// row of the matrix multiplied by that component.
	// Use this type for all calculations.
	public static Vector3D mult(Matrix M1, Vector3D v2) throws IOException{
		if (M1.cols() != 4)
			throw new IOException();

		Vector3D result = new Vector3D(0,0,0,0);
		for (int j = 0; j < M1.rows(); j++) {
			double temp = 0;
			for (int i = 0; i < M1.cols(); i++)
				temp += M1.cell(i, j) * v2.get(i);
			result.set(j, (float)temp);
		}
		return result;
	}
	
	// Each component of the resulting vector is the summation of each corresponding
	// column of the matrix multiplied by that component
	public static Vector3D mult(Vector3D v1, Matrix M2) throws IOException{
		if (4 != M2.rows())
			throw new IOException();

		Vector3D result = new Vector3D(0,0,0,0);
		for (int i = 0; i < M2.cols(); i++) {
			double temp = 0;
			for (int j = 0; j < M2.rows(); j++)
				temp += M2.cell(i, j) * v1.get(j);
			result.set(i, (float)temp);
		}
		return result;
	}

	
	// MATRIX MULTIPLICATION
	public Matrix mult(Matrix M2) throws IOException{
		if (rows() != M2.cols())
			throw new IOException();

		Matrix result = new Matrix(M2.rows(), cols());
		for (int i = 0; i < result.rows(); i++) {
			for (int j = 0; j < result.cols(); j++) {
				double temp = 0;
				for (int k = 0; k < rows(); k++)
					temp += cell(k, j) * M2.cell(i, k);
				result.setCell(i, j, (float)temp);
			}
		}
		return result;
	}

	public static Matrix mult(Matrix M1, Matrix M2) throws IOException
	{ return M1.mult(M2); }
	
	// INVERSE MATRIX
	public Matrix inverse(){
		Matrix inv = new Matrix(rows());
		return inv.inverse(this);
	}

	// sets this Matrix to the inverse of the original Matrix
	// and returns this.
	public Matrix inverse(Matrix original){

		if (original.cols() < 1 || original.cols() != original.rows() || cols() != original.cols() || cols() != rows()) return this;
		int n = cols();
		try {copy(new Matrix(n));} catch (IOException e) {System.out.println("Invalid matrix");}

		// make identity matrix

		if (cols() == 1){
			setCell(0, 0, 1 / original.cell(0, 0));
			return this;
		}

		Matrix b = new Matrix(original);

		for (int i = 0; i < n; i++){
			// find pivot
			double mag = 0;
			int pivot = -1;

			for (int j = i; j < n; j ++){
				double mag2 = Math.abs(b.cell(i, j));
				if (mag2 > mag){
					mag = mag2;
					pivot = j;
				}
			}

			// no pivot (error)
			if (pivot == -1 || mag == 0){
				return this;
			}

			// move pivot row into position
			if (pivot != i){
				double temp;
				for (int j = i; j < n; j ++){
					temp = b.cell(j, i);
					setCell(j, i, b.cell(j, pivot));
					b.setCell(j, pivot, (float)temp);
				}

				for (int j = 0; j < n; j ++){
					temp = cell(j, i);
					setCell(j, i, cell(j, pivot));
					setCell(j, pivot, (float)temp);
				}
			}

			// normalize pivot row
			mag = b.cell(i, i);
			for (int j = i; j < n; j ++) b.setCell(j, i, (float)(b.cell(j, i) / mag));
			for (int j = 0; j < n; j ++) setCell(j, i, (float)(cell(j, i) / mag));

			// eliminate pivot row component from other rows
			for (int k = 0; k < n; k ++){
				if (k == i) continue;
				double mag2 = b.cell(i, k);

				for (int j = i; j < n; j ++) b.setCell(j, k, (float)(b.cell(j, k) - mag2 * b.cell(j, i)));
				for (int j = 0; j < n; j ++) setCell(j, k, (float)(cell(j, k) - mag2 * cell(j, i)));
			}
		}
		return this;
	}

	// The following method takes a 3x3 matrix and creates a transformation matrix where
	// column 0 is x rotation, 1 is y rotation, 2 is z rotation, and 3 is translation.
	// It should be multiplied with a vector to tranform it.
	public static Matrix constructMatrix(Vector3D scale, Vector3D rotate, Vertex translate) throws IOException{
		
		Matrix identity = new Matrix(4);
		Matrix scaler = Matrix.scaleMatrix(scale.get(0), scale.get(1), scale.get(2));
		Matrix xrot = Matrix.XrotMatrix(rotate.get(0));
		Matrix yrot = Matrix.YrotMatrix(rotate.get(1));
		Matrix zrot = Matrix.ZrotMatrix(rotate.get(2));
		Matrix trans = Matrix.translateMatrix(translate.x(), translate.y(), translate.z());
		
		Matrix M = Matrix.mult(identity, scaler);
		Matrix M1 = Matrix.mult(M, xrot);
		Matrix M2 = Matrix.mult(M1, yrot);
		Matrix M3 = Matrix.mult(M2, zrot);
		Matrix M4 = Matrix.mult(M3, trans);
		
		return M4;
	}

	public void print(PrintStream out){
		String ln;
		for(int i=0; i<rows(); i++){
			ln = " ";
			for(int j=0; j<cols(); j++)
				ln += cell(i, j) + " ";
			out.println(ln);
		}
	}
	
	// TRANSLATION MATRIX
	
	public static Matrix translateMatrix(double x, double y, double z){
		Matrix trans = new Matrix(4);
		trans.setCell(0, 3, (float)x);
		trans.setCell(1, 3, (float)y);
		trans.setCell(2, 3, (float)z);
		return trans;
	}
	
	// SCALING MATRIX
	public static Matrix scaleMatrix(double x, double y, double z){
		Matrix trans = new Matrix(4);
		trans.setCell(0, 0, (float)x);
		trans.setCell(1, 1, (float)y);
		trans.setCell(2, 2, (float)z);
		return trans;
	}
	
	// REFLECTING MATRIX
	public static Matrix reflectMatrix(double a, double b, double c)
	{
		Matrix reflect = new Matrix(4);
		reflect.setCell(0, 0, (float)(1-2*a*a));
		reflect.setCell(0, 1, (float)(-2*a*b));
		reflect.setCell(0, 2, (float)(-2*a*c));
		reflect.setCell(1, 0, (float)(-2*a*b));
		reflect.setCell(1, 1, (float)(1-2*b*b));
		reflect.setCell(1, 2, (float)(-2*b*c));
		reflect.setCell(2, 0, (float)(-2*a*c));
		reflect.setCell(2, 1, (float)(-2*b*c));
		reflect.setCell(2, 2, (float)(1-2*c*c));
		return reflect;
	}
	
	// ROTATION MATRICES
	public static Matrix XrotMatrix(double angle){
		Matrix xrot = new Matrix(4);
		xrot.setCell(1,1,(float)Math.cos(angle));
		xrot.setCell(1,2,(float)(-Math.sin(angle)));
		xrot.setCell(2,1,(float)Math.sin(angle));
		xrot.setCell(2,2,(float)Math.cos(angle));
		return xrot;
	}
	
	public static Matrix YrotMatrix(double angle){
		Matrix yrot = new Matrix(4);
		yrot.setCell(0,0,(float)Math.cos(angle));
		yrot.setCell(0,2,(float)Math.sin(angle));
		yrot.setCell(2,0,(float)(-Math.sin(angle)));
		yrot.setCell(2,2,(float)Math.cos(angle));
		return yrot;
	}
	
	public static Matrix ZrotMatrix(double angle){
		Matrix zrot = new Matrix(4);
		zrot.setCell(0,0,(float)Math.cos(angle));
		zrot.setCell(0,1,(float)(-Math.sin(angle)));
		zrot.setCell(1,0,(float)Math.sin(angle));
		zrot.setCell(1,1,(float)Math.cos(angle));
		return zrot;
	}

}

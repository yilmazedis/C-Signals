#include <stdio.h>

#define MAX_SIZE 121

void printMatrix(long long int matrix[][8], int n);
void transposeMatrix(long long int target[][8], long long int source[][8], int n);
void matrixMaltiplication(long long int result[][8], long long int matrixA[][8], long long int matrixB[][8], int n);
void multiplyPolynomial(long long int P[],long long int A[],long long int B[], int m, int n);
void printSingleMarix(long long int matrix[], int n);
void copySingleMatrix(long long int target[], long long int source[], int n);
void multiplyManyPolynomial(long long int result[], long long int source[], int length);
void setSingleMarixToZero(long long int matrix[], int m);



int main(long long int argc, char const *argv[]) {

	int n = 8, i, j, k;
	long long int result[8][8] = {0};
	long long int transpose[8][8];
	long long int matrix[8][8] = {{22253, 22809, 22693, 22726, 22524, 22459, 21980, 22171}, 
									{21824, 22421, 22248, 22234, 22048, 22047, 21558, 21749},
									{21776, 22389, 22208, 22172, 22036, 22016, 21587, 21770},
									{21829, 22423, 22304, 22260, 22100, 22076, 21594, 21764},
									{22354, 22939, 22802, 22799, 22650, 22553, 22107, 22317},
									{21506, 22037, 21889, 21881, 21743, 21697, 21196, 21417},
									{21903, 22518, 22317, 22290, 22163, 22144, 21666, 21889},
									{21871, 22464, 22283, 22254, 22110, 22084, 21610, 21827}};
/*
	fprintf(stderr, "Matrix\n");
	printMatrix(matrix, n);

	transposeMatrix(transpose, matrix, n);
	fprintf(stderr, "transpose\n");
	printMatrix(transpose, n);
	
	matrixMaltiplication(result, matrix, transpose, n);
	fprintf(stderr, "Maltiplication result\n");
	printMatrix(result, n);
*/
	int length = 4;
	long long P[5];
	long long int sourceFirst[4] = {1,2,3,4};
	long long int mult = 1, sum1 = 0, sum2 = 0;
	long long int expendedSource[8][4] = {{1,3,5,9},
										  {1,2,1,7},
										  {4,3,3,7},
										  {5,2,0,4},
										  {1,3,5,9},
										  {1,2,1,7},
										  {4,3,3,7}};
	
	long long int source[4][4] = {{1,3,5,9},
								  {1,2,1,7},
								  {4,3,3,7},
								  {5,2,0,4}};

	
	multiplyManyPolynomial(P, sourceFirst, 5);

	for (i = 0; i < length; ++i)
	{
		for (j = 0; j < length; ++j)
		{
			mult *= expendedSource[i + j][i + j];
		}

		if (0 == i % 2)
		{
			sum1 += mult;
		}
		else {
			sum1 -= mult;
		}

		
		mult = 1;
	}
	

	for (i = 0; i < length; ++i)
	{
		for (j = 0; j < length; ++j)
		{	
			mult *= expendedSource[i + j][length - 1 - j];
		}
		
		if (0 == i % 2)
		{
			sum2 += mult;
		}
		else {
			sum2 -= mult;
		}
		mult = 1;
	}
	fprintf(stderr, "%lld\n", sum1);
	fprintf(stderr, "%lld\n", sum2);

	//for(i=0;i<3;i++)
	//	det = det + (arr1[0][i]*(arr1[1][(i+1)%3]*arr1[2][(i+2)%3] - arr1[1][(i+2)%3]*arr1[2][(i+1)%3]));



	//printSingleMarix(P, 5);

	return 0;
}

void multiplyManyPolynomial(long long int result[], long long int source[], int length) {

	int i;
	long long int A[MAX_SIZE], B[MAX_SIZE];

	setSingleMarixToZero(result, length);

	A[0] = -1;
	B[0] = -1;

	A[1] = source[0];
	B[1] = source[1];

	multiplyPolynomial(result, A, B, 2, 2);
	
	for (i = 2; i < length - 1; ++i)
	{
		copySingleMatrix(A, result, i + 1);

		B[0] = -1;
		B[1] = source[i];
		multiplyPolynomial(result, A, B, i + 1, 2);
	}
}

void setSingleMarixToZero(long long int matrix[], int m) {

	int i;

	for (i = 0; i < m; ++i)
	{
		matrix[i] = 0;
	}
}

void copySingleMatrix(long long int target[], long long int source[], int n) {

	int k;

	for (k = 0; k < n; ++k)
	{
		target[k] = source[k];
	}
}

void multiplyPolynomial(long long int P[], long long int A[], long long int B[], int m, int n) {

	int i, j;

	for (i = 0; i<m+n-1; i++) 
		P[i] = 0; 

	for (i=0; i<m; i++) 
	{ 
		for (j=0; j<n; j++) {
			P[i+j] += A[i] * B[j]; 
		}
	} 

}

void matrixMaltiplication(long long int result[][8], long long int matrixA[][8], long long int matrixB[][8], int n) {
	
	int i, j, k;

	for (i = 0; i < n; ++i)
	    for (j = 0; j < n; ++j)
	        for (k = 0; k < n; ++k) {
	            result[i][j] += matrixA[i][k] * matrixB[k][j];
	        }
}

void transposeMatrix(long long int target[][8], long long int source[][8], int n) {
	
	int i, j;

	for(i = 0; i < n; i++) {
		for(j = 0;j < n; j++) {
			target[j][i] = source[i][j];
		}
	}

}

void printMatrix(long long int matrix[][8], int n){
	
	int i, j;

	for(i = 0; i < n; i++)
	{
		for (j = 0; j < n; ++j)
		{
			
			fprintf(stderr, "%lld ", matrix[i][j]);
		}
		fprintf(stderr, "\n");
	}

	fprintf(stderr, "\n");
}

void printSingleMarix(long long int matrix[], int n) {

	int j;

	for (j = 0; j < n; ++j)
	{
		fprintf(stderr, "%lld\n", matrix[j]);
	}
	fprintf(stderr, "\n");	
}
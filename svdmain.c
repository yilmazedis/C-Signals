#include <stdio.h>
#include <stdlib.h>
#include "svd.h"

int main(int argc, char const *argv[])
{
  int M = 8, N = 8; 
  int err;                                                //
  double A[M][N];                                                        //
  double U[M][N];                                                        //
  double V[N][N];                                                        //
  double singular_values[N];                                             //
  double* dummy_array;    

  double source[8][8] = {{22253, 22809, 22693, 22726, 22524, 22459, 21980, 22171}, 
									{21824, 22421, 22248, 22234, 22048, 22047, 21558, 21749},
									{21776, 22389, 22208, 22172, 22036, 22016, 21587, 21770},
									{21829, 22423, 22304, 22260, 22100, 22076, 21594, 21764},
									{22354, 22939, 22802, 22799, 22650, 22553, 22107, 22317},
									{21506, 22037, 21889, 21881, 21743, 21697, 21196, 21417},
									{21903, 22518, 22317, 22290, 22163, 22144, 21666, 21889},
									{21871, 22464, 22283, 22254, 22110, 22084, 21610, 21827}};                                            
                                                                  
  dummy_array = (double*) malloc(N * sizeof(double));                    
  if (dummy_array == NULL) {printf(" No memory available\n"); } 
                                                                  
  err = Singular_Value_Decomposition((double*) source, M, N, (double*) U, singular_values, (double*) V, dummy_array);   
                                                                  
  free(dummy_array);                                                     
  if (err < 0) printf(" Failed to converge\n");                          
  else { 
  printf(" The singular value decomposition of A is \n");
      for (int i = 0; i < N; ++i)
      {
        printf("%lf\n", singular_values[i]);
      }
  }

  return 0;
}
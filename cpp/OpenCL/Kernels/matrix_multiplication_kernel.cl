
#pragma OPENCL EXTENSION cl_khr_fp64: enable

__kernel void matrix_multiplication_kernel(__global double *A,
										   __global double *B,
										   __global double *C,
										   int wA, int wB)
{
	int tx = get_global_id(0);
	int ty = get_global_id(1);

	double value = 0;
	for (int k = 0; k < wA; ++k)
	{
		double elementA = A[ty * wA + k];
		double elementB = B[k * wB + tx];
		value += elementA * elementB;
	}
	C[ty * wA + tx] = value;
}
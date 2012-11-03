
#include "CLCLasses/OpenCL.hpp"

int main(int ArgCount, char *ArgValues[])
{
	std::auto_ptr<OpenCL> ocl(new OpenCL());
	double *mat1 = new double[1024];
	double *mat2 = new double[1024];
	double *result = new double[1024];

	Buffer *bufmat1;
	Buffer *bufmat2;
	Buffer *bufres;
//	if (ArgCount > 1)
//	{
	try
	{
		ocl->InitContext();
		ocl->DumpPlatformInfo();
		std::cout << "\nAdding kernel sources...\n";
		ocl->AddKernelSource("Kernels\\matrix_multiplication_kernel.cl");
		std::cout << "Kernel compiled !\n";
		ocl->AddKernel("Kernels\\matrix_multiplication_kernel.cl", "matrix_multiplication_kernel");
		std::cout << "Kernel ready to be queued !" << std::endl;
		bufmat1 = ocl->CreateBuffer(OpenCL::ReadOnly, 1024 * sizeof(double), mat1);
		bufmat2 = ocl->CreateBuffer(OpenCL::ReadOnly, 1024 * sizeof(double), mat2);
		bufres = ocl->CreateBuffer(OpenCL::WriteOnly, 1024 * sizeof(double), result);
		ocl->EnqueueWrite(*bufmat1, OpenCL::Blocking, 0, 1024 * sizeof(double), mat1);
		ocl->EnqueueWrite(*bufmat2, OpenCL::Blocking, 0, 1024 * sizeof(double), mat2);
		ocl->SetKernelArgument("matrix_multiplication_kernel", 0, *bufmat1);
		ocl->SetKernelArgument("matrix_multiplication_kernel", 1, *bufmat2);
		ocl->SetKernelArgument("matrix_multiplication_kernel", 2, *bufres);
		ocl->SetKernelArgument("matrix_multiplication_kernel", 3, 1024);
		ocl->SetKernelArgument("matrix_multiplication_kernel", 4, 1024);
		ocl->EnqueueKernel("matrix_multiplication_kernel", cl::NullRange, 1024, 1);
//		ocl->FinishQueue();
		std::cout << "Work finished !" << std::endl;
	}
	catch (OpenCL::Exception &e)
	{
		std::cout << "ERROR : " << e.what() << std::endl;
	}
//	}
	delete bufmat1;
	delete bufmat2;
	delete bufres;
	delete mat1;
	delete mat2;
	delete result;
	getchar();
	return (0);
}
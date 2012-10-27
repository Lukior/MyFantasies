
#include "CLCLasses/OpenCL.hpp"

int main(int ArgCount, char *ArgValues[])
{
	std::auto_ptr<OpenCL> ocl(new OpenCL());

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
		getchar();
	}
	catch (OpenCL::Exception &e)
	{
		std::cout << "ERROR : " << e.what() << std::endl;
		getchar();
	}
//	}
	return (0);
}
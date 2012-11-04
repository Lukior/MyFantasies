
#include "RapidShell.hpp"
#include <boost/lexical_cast.hpp>
#include "Timer.hpp"
#include "CLCLasses/OpenCL.hpp"

int matrix_mult(std::vector<std::string> args)
{
	if (args.size() != 4)
	{
		std::cout << "NOTICE : matrix_mult <BORDER SIZE OF MATRIXES> <NUMBER OF WORKGROUPS> <NUMBER OF WORKUNITS PER GROUP>" << std::endl;
		return -1;
	}

	int MatrixSize = boost::lexical_cast<int, std::string>(args[1]);
	MatrixSize *= MatrixSize;
	int WorkGroups = boost::lexical_cast<int, std::string>(args[2]);
	int WorkUnits = boost::lexical_cast<int, std::string>(args[3]);

	Timer t;
	std::auto_ptr<OpenCL> ocl(new OpenCL());
	double *mat1 = new double[MatrixSize];
	double *mat2 = new double[MatrixSize];
	double *result = new double[MatrixSize];

	cl::Buffer *bufmat1;
	cl::Buffer *bufmat2;
	cl::Buffer *bufres;
	try
	{
		ocl->InitContext();
		ocl->DumpPlatformInfo();
		std::cout << "\nAdding kernel sources...\n";
		ocl->AddKernelSource("Kernels\\matrix_multiplication_kernel.cl");
		std::cout << "Kernel compiled !\n";
		ocl->AddKernel("Kernels\\matrix_multiplication_kernel.cl", "matrix_multiplication_kernel");
		std::cout << "Kernel ready to be queued !" << std::endl;
		bufmat1 = ocl->CreateBuffer(OpenCL::ReadOnly, MatrixSize * sizeof(double), mat1);
		bufmat2 = ocl->CreateBuffer(OpenCL::ReadOnly, MatrixSize * sizeof(double), mat2);
		bufres = ocl->CreateBuffer(OpenCL::WriteOnly, MatrixSize * sizeof(double), result);
		t.Start();
		ocl->EnqueueWrite(*bufmat1, OpenCL::Blocking, 0, MatrixSize * sizeof(double), mat1);
		ocl->EnqueueWrite(*bufmat2, OpenCL::Blocking, 0, MatrixSize * sizeof(double), mat2);
		ocl->SetKernelArgument("matrix_multiplication_kernel", 0, *bufmat1);
		ocl->SetKernelArgument("matrix_multiplication_kernel", 1, *bufmat2);
		ocl->SetKernelArgument("matrix_multiplication_kernel", 2, *bufres);
		ocl->SetKernelArgument("matrix_multiplication_kernel", 3, 1024);
		ocl->SetKernelArgument("matrix_multiplication_kernel", 4, 1024);
		std::cout << "Enqueueing kernel" << std::endl;
		ocl->FinishQueue();
		ocl->EnqueueKernel("matrix_multiplication_kernel", cl::NullRange, WorkGroups, WorkUnits);
		ocl->FinishQueue();
		ocl->EnqueueRead(*bufres, OpenCL::Blocking, 0, MatrixSize * sizeof(double), result);
		t.Stop();
		t.PrintTime();
		std::cout << "Work finished !" << std::endl;
	}
	catch (OpenCL::Exception &e)
	{
		std::cout << "ERROR : " << e.what() << std::endl;
	}
	delete bufmat1;
	delete bufmat2;
	delete bufres;
	delete mat1;
	delete mat2;
	delete result;
	return (0);
}

int main(void)
{
	RapidShell<> rs("OpenCL Testing Program > ", false, false);

	rs.AddCommand("matrix_mult", &matrix_mult);
	rs.Run();
	return 0;
}
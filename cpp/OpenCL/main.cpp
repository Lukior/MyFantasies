
#include "CLCLasses/OpenCL.hpp"

#include <boost/gil/gil_all.hpp>

int main(int ArgCount, char *ArgValues[])
{
	std::auto_ptr<OpenCL> ocl(new OpenCL());

//	if (ArgCount > 1)
//	{
	try
	{
		ocl->InitContext(OpenCL::GPU, OpenCL::NVidia);
		ocl->DumpInfo();
		getchar();
	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
	}
//	}
	return (0);
}

#include "CLCLasses/OpenCL.hpp"

#include <boost/gil/gil_all.hpp>

int main(int ArgCount, char *ArgValues[])
{
	std::auto_ptr<OpenCL> ocl(new OpenCL());

//	if (ArgCount > 1)
//	{
	try
	{
		ocl->InitContext();
		ocl->DumpPlatformInfo();
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
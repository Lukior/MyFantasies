
#include "Classes/OpenCL.hpp"

int main()
{
	std::auto_ptr<OpenCL> ocl(new OpenCL());

	ocl->InitContext(OpenCL::GPU, OpenCL::NVidia);

	getchar();
	return (0);
}
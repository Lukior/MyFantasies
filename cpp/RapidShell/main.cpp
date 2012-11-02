
#include "Classes/RapidShell.h"

int main()
{
	RapidShell rs("Hello, I'm a shell > ", false, true);

	rs.AddCommand("HelloWorld", [] (std::string str) -> int {
		std::cout << "Hello World !" << std::endl;
		return 0;
	});
	rs.Run();
	return (0);
}
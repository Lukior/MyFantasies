
#include "Classes/RapidShell.hpp"

int main()
{
	RapidShell<> rs("Hello, I'm a shell > ", false, true);

	rs.AddCommand("HelloWorld", [] (std::vector<std::string> str) -> int {
		std::cout << "Hello World !" << std::endl;
		return 0;
	});
	rs.Run();
	return (0);
}
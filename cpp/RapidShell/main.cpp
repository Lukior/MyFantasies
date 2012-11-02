
#include "Classes/RapidShell.h"

int main()
{
	RapidShell rs("Hello, I'm a shell > ");

	rs.AddCommand("capitalize", [] (std::string str) -> int {
		for (unsigned int i = 0; i < str.size(); ++i)
			std::cout << ((str.c_str()[i] <= 122 && str.c_str()[i] >= 97)? str.c_str()[i] + 32 : str.c_str()[i]);
		std::cout << std::endl;
		return 0;
	});
	rs.Run();
	return (0);
}
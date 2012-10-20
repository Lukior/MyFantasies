
#include "GamingTools.h"

#include <iostream>

int main()
{
	CardGame cg;
	Card const *card;

	std::cout << "\n===== Testing PickCard =====\n\n";
	for (int i = 0; i < 52; ++i)
	{
		card = cg.PickCard();
		std::cout << "\npicked card " << card->GetNumber() << " of family " << card->GetFamily() << std::endl << std::endl;
		std::cout << "Remaining ";
		cg.ShowRemainingCards();
		std::cout << std::endl;
	}
	getchar();
	std::cout << "\n===== Testing DropCard =====\n\n";
	std::cout << "Dropping " << card->GetNumber() << " of " << card->GetFamily() << std::endl;
	cg.DropCard(*card);
	std::cout << "Remaining ";
	cg.ShowRemainingCards();
	getchar();
	std::cout << "\n===== Testing Reset =====\n\n";
	cg.Reset();
	cg.ShowRemainingCards();

	getchar();

	return (0);
}
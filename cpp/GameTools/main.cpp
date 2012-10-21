
#include "GamingTools.h"

#include <iostream>
#include <windows.h>

enum Modules
{
	CARD_GAME = 1
};

int SelectModuleToTest()
{
	int choice;

	while (true)
	{
		std::cout << "!! Select a module to test !!\n\n";
		std::cout << " 1 - CardGame\n\n";
		std::cout << "My choice : ";
		std::cin >> choice;
		return choice;
	}
}

void TestCardGame()
{
	CardGame cg;
	Card const *card;
	std::list<Card const *> pickedCards;

	std::cout << "\n===== Testing PickCard =====\n\n";
	for (int i = 0; i < 52; ++i)
	{
		card = cg.PickCard();
		std::cout << "\npicked card " << card->GetNumber() << " of family " << card->GetFamily() << std::endl << std::endl;
		std::cout << "Remaining cards :\n";
		cg.ShowRemainingCards();
		std::cout << std::endl;
		pickedCards.push_back(card);
		Sleep(500);
	}
	std::cout << "Press enter to continue..."; getchar();
	std::cout << "\n===== Testing DropCard =====\n\n";
	for (Card const *card: pickedCards)
	{
		std::cout << "\nDropping " << card->GetNumber() << " of " << card->GetFamily() << std::endl;
		cg.DropCard(*card);
		std::cout << "\nRemaining cards :\n";
		cg.ShowRemainingCards();
		Sleep(500);
	}
	std::cout << "Press enter to continue..."; getchar();
	std::cout << "\n===== Testing Reset =====\n\n";
	std::cout << "Emptying deck\n\n";
	cg.EmptyDeck();
	cg.ShowRemainingCards();
	Sleep(500);
	std::cout << "\nResetting deck\n\n";
	cg.ResetDeck();
	cg.ShowRemainingCards();
	std::cout << "Press enter to continue..."; getchar();
}

int main()
{
	switch (SelectModuleToTest())
	{
	case CARD_GAME: TestCardGame(); break;
	default:						return (0);
	}
	return (0);
}
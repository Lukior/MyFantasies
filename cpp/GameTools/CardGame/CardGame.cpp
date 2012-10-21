
#include "CardGame.h"

#pragma region CARD
Card::Card(string const &family, int number)
	:Family(family), Number(number) {}

string const &Card::GetFamily() const
{
	return Family;
}

int const Card::GetNumber() const
{
	return Number;
}
#pragma endregion

#pragma region CARDGAME
CardGame::CardGame()
{
	int cardNbr = 1;
	int fam = 0;
	string currentFamily[] = { "spade", "heart", "diamond", "club" };

	Cards.resize(52);
	for (int idx = 0; idx < 52; idx++)
	{
		if (cardNbr % 14 == 0)
		{
			++fam;
			cardNbr = 1;
		}
		Cards[idx].reset(new Card(currentFamily[fam], cardNbr++));
		NotPicked.push_back(idx);
	}
}

Card const *CardGame::PickCard()
{
	int pick = generator() % NotPicked.size();
	int idx = 0;
	int card;

	for (int i: NotPicked)
	{
		if (idx++ == pick)
		{
			card = i;
			break;
		}
	}
	NotPicked.remove(card);
	return (Cards[card].get());
}

void CardGame::DropCard(Card const &card)
{
	int idx = (card.GetFamily() == "spade" ? 0 : (card.GetFamily() == "heart" ? 1 : (card.GetFamily() == "diamond" ? 2 : (card.GetFamily() == "club" ? 3 : throw Error("DropCard : Unknown card family")))));
	NotPicked.push_back(card.GetNumber() + (idx * 13) - 1);
	NotPicked.sort();
	NotPicked.unique();
}

void CardGame::ShowRemainingCards()
{
	if (NotPicked.size() == 0)
		std::cout << "No more Cards" << std::endl;
	else
		for (int i: NotPicked)
			std::cout << Cards[i]->GetFamily() << " " << Cards[i]->GetNumber() << std::endl;
}

void CardGame::ShowRemainingCards(string &ToFill)
{
	if (NotPicked.size() == 0)
		ToFill = "No More Cards\n";
	else
		for (int i: NotPicked)
			ToFill += (Cards[i]->GetFamily() + " " + ("" + Cards[i]->GetNumber()) + "\n");
}

void CardGame::EmptyDeck()
{
	NotPicked.clear();
}
void CardGame::ResetDeck()
{
	NotPicked.clear();
	for (int idx = 0; idx < 52; ++idx)
		NotPicked.push_back(idx);
}
#pragma endregion

#pragma region ERROR
CardGame::Error::Error(string const &errorMsg) throw()
	:msg(errorMsg) {}

string const &CardGame::Error::what() throw()
{
	return (msg);
}
#pragma endregion
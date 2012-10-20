
#pragma once

#include <vector>
#include <list>
#include <string>
#include <memory>
#include <random>
#include <exception>
#include <iostream>

using namespace std;

class Card
{
private:
	string Family;
	int Number;

public:
	Card(string const &family, int number);

	string const &GetFamily() const;
	int const GetNumber() const;
};

class CardGame
{
private:
	vector<auto_ptr<Card>> Cards;
	list<int> NotPicked;
	random_device generator;

public:
	CardGame();

	Card const *PickCard();
	void DropCard(Card const &card);
	void ShowRemainingCards();
	void ShowRemainingCards(string &ToFill);
	void Reset();

	class Error : std::exception
	{
	private:
		string msg;

	public:
		Error(string const &errorMsg) throw();

		virtual string const &what() throw();
	};
};
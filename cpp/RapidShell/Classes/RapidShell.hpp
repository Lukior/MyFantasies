
#pragma once

#pragma region INCLUDES
#include <string>
#include <map>
#include <functional>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <locale>

#include <process.h>
#include <errno.h>

#include "boost_include.hpp"
#pragma endregion

#pragma region NAMESPACES
namespace spirit = boost::spirit;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;
#pragma endregion

#pragma region GRAMMAR DECLARATION
template <typename Iterator>
struct CommandParser
	: qi::grammar<Iterator, std::list<std::vector<std::string>>()>
{
	CommandParser()
		: CommandParser::base_type(CommandListPattern)
	{
		// StringPattern
		StringPattern =
		(
			qi::lit('"') >> qi::lexeme[*(ascii::char_ - '"')[qi::_val += qi::_1]] >> qi::lit('"')
			|
			qi::lexeme[+(ascii::char_ - '&' - ' ')[qi::_val += qi::_1]]
		);

		// CommandPattern
		CommandPattern =
		(
			StringPattern[phoenix::push_back(qi::_val, qi::_1)] >>
			*(qi::lit(' ') >> StringPattern[phoenix::push_back(qi::_val, qi::_1)])
		);

		// CommandListPattern
		CommandListPattern =
		(
			CommandPattern[phoenix::push_back(qi::_val, qi::_1)] >>
			*(qi::lit(" && ") >> CommandPattern[phoenix::push_back(qi::_val, qi::_1)])
		);
	}
	qi::rule<Iterator, std::string()> StringPattern;
	qi::rule<Iterator, std::vector<std::string>()> CommandPattern;
	qi::rule<Iterator, std::list<std::vector<std::string>>()> CommandListPattern;
};
#pragma endregion

class RapidShell
{
private:

#pragma region ATTRIBUTES
	std::string prompt;
	std::map<std::string const , std::function<int (std::vector<std::string>)>> commands;
	bool IsCaseSensitive;
	bool IsPromptSetable;
	bool IsSystemShell;
	CommandParser<std::string::iterator> Parser;
	std::list<std::vector<std::string>> CommandList;
#pragma endregion

#pragma region PRIVATE FUNCTIONS
	std::string Trim(const std::string& str, const std::string& whitespace = " \t")
	{
		const auto strBegin = str.find_first_not_of(whitespace);
		if (strBegin == std::string::npos)
			return "";

		const auto strEnd = str.find_last_not_of(whitespace);
		const auto strRange = strEnd - strBegin + 1;

		return str.substr(strBegin, strRange);
	}

	std::string Reduce(const std::string& str, const std::string& fill = " ", const std::string& whitespace = " \t")
	{
		// trim first
		auto result = Trim(str, whitespace);

		// replace sub ranges
		auto beginSpace = result.find_first_of(whitespace);
		while (beginSpace != std::string::npos)
		{
			const auto endSpace = result.find_first_not_of(whitespace, beginSpace);
			const auto range = endSpace - beginSpace;

			result.replace(beginSpace, range, fill);

			const auto newStart = beginSpace + fill.length();
			beginSpace = result.find_first_of(whitespace, newStart);
		}

		return result;
	}

	void GetArgs(std::string &command)
	{
		bool result = qi::parse(command.begin(), command.end(), Parser, CommandList);
		if (!result)
			std::cout << "ERROR : Invalid command" << std::endl;
		return;
	}
#pragma endregion

public:

#pragma region MAIN FUNCTIONS
	RapidShell(std::string const &newPrompt, bool caseSensitive = true, bool promptSetable = true, bool systemShell = true)
		:prompt(newPrompt), IsCaseSensitive(caseSensitive), IsPromptSetable(promptSetable), IsSystemShell(systemShell)
	{
		AddCommand("setprompt", [&] (std::vector<std::string> args) -> int {
			SetPrompt(args[1]);
			return 0;
		});
	}

	void Run()
	{
		std::string commandArgs;
		std::string command;

		std::cout << prompt;
		while (true && std::getline(std::cin, command))
		{
			if (!IsCaseSensitive)
				std::transform(command.begin(), command.end(), command.begin(), std::tolower);
			command = Reduce(command);

			if (command != "")
			{
				GetArgs(command);
				for (std::vector<std::string> commandSnippet: CommandList)
				{
					// BUILT-IN EXIT
					if (commandSnippet[0] == "exit")
						return;

					// COMMAND TRY
					try
					{
						commands[commandSnippet[0]](commandSnippet);
					}
					catch (...)
					{
						if (IsSystemShell)
						{
							std::string comm;
							for (std::string str: commandSnippet)
								comm += str;
							if (system(comm.c_str()) == -1)
								std::cout << strerror(errno) << std::endl;
						}
						else
							std::cout << commandSnippet[0] << " : Unknown Command" << std::endl;
					}
				}
			}

			// RESET
			command.clear();
			commandArgs.clear();
			CommandList.clear();
			std::cout << prompt;
		}
	}
#pragma endregion

#pragma region UTILITIES
	inline void SetPrompt(std::string const &newPrompt)
	{
		prompt = newPrompt + " ";
	}

	inline void AddCommand(std::string const &command, std::function<int (std::vector<std::string>)> func)
	{
		if (!IsCaseSensitive)
		{
			std::string &lowCommand = const_cast<std::string &>(command);
			std::transform(lowCommand.begin(), lowCommand.end(), lowCommand.begin(), std::tolower);
			commands[lowCommand] = func;
		}
		else
			commands[command] = func;
	}

	inline void SetStringPattern(qi::rule<std::string::iterator, std::string()> NewStringPattern)
	{
		Parser.StringPattern = NewStringPattern;
	}

	inline void SetCommandPattern(qi::rule<std::string::iterator, std::vector<std::string>()> NewCommandPattern)
	{
		Parser.CommandPattern = NewCommandPattern;
	}

	inline void SetCommandListPattern(qi::rule<std::string::iterator, std::list<std::vector<std::string>>()> NewCommandListPattern)
	{
		Parser.CommandListPattern = NewCommandListPattern;
	}
#pragma endregion

#pragma region EXCEPTION
	class Exception : std::exception
	{
	private:
		std::string msg;

	public:
		Exception(std::string const &error)
			:msg(error)
		{}

		virtual inline char const *what() const
		{
			return msg.c_str();
		}
	};
#pragma endregion
};
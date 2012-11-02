
#pragma once

#include <string>
#include <map>
#include <functional>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <locale>

class RapidShell
{
private:

#pragma region ATTRIBUTES
	std::string prompt;
	std::map<std::string const , std::function<int (std::string)>> commands;
	bool IsCaseSensitive;
	bool IsPromptSetable;
#pragma endregion

#pragma region PRIVATE FUNCTIONS
	std::string Trim(const std::string& str, const std::string& whitespace = " \t")
	{
		const auto strBegin = str.find_first_not_of(whitespace);
		if (strBegin == std::string::npos)
			return ""; // no content

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

	std::string GetArgs(std::string &command)
	{
		std::string args;

		if (command.find(" ") == std::string::npos)
			return "";
		args = command;
		args.erase(0, args.find(" ") + 1);
		command.erase(command.find(" "), command.size());
		return args;
	}
#pragma endregion

public:

	RapidShell(std::string const &newPrompt, bool caseSensitive = true, bool promptSetable = true)
		:prompt(newPrompt), IsCaseSensitive(caseSensitive), IsPromptSetable(promptSetable)
	{
	}

	void Run()
	{
		std::string commandArgs;
		std::string command;

		std::cout << prompt;
		while (true && std::getline(std::cin, command))
		{
			command = Reduce(command);
			commandArgs = GetArgs(command);

			if (!IsCaseSensitive)
				std::transform(command.begin(), command.end(), command.begin(), std::tolower);
			if (command != "")
			{
				// BUILT-IN EXIT
				if (command == "exit")
					return;

				//BUILT-IN SETPROMPT
				if (IsPromptSetable && command == "setprompt")
				{
					SetPrompt(commandArgs + " ");
					std::cout << prompt;
					continue;
				}

				// COMMAND TRY
				try
				{
					commands[command](commandArgs);
				}
				catch (...)
				{
					std::cout << command << " : Unknown command" << std::endl;
				}
			}

			// RESET
			command.clear();
			commandArgs.clear();
			std::cout << prompt;
		}
	}

	inline void SetPrompt(std::string const &newPrompt)
	{
		prompt = newPrompt;
	}

	inline void AddCommand(std::string const &command, std::function<int (std::string)> func)
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
};
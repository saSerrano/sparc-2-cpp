#include <PState.hpp>
#include <sparc2cpp.hpp>

bool sp::PState::extractPStateFromString(std::string psString)
{
	//Strip ' ' and '\n' characters
	psString.erase(std::remove(psString.begin(), psString.end(), ' '),psString.end());
	psString.erase(std::remove(psString.begin(), psString.end(), '\n'),psString.end());

	//Verify that the input string is enclosed by '{' and '}'
	if(!startsWith(psString,"{") || !endsWith(psString,"}")) return false;

	//In case of the empty answer set
	if(psString == "{}")
	{
		//A state with no literals is a valid p-state
		PStateLength = 0;
		PStateString = "";
		LitNames.clear();
		LitArgs.clear();
		LitArgArgs.clear();
		return true;
	}

	//Remove brackets
	std::string litsStr = psString.substr(1,psString.length()-2);

	//Get vector of individual literals
	std::vector<std::string> lits = splitStr(litsStr,",");

	//Correct to get the tokens that correspond to the most outer-parentesis ones, i.e., ignore commas inside parentesis
	lits = correctCommaSplit(lits);

	//Literals extraction from non-empty states
	std::vector<std::string> litNames;
	std::vector<std::vector<std::string>> litArgs;
	std::vector<std::vector<std::vector<std::string>>> litArgArgs;
	for(unsigned int i = 0; i < lits.size(); i++)
	{
		int firstParentesis =  lits[i].find("(");
		int lastParentesis = lits[i].rfind(")");

		//The literal's name
		std::string nameString = lits[i].substr(0,firstParentesis);

		//The args' names
		std::string argsStr = strBtwn(lits[i],firstParentesis,lastParentesis);
		std::vector<std::string> args = splitStr(argsStr,",");
		//Correct the comma split
		args = correctCommaSplit(args);
		//Extract the args' names
		std::vector<std::string> tempA;
		std::vector<std::vector<std::string>> tempAA;
		for(unsigned int j = 0; j < args.size(); j++)
		{
			int parIndex = args[j].find("(");
			if(parIndex != -1)
			{
				//Extraction of an argument's arguments
				int fP = parIndex;
				int lP = args[j].find(")");
				std::vector<std::string> argsOfArg = splitStr(strBtwn(args[j],fP,lP),",");

				tempA.push_back(args[j].substr(0,fP));
				tempAA.push_back(argsOfArg);
			}
			else
			{
				//This agrument has no arguments
				tempA.push_back(args[j]);
				tempAA.push_back(std::vector<std::string>());
			}
		}

		//Insert the components extracted for the literal
		litNames.push_back(nameString);
		litArgs.push_back(tempA);
		litArgArgs.push_back(tempAA);
	}

	//Save the extracted P-State
	PStateLength = litNames.size();
	PStateString = psString;
	LitNames = litNames;
	LitArgs = litArgs;
	LitArgArgs = litArgArgs;

	return true;
}

void sp::PState::pstateDisplay() const
{
	if(PStateLength > -1)
	{
		for(unsigned int i = 0; i < LitArgArgs.size(); i++)
		{
			std::cout << LitNames[i] << std::endl;
			for(unsigned int j = 0; j < LitArgArgs[i].size(); j++)
			{
				std::cout << "\t" << LitArgs[i][j] << std::endl;
				for(unsigned int k = 0; k < LitArgArgs[i][j].size(); k++)
				{
					std::cout << "\t\t" << LitArgArgs[i][j][k] << std::endl;
				}
			}
		}
	}
}

sp::PState::PState()
{
	PStateLength = -1;
	PStateString = "not-initialized";
}

sp::PState::PState(std::string psString)
{
	PState();
	extractPStateFromString(psString);
}

bool sp::PState::set(std::string psString)
{
	return extractPStateFromString(psString);
}

int sp::PState::len() const
{
	return PStateLength;
}

std::string sp::PState::str() const
{
	return PStateString;
}

void sp::PState::debug() const
{
	std::cout << "PStateLength: " << PStateLength << std::endl;
	std::cout << "PStateString: " << PStateString << std::endl;
	pstateDisplay();
}

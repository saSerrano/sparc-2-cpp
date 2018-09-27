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

bool sp::PState::equals(sp::PState const &ps2) const
{
	//First make the comparison based on their 'PStateString' member
	if(this->PStateString == ps2.PStateString) return true;

	//Must have same amount of literals
	if(this->PStateLength != ps2.PStateLength) return false;

	//For each literal in 'this-> is must also be in 'ps2'
	std::vector<int> matchedLitIndexes;
	for(int i = 0; i < this->PStateLength; i++)
	{
		std::vector<int>::iterator it;
		for(int j = 0; j < ps2.PStateLength ; j++)
		{
			//Avoid checking on literals of 'ps2' that were already matched
			it = std::find(matchedLitIndexes.begin(),matchedLitIndexes.end(),j);
			if(it != matchedLitIndexes.end()) continue;

			//Both literals have same name
			if(this->LitNames[i] != ps2.LitNames[j]) continue;

			//Both literals must have same amount of arguments
			if(this->LitArgs[i].size() != ps2.LitArgs[j].size()) continue;

			bool allArgMatch = true;
			for(unsigned int k = 0; k < this->LitArgs[i].size(); k++)
			{
				//Verify that the literals' argument correspond and have the same amount of sub-arguments
				if((this->LitArgs[i][k] != ps2.LitArgs[j][k]) ||
				   (this->LitArgArgs[i][k].size() != ps2.LitArgArgs[j][k].size()))
				{
					allArgMatch = false;
					break;
				}

				//Verify that the sub-argguments match between the literals' arguments
				for(unsigned int l = 0; l < this->LitArgArgs[i][k].size(); l++)
				{
					if(this->LitArgArgs[i][k][l] != ps2.LitArgArgs[j][k][l])
					{
						allArgMatch = false;
						break;
					}
				}

				if(!allArgMatch) break;
			}

			//The i-th literal of 'this-> matches the j-th literal of 'ps2'
			if(allArgMatch)
			{
				//Store index j to avoid checkin on 'ps2' that have already matched a literal of 'this->
				matchedLitIndexes.push_back(j);
				break;
			}
		}
	}

	//Return true if the amount of matched literals equals the total amount of literals
	if(matchedLitIndexes.size() == this->PStateLength) return true;
	else return false;
}

bool sp::PState::contains(sp::PState const &ps2) const
{
	//Every state conatins the empty state
	if(ps2.PStateString == "{}") return true;

	//Cannot contain a state that has more literals
	if(ps2.PStateLength > this->PStateLength) return false;

	//For each literal in 'ps2' check if its conatained in 'this'
	int literals_found = 0;
	for(unsigned int i = 0; i < ps2.LitNames.size(); i++)
	{
		bool literal_is_contained = false;
		for(unsigned int j = 0; j < this->LitNames.size(); j++)
		{
			bool found_literal = true;
			if(ps2.LitNames[i] == this->LitNames[j] && ps2.LitArgs[i].size() == this->LitArgs[j].size())
			{
				for(unsigned int k = 0; k < ps2.LitArgs[i].size(); k++)
				{
					if(ps2.LitArgs[i][k] == this->LitArgs[j][k] && 
					   ps2.LitArgArgs[i][k].size() == this->LitArgArgs[j][k].size())
					{
						for(unsigned int l = 0; l < ps2.LitArgArgs[i][k].size(); l++)
						{
							if(ps2.LitArgArgs[i][k][l] != this->LitArgArgs[j][k][l])
							{
								found_literal = false;
								break;
							}
						}

						if(!found_literal) break;
					}
					else found_literal = false;
				}
			}
			else found_literal = false;

			if(found_literal)
			{
				literal_is_contained = true;
				break;
			}
		}

		if(literal_is_contained) literals_found++;
	}

	return (literals_found == ps2.PStateLength);
}

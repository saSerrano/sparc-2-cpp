#include <DomainSort.hpp>
#include <PState.hpp>
#include <sparc2cpp.hpp>
#include <Transition.hpp>
#include <Action.hpp>

void sp::DomainSort::verifyAndLoad()
{
	std::ifstream inputFile;
	inputFile.open(SourceFile,std::ifstream::in);

	//Verify for a reachable file
	if(!inputFile.is_open())
	{
		SourceFile = "";
		FileIsLoaded = false;
		
		return;
	}

	//Get those lines from the file that are not comments and also non-empty
	std::string line;
	std::vector<std::string> lines;
	while(std::getline(inputFile,line)) lines.push_back(line);
	inputFile.close();
	std::vector<std::string> noCommLines;
	std::vector<int> noCommIndexes;
	for(unsigned int i = 0; i < lines.size() ; i++)
	{
		if(lines[i].find('%') == std::string::npos && lines[i] != "")
		{
			noCommLines.push_back(lines[i]);
			noCommIndexes.push_back(i);
		}
	}

	//Strip spaces and tabs
	for(unsigned int i = 0; i < noCommLines.size(); i++)
	{
		noCommLines[i] = sp::stripSpaces(noCommLines[i]);
		noCommLines[i] = sp::stripTabs(noCommLines[i]);
	}

	//Get the lines that start '#const'
	std::vector<std::string> conNames;
	std::vector<std::string> conVals;
	for(unsigned int i = 0; i < noCommLines.size(); i++)
	{
		if(noCommLines[i].find("#const") == 0)
		{
			std::vector<std::string> temp;
			temp = splitStr(noCommLines[i],"=");
			conNames.push_back(temp[0].substr(6,-1));
			conVals.push_back(temp[1].substr(0,temp[1].length()-1));
		}
	}

	//Get those lines that correspond to the definition of sorts
	std::vector<std::string> sortDefLines;
	bool sortsWasRead = false;
	bool predicatesWasRead = false;
	int predicate_line_index = 0;
	for(unsigned int i = 0; i < noCommLines.size(); i++)
	{
		if(noCommLines[i] == "sorts")
		{
			sortsWasRead = true;
			continue;
		}

		if(noCommLines[i] == "predicates")
		{
			predicate_line_index = i;
			break;
		}

		if(sortsWasRead)
		{
			if((noCommLines[i].substr(0,1) == "#") && endsWith(noCommLines[i],"."))
			{
				//Sort defined in a single line
				sortDefLines.push_back(noCommLines[i]);
			}
			else
			{
				//Make sure its an empty line, or one with only "space" characters
				for(unsigned int j = 0; j < noCommLines[i].length(); j++)
				{
					if(noCommLines[i].substr(j,1) != " ")
					{
						//Only sorts defined in a single line will be acceppted
						SourceFile = "";
						FileIsLoaded = false;
						return;
					}
				}
			}
		}
	}

	//Get those lines that correspond to the definition of a rule and concatenate them
	//in order to have a single string for each rule. Also save the line number of the
	//beginning of each rule and the amount of lines it occupies.
	bool rulesWasRead = false;
	bool readingARule = false;
	std::vector<std::string> ruleDefs;
	std::vector<std::string> ruleLines;
	std::vector<int> temp_cl_start;
	std::vector<int> temp_cl_len;
	for(unsigned int i = predicate_line_index+1; i < noCommLines.size(); i++)
	{
		if(noCommLines[i] == "rules")
		{
			rulesWasRead = true;
			continue;
		}

		if(rulesWasRead)
		{
			if(!readingARule)
			{
				//The whole rule is contained in a single line
				if(sp::endsWith(noCommLines[i],"."))
				{
					ruleDefs.push_back(noCommLines[i]);

					//Save the rule's starting line and its length
					temp_cl_start.push_back(noCommIndexes[i]);
					temp_cl_len.push_back(1);
				}
				else
				{
					//Start reading a new rule
					readingARule = true;
					ruleLines.push_back(noCommLines[i]);

					//Save the rule's starting line
					temp_cl_start.push_back(noCommIndexes[i]);
				}
			}
			else
			{
				ruleLines.push_back(noCommLines[i]);

				//The last part of the currently read rule has been gathered
				if(sp::endsWith(noCommLines[i],"."))
				{
					//Concatenate all the parts of the rule into a single string
					std::string temp_rule_line("");
					for(unsigned int j = 0; j < ruleLines.size(); j++)
					{
						temp_rule_line += ruleLines[j];
					}

					//Save the amount of lines this rule occupies in the SPARC file
					temp_cl_len.push_back(static_cast<int>(ruleLines.size()));

					//Save the rule string
					ruleDefs.push_back(temp_rule_line);
					ruleLines.clear();
					readingARule = false;
				}
			}
		}
	}

	//From the collection of rules, select only thos that correspond to a causal law, i.e. rules that:
	//	- Have a single val() literal in the rule's head
	//	- Have a "<" character and a single occurs() literal in the rule's body
	std::vector<std::string> causalHeads;
	std::vector<std::string> causalHeadFluents;
	std::vector<std::vector<std::string>> causalHeadFluentsArgs;

	std::vector<std::string> causalBodies;
	std::vector<std::string> causalBodyActions;
	std::vector<std::vector<std::string>> causalBodyActionsArgs;
	std::vector<std::vector<std::string>> causalBodyOthers;

	std::vector<int> causalLawStartLines;
	std::vector<int> causalLawLengths;
	for(unsigned int i = 0; i < ruleDefs.size(); i++)
	{
		int split_index = ruleDefs[i].find(":-");
		if(ruleDefs[i].find("val") < split_index    && //There is a 'val' in the head 
		   ruleDefs[i].find("<") > split_index      && //There is a '<' in the body
		   ruleDefs[i].find("occurs") > split_index && //There is a 'occurs' in the body
		   sp::countOcc(ruleDefs[i],"occurs") == 1 )   //There is exactly 1 'occurs' in the whole rule
		{
			//Split the causal law into: head and body
			std::vector<std::string> ruleParts = sp::splitStr(ruleDefs[i],":-");

			//Extract the causal law's head and its components
			causalHeads.push_back(ruleParts[0]);// <---- causal law's head
			int lp_index = ruleParts[0].find("(");
			int rp_index = ruleParts[0].rfind(")");
			std::vector<std::string> temp_vec = sp::splitStr(sp::strBtwn(ruleParts[0],lp_index,rp_index),",");
			temp_vec = sp::correctCommaSplit(temp_vec);
			lp_index = temp_vec[0].find("(");
			rp_index = temp_vec[0].rfind(")");
			causalHeadFluents.push_back(temp_vec[0].substr(0,lp_index));// <-- causal law head's fluent
			temp_vec = sp::splitStr(sp::strBtwn(temp_vec[0],lp_index,rp_index),",");
			causalHeadFluentsArgs.push_back(temp_vec);// <-- causal law head fluent's arguments

			//Extract the causal law's head and its components
			causalBodies.push_back(ruleParts[1]);// <--- causal law's body
			temp_vec = sp::splitStr(ruleParts[1],",");
			temp_vec = sp::correctCommaSplit(temp_vec);
			int action_index = 0;
			std::vector<std::string> others_vec;
			for(unsigned int j = 0; j < temp_vec.size(); j++)
			{
				if(temp_vec[j].find("occurs") != -1)
				{
					action_index = j;
				}
				else
				{
					//Other elements of the rule's body
					//If its the case, get rid of the '.' at the end
					if(sp::endsWith(temp_vec[j],"."))
					{
						int temp_len = temp_vec[j].length();
						others_vec.push_back(temp_vec[j].substr(0,temp_len-1));
					}
					else others_vec.push_back(temp_vec[j]);
				}
			}
			lp_index = temp_vec[action_index].find("(");
			rp_index = temp_vec[action_index].rfind(")");
			temp_vec = sp::splitStr(sp::strBtwn(temp_vec[action_index],lp_index,rp_index),",");
			temp_vec = sp::correctCommaSplit(temp_vec);
			lp_index = temp_vec[0].find("(");
			rp_index = temp_vec[0].rfind(")");
			causalBodyActions.push_back(temp_vec[0].substr(0,lp_index));// <--- causal law body's action
			temp_vec = sp::splitStr(sp::strBtwn(temp_vec[0],lp_index,rp_index),",");
			causalBodyActionsArgs.push_back(temp_vec);// <--- causal law body action's arguments
			causalBodyOthers.push_back(others_vec);// <--- causal law body other elements

			//Save the starting line and the amount of lines this causal law occupies in the SPARC file
			causalLawStartLines.push_back(temp_cl_start[i]);
			causalLawLengths.push_back(temp_cl_len[i]);
		}
	}

	//Get the sort's names and their respective elements
	std::vector<std::string> sortNames;
	std::vector<std::vector<std::string>> sortElements;
	std::vector<std::vector<std::vector<std::string>>> sortArguments;
	for(unsigned int i = 0; i < sortDefLines.size(); i++)
	{
		//Delete the '#' and '.' at the beginning and the end
		int lineLen = sortDefLines[i].length();
		sortDefLines[i].erase(lineLen-1,1);
		sortDefLines[i].erase(0,1);

		//Get both substrings separated by '='
		std::vector<std::string> sortComp = splitStr(sortDefLines[i],"=");
		if(sortComp.size() != 2)
		{
			//Sorts must be composed of 2 parts separated by '='
			SourceFile = "";
			FileIsLoaded = false;
			return;
		}

		//Insert the sort name
		sortNames.push_back(sortComp[0]);

		//Get sort individual elements
		if(sortComp[1].find("..") != -1)
		{
			//Enlist explictly the elements of this sort
			std::vector<std::string> numSort = splitStr(sortComp[1],"..");
			int minVal = std::atoi(numSort[0].c_str());
			int maxVal = -1;
			if(isInt(numSort[1])) maxVal = std::atoi(numSort[1].c_str());
			else
			{
				for(unsigned int j = 0; j < conNames.size(); j++)
				{
					if(conNames[j] == numSort[1])
					{
						maxVal = std::atoi(conVals[j].c_str());
						break;
					}
				}
			}

			//Insert the numeric sort elements
			std::vector<std::string> numElems;
			std::vector<std::vector<std::string>> numArgs;
			for(int j = minVal; j <= maxVal ; j++)
			{
				numElems.push_back(std::to_string(j));
				numArgs.push_back(std::vector<std::string>());
			}
			sortElements.push_back(numElems);//insert vector of sort elements
			sortArguments.push_back(numArgs);//insert vector of vectors of arguments for those sort elements
		}
		else
		{
			std::vector<std::string> sortElem = splitStr(sortComp[1],"+");
			std::vector<std::string> subsortElem;
			std::vector<std::vector<std::string>> subsortArgs;
			for(unsigned int j = 0; j < sortElem.size(); j++)
			{
				if(sortElem[j].find("{") == 0 && sortElem[j].find("}") == sortElem[j].length()-1)
				{
					//Comma separated sort elements
					std::vector<std::string> subElem;
					subElem = splitStr(sortElem[j].substr(1,sortElem[j].length()-2),",");
					subsortElem.insert(subsortElem.end(),subElem.begin(),subElem.end());
					for(unsigned int k = 0; k < subElem.size(); k++)
					{
						subsortArgs.push_back(std::vector<std::string>());
					}
				}
				else if(sortElem[j].substr(0,1) == "#")
				{
					//An already defined sort
					subsortElem.push_back(sortElem[j].substr(1,-1));
					subsortArgs.push_back(std::vector<std::string>());
				}
				else if(sortElem[j].find("(") != -1 && sortElem[j].find(")") != -1)
				{
					//A sort with input arguments (other sorts)
					int firstParentesis = sortElem[j].find("(");
					int secondParentesis = sortElem[j].find(")");
					std::string sortString = sortElem[j].substr(0,firstParentesis);
					std::string argsString = sortElem[j].substr(firstParentesis+1,secondParentesis-firstParentesis-1);
					std::vector<std::string> tempArgs = splitStr(argsString,",");
					for(unsigned int k = 0; k < tempArgs.size(); k++) tempArgs[k] = tempArgs[k].substr(1,-1);

					subsortElem.push_back(sortString);
					subsortArgs.push_back(tempArgs);
				}
			}
			sortElements.push_back(subsortElem);//insert vector of sort elements
			sortArguments.push_back(subsortArgs);//insert vector of vectors of arguments for those sort elements
		}
	}

	//Get the indexes to construct a 'sub-sort' based hierarchy
	std::vector<std::vector<int>> elemIndexes;
	std::vector<std::vector<std::vector<int>>> argsIndexes;
	std::vector<int> tempEI;
	std::vector<std::vector<int>> tempAI;
	//Indexes of sort elements
	for(unsigned int i = 0; i < sortElements.size(); i++)
	{
		tempEI.clear();
		for(unsigned int j = 0; j < sortElements[i].size(); j++)
		{
			int foundIndex = -1;
			for(unsigned int k = 0; k < sortNames.size(); k++)
			{
				if(sortElements[i][j] == sortNames[k])
				{
					foundIndex = k;
					break;
				}
			}
			tempEI.push_back(foundIndex);
		}
		elemIndexes.push_back(tempEI);
	}
	//Indexes of sort arguments
	for(unsigned int i = 0; i < sortArguments.size(); i++)
	{
		tempAI.clear();
		for(unsigned int j = 0; j < sortArguments[i].size(); j++)
		{
			std::vector<int> ttAI;
			for(unsigned int k = 0; k < sortArguments[i][j].size(); k++)
			{
				int foundIndex = -1;
				for(unsigned int l = 0; l < sortNames.size(); l++)
				{
					if(sortArguments[i][j][k] == sortNames[l])
					{
						foundIndex = l;
						break;
					}
				}
				ttAI.push_back(foundIndex);
			}
			tempAI.push_back(ttAI);
		}
		argsIndexes.push_back(tempAI);
	}

	//Save the structures extracted from the sorts
	Consts = conNames;
	ConstVals = conVals;
	Sorts = sortNames;
	SortElements = sortElements;
	SortArguments = sortArguments;
	SubsortElemIndexes = elemIndexes;
	SubsortArgsIndexes = argsIndexes;
	FileIsLoaded = true;

	//Save the structures extracted from the causal laws
	CausalHeadFluents = causalHeadFluents;
	CausalHeadFluentsArgs = causalHeadFluentsArgs;
	CausalBodyActions = causalBodyActions;
	CausalBodyActionsArgs = causalBodyActionsArgs;
	CausalBodyOthers = causalBodyOthers;
	CausalLawStartLines = causalLawStartLines;
	CausalLawLengths = causalLawLengths;

	return;
}

void sp::DomainSort::sortDisplay() const
{
	for(unsigned int i = 0; i < Sorts.size(); i++)
	{
		std::cout << i << ":" + Sorts[i] + "\n";
		for(unsigned int j = 0; j < SortElements[i].size(); j++)
		{
			std::cout << "\t" << SubsortElemIndexes[i][j] << ":" + SortElements[i][j] + "\n";
			for(unsigned int k = 0; k < SortArguments[i][j].size(); k++)
			{
				std::cout << "\t\t" << SubsortArgsIndexes[i][j][k] << ":" + SortArguments[i][j][k] + "\n";
			}
		}
	}
	std::cout << "Source file >> " + SourceFile << std::endl;
	std::cout << "File is loaded >> " << FileIsLoaded << std::endl;
}

void sp::DomainSort::hierDisplay() const
{
	for(unsigned int i = 0; i < SubsortElemIndexes.size(); i++)
	{
		std::cout << "sort " + std::to_string(i) + ":"<< std::endl;

		for(unsigned int j = 0; j < SubsortElemIndexes[i].size(); j++)
		{
			std::cout << "\t" + std::to_string(SubsortElemIndexes[i][j]) << std::endl;

			for(unsigned int k = 0; k < SubsortArgsIndexes[i][j].size(); k++)
			{
				std::cout << "\t\t" + std::to_string(SubsortArgsIndexes[i][j][k]) << std::endl;
			}
		}
	}
}

void sp::DomainSort::causalDisplay() const
{
	for(unsigned int i = 0; i < CausalHeadFluents.size(); i++)
	{
		std::cout << "*********************************************" << std::endl;
		std::cout << "Causal law head fluent: " + CausalHeadFluents[i] << std::endl;
		std::cout << "Causal law head fluent args: ";
		for(unsigned int j = 0; j < CausalHeadFluentsArgs[i].size(); j++)
		{
			if(j == 0) std::cout << CausalHeadFluentsArgs[i][j];
			else std::cout << ", " + CausalHeadFluentsArgs[i][j];
		}
		std::cout << std::endl;
		std::cout << "Causal law body action:" + CausalBodyActions[i] << std::endl;
		std::cout << "Causal law body action args: ";
		for(unsigned int j = 0; j < CausalBodyActionsArgs[i].size(); j++)
		{
			if(j == 0) std::cout << CausalBodyActionsArgs[i][j];
			else std::cout << ", " + CausalBodyActionsArgs[i][j];
		}
		std::cout << std::endl;
		std::cout << "Causal law body other elements: ";
		for(unsigned int j = 0; j < CausalBodyOthers[i].size(); j++)
		{
			if(j == 0) std::cout << CausalBodyOthers[i][j];
			else std::cout << ", " + CausalBodyOthers[i][j];

		}
		std::cout << std::endl;
		std::cout << "Starting line:" + std::to_string(CausalLawStartLines[i]) << std::endl;
		std::cout << "Amount of line occupied:" + std::to_string(CausalLawLengths[i]) << std::endl;
		std::cout << std::endl;
		std::cout << "*********************************************" << std::endl;
	}
}

sp::DomainSort::DomainSort()
{
	FileIsLoaded = false;
	SourceFile = "";
}

sp::DomainSort::DomainSort(std::string SourceFile)
{
	DomainSort();
	this->SourceFile = SourceFile;
	verifyAndLoad();
}

bool sp::DomainSort::loadFile(std::string SourceFile)
{
	this->SourceFile = SourceFile;
	verifyAndLoad();
	return FileIsLoaded;
}

bool sp::DomainSort::fileIsLoaded() const
{
	return FileIsLoaded;
}

std::string sp::DomainSort::getFile() const
{
	return SourceFile;
}

void sp::DomainSort::debug() const
{
	//sortDisplay();
	//hierDisplay();
	causalDisplay();
}

std::vector<std::vector<sp::PState>> sp::DomainSort::generateStates(std::string const &rootGenSort, std::vector<std::string> const &subStateDefs) const
{
	/*

	This method assumes that the SPARC file loaded to generate this DomainSort:

	- Has predicate: val(#fluent,#boolean,#step)
	- Has sort: #boolean = {true,false}
	- Has sort: #step = 0..max_step
	- Has sort: #defined_f = 

	*/

	//Verification of input parameters
	if(!FileIsLoaded) return std::vector<std::vector<sp::PState>>();
	if(rootGenSort.length() <= 1) return std::vector<std::vector<sp::PState>>();
	if(!sp::startsWith(rootGenSort,"#")) return std::vector<std::vector<sp::PState>>();

	//Gather all the sorts that belong to 'rootGenSort' or to one of its sub-sorts,
	// and their respectie arguments
	int rootIndex = -1;
	for(unsigned int i = 0; i < Sorts.size(); i++)
	{
		if(Sorts[i] == rootGenSort.substr(1,-1))
		{
			rootIndex = i;
			break;
		}
	}
	if(rootIndex == -1) return std::vector<std::vector<sp::PState>>();
	std::vector<std::string> gathElements;
	std::vector<std::vector<std::string>> gathArguments;
	for(unsigned int i = 0; i <= rootIndex ; i++)
	{
		for(unsigned int j = 0; j < SortElements[i].size(); j++)
		{
			if((SubsortElemIndexes[i][j] == -1) && sp::recursiveCheck((*this),SortElements[i][j],rootIndex))
			{
				gathElements.push_back(SortElements[i][j]);
				if(SortArguments[i][j].size() > 0)
				{
					std::vector<std::string> temp_gathArguments;
					for(unsigned int k = 0; k < SortArguments[i][j].size(); k++)
					{
						temp_gathArguments.push_back(SortArguments[i][j][k]);
					}
					gathArguments.push_back(temp_gathArguments);
				}
				else gathArguments.push_back(std::vector<std::string>());
			}
		}
	}

	//For each of the gathered elements that have arguments, generate:
	//	- A defined fluent
	// 	- The defined fluent's definition in the form of a SPARC rule
	//	- A states generating rule (SPARC choice rule)
	std::vector<std::string> dfName;
	std::vector<std::string> dfDef;
	std::vector<std::string> dfGenRule;
	for(unsigned int i = 0; i < gathElements.size(); i++)
	{
		std::string temp_dfName("");
		std::string temp_dfDef("");
		std::string temp_dfGenRule("");

		//Name of the 'part of state' defined fluent
		temp_dfName = "ds_part_of_pstate_" + gathElements[i];

		//Definition of the defined fluent
		temp_dfDef = "val(" + temp_dfName + "(";
		for(unsigned int j = 0; j < gathArguments[i].size(); j++)
		{
			if(j == 0) temp_dfDef = temp_dfDef + "V" + std::to_string(j);
			else temp_dfDef = temp_dfDef + ",V" + std::to_string(j);
		}
		temp_dfDef = temp_dfDef + "),true,I) :- ";
		for(unsigned int j = 0; j < gathArguments[i].size(); j++)
		{
			temp_dfDef = temp_dfDef + "#" + gathArguments[i][j] + "(V" + std::to_string(j) + ")";
			if(j == static_cast<int>(gathArguments[i].size()-1)) temp_dfDef = temp_dfDef + ".";
			else temp_dfDef = temp_dfDef + ",";
		}

		//Definition of the state generating rule for this fluent
		temp_dfGenRule = "{val(" + gathElements[i] + "(";
		for(unsigned int j = 0; j < gathArguments[i].size(); j++)
		{
			if(j == 0) temp_dfGenRule = temp_dfGenRule + "V" + std::to_string(j);
			else temp_dfGenRule = temp_dfGenRule + ",V" + std::to_string(j);
		}
		temp_dfGenRule = temp_dfGenRule + "),true,0):val(" + temp_dfName + "(";
		for(unsigned int j = 0; j < gathArguments[i].size(); j++)
		{
			if(j == 0) temp_dfGenRule = temp_dfGenRule + "V" + std::to_string(j);
			else temp_dfGenRule = temp_dfGenRule + ",V" + std::to_string(j);
		}
		temp_dfGenRule = temp_dfGenRule + "),true,0)}.";

		//Save the definitions
		dfName.push_back(temp_dfName);
		dfDef.push_back(temp_dfDef);
		dfGenRule.push_back(temp_dfGenRule);
	}

	//Insert the 'part of state' defined fluents in a copy of the original SPARC file
	//to generate the the p-states

	//Create a copy file from 'SourceFile'
	std::vector<std::string> path_vec = sp::splitStr(SourceFile,"/");
	int last_index = path_vec.size() - 1;
	std::vector<std::string> temp_vec = sp::splitStr(path_vec[last_index],".");
	std::string cpFile = temp_vec[0] + "_GEN_STATES.sp";

	//Add the defined fluents
	std::ifstream inputFile;
	std::ofstream outputFile;
	inputFile.open(SourceFile,std::ifstream::in);
	outputFile.open(cpFile,std::ofstream::out);
	std::string temp_line;
	while(std::getline(inputFile,temp_line))
	{
		if(sp::startsWith(temp_line,"#defined_f"))
		{
			std::vector<std::string> temp_vec = sp::splitStr(temp_line,".");
			temp_line = temp_vec[0];
			//Append the defined fluents
			for(unsigned int i = 0; i < dfName.size(); i++)
			{
				temp_line = temp_line + " + " + dfName[i] + "(";
				for(unsigned int j = 0; j < gathArguments[i].size(); j++)
				{
					if(j == 0) temp_line = temp_line + "#" + gathArguments[i][j];
					else temp_line = temp_line + ",#" + gathArguments[i][j];
				}
				temp_line = temp_line + ")";
			}
			temp_line = temp_line + ".";

			outputFile << temp_line << std::endl;
		}
		else if(sp::startsWith(temp_line,"display")) break;
		else outputFile << temp_line << std::endl;
	}
	inputFile.close();

	//Add the defined fluents' rules
	for(unsigned int i = 0; i < dfDef.size(); i++)
	{
		outputFile << dfDef[i] << std::endl;
	}

	//Add the p-state generating rules
	for(unsigned int i = 0; i < dfGenRule.size(); i++)
	{
		outputFile << dfGenRule[i] << std::endl;
	}

	//Add the filter to display only the literals of interest
	outputFile << "display" << std::endl;
	for(unsigned int i = 0; i < gathElements.size(); i++)
	{
		std::string disp_str;
		disp_str = "val("+gathElements[i]+"(";
		for(unsigned int j = 0; j < gathArguments[i].size(); j++)
		{
			if(j == 0) disp_str = disp_str + "V" + std::to_string(j);
			else disp_str = disp_str + ",V" + std::to_string(j);
		}
		disp_str = disp_str + "),true,0).";

		outputFile << disp_str << std::endl;
	}
	outputFile.close();

	//Generate the p-states answer-sets file
	sp::shellCmd("java -jar "+std::string(PATH_TO_SPARC)+" "+cpFile+" -A > STATES_AS.txt");
	//Extract the p-states from the answer-sets file
	std::vector<sp::PState> extPS = sp::pstatesFromFile("STATES_AS.txt");

	//From the whole set of p-states keep those with the largest length
	int max_len = -1;
	std::vector<sp::PState> largePS;
	for(unsigned int i = 0; i < extPS.size(); i++) if(extPS[i].len() > max_len) max_len = extPS[i].len();
	for(unsigned int i = 0; i < extPS.size(); i++) if(extPS[i].len() == max_len) largePS.push_back(extPS[i]);

	//From the p-states with the largest length, for each definition in 'subStateDefs' extract sub-pstates
	//that satisfy such definition
	std::vector<std::vector<sp::PState>> outputAnswerSets;
	for(unsigned int i = 0; i < subStateDefs.size(); i++)
	{
		std::vector<sp::PState> temp_as;
		std::vector<std::string> temp_def;
		temp_def.push_back(subStateDefs[i]);
		for(unsigned int j = 0; j < largePS.size(); j++)
		{
			sp::PState temp_ps = sp::subPstate((*this),largePS[j],temp_def);
			temp_as.push_back(temp_ps);
		}
		outputAnswerSets.push_back(temp_as);
	}
	//Always include the original whole Answer sets
	outputAnswerSets.push_back(largePS);

	return outputAnswerSets;
}

std::vector<sp::Action> sp::DomainSort::generateActions(std::string const &rootGenSort) const
{
	//Output list of generated actions
	std::vector<sp::Action> listOfActions;

	//Only if a SPARC has been loaded successfully this method can generate actions
	if(!FileIsLoaded) return listOfActions;

	//Input parameter checking
	if(rootGenSort.length() <= 1) return listOfActions;
	if(!sp::startsWith(rootGenSort,"#")) return listOfActions;

	//Get the sort index that corresponds to sort rootGenSort
	int rootIndex = -1;
	for(unsigned int i = 0; i < Sorts.size(); i++)
	{
		if(rootGenSort.substr(1,-1) == Sorts[i])
		{
			rootIndex = i;
			break;
		}
	}
	if(rootIndex == -1) return listOfActions;

	//Gather list of actions that are elements of rootGenSort or to one of its sub-sorts and
	//have a causal law that describe their effects
	std::vector<std::string> gathActions;
	std::vector<std::vector<std::string>> gathActionArgs;
	for(unsigned int i = 0; i < SortElements.size(); i++)
	{
		for(unsigned int j = 0; j < SortElements[i].size(); j++)
		{
			if(SubsortElemIndexes[i][j] == -1 && 
			   sp::recursiveCheck((*this),SortElements[i][j],rootIndex))
			{
				for(unsigned int k = 0; k < CausalBodyActions.size(); k++)
				{
					if(SortElements[i][j] == CausalBodyActions[k] &&
					   SortArguments[i][j].size() == CausalBodyActionsArgs[k].size())
					{
						gathActions.push_back(SortElements[i][j]);
						gathActionArgs.push_back(SortArguments[i][j]);
					}
				}
			}
		}
	}

	//For each action with a causal law, generate all the possible combinations
	//of input parameters it can receive
	std::vector<std::vector<std::string>> actionParamCombs;
	for(unsigned int i = 0; i < gathActionArgs.size(); i++)
	{
		std::vector<std::vector<std::string>> params_values;
		for(unsigned int j = 0; j < gathActionArgs[i].size(); j++)
		{
			//Get all the constant objects that can be inserted in the input param gathActionArgs[i][j]
			std::vector<std::string> temp_values;
			//Get the root index for this param in the sort structure
			int param_root_index = -1;
			for(unsigned int k = 0; k < Sorts.size(); k++)
			{
				if(gathActionArgs[i][j] == Sorts[k])
				{
					param_root_index = k;
					break;
				}
			}
			for(unsigned int k = 0; k < SortElements.size(); k++)
			{
				for(unsigned int l = 0; l < SortElements[k].size(); l++)
				{
					if(SubsortElemIndexes[k][l] == -1 &&
					   sp::recursiveCheck((*this),SortElements[k][l],param_root_index))
					{
						temp_values.push_back(SortElements[k][l]);
					}
				}
			}
			//Save the lis of possible values
			params_values.push_back(temp_values);
		}

		//Compute the total amount of combinations and initialize a vector of empty strings
		int total_combs = 1;
		for(unsigned int j = 0; j < params_values.size(); j++)
			total_combs *= static_cast<int>(params_values[j].size());
		std::vector<std::string> temp_param_comb(total_combs,"");

		//Generate the parameter combinitations
		int times_each_val = total_combs;
		for(unsigned int j = 0; j < params_values.size(); j++)
		{
			//Compute how many times a value wil be written uninterrupted in each cycle
			times_each_val /= static_cast<int>(params_values[j].size());
			//Compute how many cycles
			int repetitions = total_combs / (times_each_val * static_cast<int>(params_values[j].size()));

			for(int k = 0; k < repetitions; k++)
			{
				for(unsigned int l = 0; l < params_values[j].size(); l++)
				{
					for(int m = 0; m < times_each_val; m++)
					{
						int abs_index = k*(total_combs/repetitions) +
								l*times_each_val + 
								m;
						//The first parameter does not have a comma in front of it
						if(j == 0) temp_param_comb[abs_index] += params_values[j][l];
						else temp_param_comb[abs_index] += (","+params_values[j][l]);
					}
				}
			}
		}

		//Save the list of param-value combinations for this action
		actionParamCombs.push_back(temp_param_comb);
	}

	//Insert each parameter into a valid SPARC action occurrence, that is, of the form
	//	occurs(action_name(param0,param1,...,paramn),0)
	//And use that to create an Action object
	for(unsigned int i = 0; i < actionParamCombs.size(); i++)
	{
		for(unsigned int j = 0; j < actionParamCombs[i].size(); j++)
		{
			std::string init_str = "occurs(" + gathActions[i] + "(" + actionParamCombs[i][j] +"),0)";
			std::vector<std::string> init_vec;
			init_vec.push_back(init_str);
			sp::Action temp_action(init_vec);
			listOfActions.push_back(temp_action);
		}
	}

	return listOfActions;
}

std::vector<sp::Transition> sp::DomainSort::generateTransitions(std::vector<sp::PState> const &setS0, std::vector<sp::Action> &setA, std::string const &rootDisplaySort)
{
	/*

	This method assumes that the SPARC file loaded to generate this DomainSort:

	 - For each action, it has a defined fluent that defines the set of possible outcomes
	   an action can result in. This fluent is defined by the user, and must 

			defined fluent for possible outcome:

				actionname_pos_outcome(#arg1,#arg2,...)

	The list of args of this def. fluent must match those of the inertial fluent the action
	modifies. Also, the first argument of the inertial and defined fluent, must be of the
	same sort than the action, for instance,

			inertial fluent: location(#robot,#cell)
			defined fluent: move_left_pos_outcome(#robot,#cell)
			action: move_left(#robot)

	Thus, this method does not verifies if the the correspondant defined fluent for each
	action actually exists, it is responsability of the SPARC coder to provide definition
	of these defined fluents.

	*/

	/*

	* ISSUES WITH NON-DETERMINISTIC CAUSAL LAWS

	When a causal law is being extended to its non-deterministic version, a defined fluent 
	is created for the purpose of defining the set of possible outcomes for the particular 
	occurrences of the action.

	Once this defined fluent has been created, there are two sets of variables that are of 
	our particular interest:
		- The variables in the head of the causal law (set A)
		- The variables that are parameters of the action (set B)

	It is clear that the variables in the fluent that is a result of executing the action 
	and the variables of the defined fluent must have a 1-to-1 correspondance. However, it 
	is not obvious the way the varibles from set A and set B relate to each other.

	In order for a system to automatically generate transitions of states using non-deterministic 
	causal laws (with choice rules), it is necessary to homogenize the way causal laws are written. 
	By homogenizing causal law's structures, we provide a standard rule on the correspondance of 
	variables in set A and set B.

	As an attempt to homogenize causal laws, we provide a standard that described as follows:

	  i) The list of input parameters of an action and the fluent it modifies, must be share 
	the first type of input parameter, therefore, they can have different lists of input 
	parameters. For instance,

					location(#robot,#cell) and move_left(#robot)

	 ii) All inertial fluents (those modified by actions) are interpreted as properties of an 
	object. In our homogenization, the object this properties describe is always the first 
	variable of the fluent, that is,

					Property(Object,Prop_param1,Prop_param1,...)

	iii) When generating a non-deterministic causal law, the correspondance between variables 
	of set A and those of set B, is defined only by a 1-to-1 correspondance between the first 
	parameter of A and B. The rest of the parameters are completely unrelated. For instance,

	Example 1.
	{val(on(B,V0),true,I+1) : val(poss_outcome_put(B,V0),true,I)} :- I < n, occurs(put(B,L),I).

	Example 2.
	{val(location(R,V0),true,I+1) : val(poss_outcome_move_left(R,V0),true,I)} :- I < n, occurs(move_left(R),I).

	*/

	//Vector with hold the generated transitions
	std::vector<sp::Transition> outputTrans;

	//Initial checking on input parameters
	if(!FileIsLoaded) return outputTrans;
	if(setS0.size() == 0) return outputTrans;
	if(setA.size() == 0) return outputTrans;
	if(rootDisplaySort.length() <= 1) return outputTrans;
	if(!sp::startsWith(rootDisplaySort,"#")) return outputTrans;
	int disp_root_index = -1;
	for(unsigned int i = 0; i < Sorts.size(); i++)
	{
		if(rootDisplaySort.substr(1,-1) == Sorts[i])
		{
			disp_root_index = i;
			break;
		}
	}
	if(disp_root_index == -1) return outputTrans;

	//Create a copy file from 'SourceFile'
	std::vector<std::string> path_vec = sp::splitStr(SourceFile,"/");
	int last_index = path_vec.size() - 1;
	std::vector<std::string> temp_vec = sp::splitStr(path_vec[last_index],".");
	std::string cpFile =  temp_vec[0] + "_GEN_TRANS.sp";

	//Replace the causal laws by their non-deterministic version
	std::ifstream inputFile;
	std::ofstream outputFile;
	inputFile.open(SourceFile,std::ifstream::in);
	outputFile.open(cpFile,std::ofstream::out);
	std::string temp_line;
	int ln_counter = 0;
	int cl_counter = 0;
	while(std::getline(inputFile,temp_line))
	{
		if(CausalLawStartLines.size() > 0)
		{
			//Look for the beggining of causal laws
			if(ln_counter == CausalLawStartLines[cl_counter])
			{
				//Ignore the lines that correspond a causal law
				for(int i = 0; i < CausalLawLengths[cl_counter]-1; i++)
				{
					std::getline(inputFile,temp_line);
					ln_counter++;
				}

				//Generate the non-determinitsic causal law
				std::string ndcl_str("1{val(");
				ndcl_str += CausalHeadFluents[cl_counter]+"(X";
				for(int i = 1; i < CausalHeadFluentsArgs[cl_counter].size(); i++)
					ndcl_str += (",V" + std::to_string(i));
				ndcl_str += ("),true,I+1):");
				ndcl_str += ("val(" + CausalBodyActions[cl_counter] + "_pos_outcome(X");
				for(unsigned int i = 1; i < CausalHeadFluentsArgs[cl_counter].size(); i++)
					ndcl_str += (",V" + std::to_string(i));
				ndcl_str += ("),true,I)}1:-\n\t");
				for(unsigned int i = 0; i < CausalBodyOthers[cl_counter].size(); i++)
					ndcl_str += (CausalBodyOthers[cl_counter][i] + ",\n\t");
				ndcl_str += ("occurs(" + CausalBodyActions[cl_counter] + "(X");
				for(int i = 1; i < CausalBodyActionsArgs[cl_counter].size(); i++)
					ndcl_str += (",X" + std::to_string(i));
				ndcl_str += "),I).";
				outputFile << ndcl_str << std::endl;

				//Increment counter to replace the next causal law
				cl_counter++;
			}
			else if(sp::startsWith(temp_line,"display")) break;
			else outputFile << temp_line << std::endl;
		}
		else if(sp::startsWith(temp_line,"display")) break;
		else outputFile << temp_line << std::endl;

		ln_counter++;
	}
	inputFile.close();
	outputFile.close();

	//For each state and each action generate a single string, that will be re-used
	//when adding them to a SPARC FILE
	std::vector<std::string> states_list;
	std::vector<std::string> actions_list;
	for(unsigned int i = 0; i < setS0.size(); i++)
	{
		std::string state_str("");
		//Write the i-th state's literals
		for(unsigned int k = 0; k < setS0[i].LitNames.size(); k++)
		{
			std::string lit_str("");
			lit_str += (setS0[i].LitNames[k] + "(");
			for(unsigned int l = 0; l < setS0[i].LitArgs[k].size(); l++)
			{
				if(l > 0) lit_str += ",";
				lit_str += setS0[i].LitArgs[k][l];
				if(setS0[i].LitArgArgs[k][l].size() > 0) lit_str += "(";
				for(unsigned int m = 0; m < setS0[i].LitArgArgs[k][l].size(); m++)
				{
					if(m > 0) lit_str += ",";
					lit_str += setS0[i].LitArgArgs[k][l][m];
				}
				if(setS0[i].LitArgArgs[k][l].size() > 0) lit_str += ")";
			}
			lit_str += ").";

			//Write the k-th literal of the i-th state
			state_str += (lit_str + "\n");
		}

		//Save the state string
		states_list.push_back(state_str);
	}
	for(unsigned int i = 0; i < setA.size(); i++)
	{
		std::string act_str("");
		//Write all the actions that constitute the Action object
		for(unsigned int j = 0; j < setA[i].ActionNames.size(); j++)
		{
			act_str += ("occurs(" + setA[i].ActionNames[j]);
			if(setA[i].ActionArgs[j].size() > 0) act_str += "(";
			for(unsigned int k = 0; k < setA[i].ActionArgs[j].size(); k++)
			{
				if(k > 0) act_str += ",";
				act_str += setA[i].ActionArgs[j][k];
			}
			if(setA[i].ActionArgs[j].size() > 0) act_str += ")";

			act_str += ",0).\n";
		}

		//Save the action string
		actions_list.push_back(act_str);
	}

	//Generate the list of fluents to be displayed at the output
	std::vector<std::string> display_list;
	for(unsigned int i = 0; i <= disp_root_index; i++)
	{
		for(unsigned int j = 0; j < SortElements[i].size(); j++)
		{
			if(SubsortElemIndexes[i][j] == -1)
			{
				if(sp::recursiveCheck((*this),SortElements[i][j],disp_root_index))
				{
					//Add this fluent to the display list
					std::string dis_str("");
					dis_str += ("val(" + SortElements[i][j] + "(");
					for(unsigned int k = 0; k < SortArguments[i][j].size(); k++)
					{
						if(k > 0) dis_str += ",";
						dis_str += ("V" + std::to_string(k));
					}
					dis_str += "),true,1).";

					//Save the display string
					display_list.push_back(dis_str);
				}
			}
		}
	}

	// For each pair state-action, append them to the base GEN_TRANS SPARC file
	// and compute their answer-sets to obtain the reached states
	std::vector<bool> actionReachedState(static_cast<int>(setA.size()),false);
	sp::shellCmd("rm -rf gentrans_files;mkdir gentrans_files");
	sp::shellCmd("rm -rf gentrans_as;mkdir gentrans_as");
	for(unsigned int i = 0; i < states_list.size(); i++)
	{
		for(unsigned int j = 0; j < actions_list.size(); j++)
		{
			//Create temporary copy of '_GEN_TRANS.sp' file
			std::string safile("gentrans_files/s"+std::to_string(i)+"a"+std::to_string(j)+"_"+cpFile);
			std::string cmd("cp " + cpFile + " " + safile);
			sp::shellCmd(cmd);

			//Append to the temporary copy the i-th state and j-th action
			std::ofstream saFile;
			saFile.open(safile,std::ofstream::app);
			saFile << "\%---------STATE-" << i << std::endl;
			saFile << states_list[i] << std::endl;
			saFile << "\%---------ACTION-" << j << std::endl;
			saFile << actions_list[j] << std::endl;

			//Add lines to display only states reached by executing j-th action
			saFile << "display" << std::endl;
			for(unsigned int k = 0; k < display_list.size(); k++)
				saFile << display_list[k] << std::endl;
			saFile.close();

			//Solve the ASP program by compiling the SPARC file
			std::string tas_file("gentrans_as/s"+std::to_string(i)+"a"+std::to_string(j)+"_TRANS_AS.txt");
			sp::shellCmd("java -jar " + std::string(PATH_TO_SPARC) + " " + safile + " -A > " + tas_file);
			//Extract the reached states from the answer-sets file
			std::vector<sp::PState> setS1 = sp::pstatesFromFile(tas_file);

			//Build the transitions from:
			// - Starting state i
			// - Action j
			// - Reached state k
			if(setS1.size() == 0) continue;
			else
			{
				actionReachedState[j] = true;
				for(unsigned int k = 0; k < setS1.size(); k++)
				{
					sp::Transition saT(setA[j],setS0[i],setS1[k]);
					outputTrans.push_back(saT);
				}
			}
		}
	}

	//Delete from the vector of actions, those that did not reach a state
	//from any state if S0
	std::vector<bool>::iterator it;
	while(true)
	{
		it = std::find(actionReachedState.begin(),actionReachedState.end(),false);
		if(it == actionReachedState.end()) break;
		for(unsigned int i = 0; i < actionReachedState.size(); i++)
		{
			if(!actionReachedState[i])
			{
				actionReachedState.erase(actionReachedState.begin()+i);
				setA.erase(setA.begin()+i);
			}
		}
	}

	// Assign IDs to the states and actions of each transition, with respect to
	// the steS0 and setA vectors
	for(unsigned int i = 0; i < outputTrans.size(); i++)
	{
		outputTrans[i].setStatesID(setS0);
		outputTrans[i].setActionID(setA);
	}

	return outputTrans;
}











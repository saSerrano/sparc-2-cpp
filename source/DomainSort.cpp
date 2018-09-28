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

	std::vector<int> causalLawStartLines;
	std::vector<int> causalLawLengths;
	for(unsigned int i = 0; i < ruleDefs.size(); i++)
	{
		int split_index = ruleDefs[i].find(":-");
		if(ruleDefs[i].rfind("val") < split_index   && 
		   ruleDefs[i].find("<") > split_index      &&
		   ruleDefs[i].find("occurs") > split_index &&
		   sp::countOcc(ruleDefs[i],"val") == 1     &&
		   sp::countOcc(ruleDefs[i],"<") == 1       &&
		   sp::countOcc(ruleDefs[i],"occurs") == 1 )
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
			for(unsigned int j = 0; j < temp_vec.size(); j++)
			{
				if(temp_vec[j].find("occurs") != -1)
				{
					action_index = j;
					break;
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
	CausalHeads = causalHeads;
	CausalHeadFluents = causalHeadFluents;
	CausalHeadFluentsArgs = causalHeadFluentsArgs;
	CausalBodies = causalBodies;
	CausalBodyActions = causalBodyActions;
	CausalBodyActionsArgs = causalBodyActionsArgs;
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
	for(unsigned int i = 0; i < CausalHeads.size(); i++)
	{
		std::cout << "*********************************************" << std::endl;
		std::cout << "Causal law head:" + CausalHeads[i] << std::endl;
		std::cout << "Causal law head fluent: " + CausalHeadFluents[i] << std::endl;
		std::cout << "Causal law head fluent args: ";
		for(unsigned int j = 0; j < CausalHeadFluentsArgs[i].size(); j++)
		{
			if(j == 0) std::cout << CausalHeadFluentsArgs[i][j];
			else std::cout << ", " + CausalHeadFluentsArgs[i][j];
		}
		std::cout << std::endl;
		std::cout << "Causal law body:" + CausalBodies[i] << std::endl;
		std::cout << "Causal law body action:" + CausalBodyActions[i] << std::endl;
		std::cout << "Causal law body action args: ";
		for(unsigned int j = 0; j < CausalBodyActionsArgs[i].size(); j++)
		{
			if(j == 0) std::cout << CausalBodyActionsArgs[i][j];
			else std::cout << ", " + CausalBodyActionsArgs[i][j];
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
	sortDisplay();
	hierDisplay();
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
	std::string cpFile = temp_vec[0] + "_DS.sp";

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
	sp::shellCmd("java -jar "+std::string(PATH_TO_SPARC)+" "+cpFile+" -A > DS_answer_set.txt");
	//Extract the p-states from the answer-sets file
	std::vector<sp::PState> extPS = sp::pstatesFromFile("DS_answer_set.txt");

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

std::vector<sp::Transition> sp::DomainSort::generateTransitions(std::vector<sp::PState> const &setS0, std::string const &rootActionSort) const
{
/*

This method assumes that the SPARC file loaded to generate this DomainSort:

 - For each action, it has a defined fluent that defines the set of possible outcomes
   an action can result in. This fluent is defined by the user, and must have the form,

		action:
			put(#block,#location)

		defined fluent for possible outcome:
			put_pos_outcome(#block,#location)

*/

	//Vector with hold the generated transitions
	std::vector<sp::Transition> outputTrans;

	//Initial checking on input parameters
	if(!FileIsLoaded) return outputTrans;
	if(setS0.size() == 0) return outputTrans;
	if(rootActionSort.length() <= 1) return outputTrans;
	if(!sp::startsWith(rootActionSort,"#")) return outputTrans;

	//Get the index of the sort from which we will start looking for actions
	int root_index = -1;
	for(unsigned int i = 0; i < Sorts.size(); i++)
	{
		if(rootActionSort.substr(1,-1) == Sorts[i])
		{
			root_index = i;
			break;
		}
	}
	if(root_index == -1) return outputTrans;

	//Gather those sorts of actions to be executed, as well as their arguments
	std::vector<std::string> gathActions;
	std::vector<std::vector<std::string>> gathActArgs;
	for(int i = 0; i <= root_index; i++)
	{
		for(unsigned int j = 0; j < SortElements[i].size(); j++)
		{
			if(SubsortElemIndexes[i][j] == -1 && 
			   sp::recursiveCheck((*this),SortElements[i][j],root_index))
			{
				gathActions.push_back(SortElements[i][j]);
				gathActArgs.push_back(SortArguments[i][j]);
			}
		}
	}

	//For every gathered action, verify that the SPARC file has its corresponding
	//defined fluent that defines the possible outcomes of the action
	int action_deff_count = 0;
	for(unsigned int i = 0; i < gathActions.size(); i++)
	{
		std::string action_deff_name(gathActions[i] + std::string("_pos_outcome"));
		for(unsigned int j = 0; j < SortElements.size(); j++)
		{
			bool found_fluent = false;
			for(unsigned int k = 0; k < SortElements[j].size(); k++)
			{
				if(SortElements[j][k] == action_deff_name)
				{
					std::string temp_ga_args("");
					std::string temp_df_args("");
					for(unsigned int l = 0; l < gathActArgs[i].size(); l++) temp_ga_args += gathActArgs[i][l];
					for(unsigned int l = 0; l < SortArguments[j][k].size(); l++) temp_df_args += SortArguments[j][k][l];

					if(temp_ga_args == temp_df_args)
					{
						found_fluent = true;
						action_deff_count++;
						break;
					}
				}
			}
			if(found_fluent) break;
		}
	}
	if(action_deff_count != gathActions.size()) return outputTrans;

//Generate the SPARC rules (choice rules) required to generate the transitions
//one for each gathered action


//debug
//for(int i = 0; i < gathActions.size(); i++)
//{
//std::cout <<  gathActions[i] << std::endl;
//	for(unsigned int j = 0; j < gathActArgs[i].size(); j++)
//	{
//std::cout << "\t" + gathActArgs[i][j] << std::endl;
//	}
//}

return outputTrans;
}











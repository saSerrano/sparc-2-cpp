#include <DomainSort.hpp>
#include <PState.hpp>
#include <sparc2cpp.hpp>
#include <Transition.hpp>

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

	//Get those lines from the file that are not comments
	std::string line;
	std::vector<std::string> lines;
	while(std::getline(inputFile,line)) lines.push_back(line);
	inputFile.close();
	std::vector<std::string> noCommLines;
	for(unsigned int i = 0; i < lines.size() ; i++)
	{
		if(lines[i].find('%') == std::string::npos)
		{
			noCommLines.push_back(lines[i]);
		}
	}

	//Strip spaces
	for(unsigned int i = 0; i < noCommLines.size(); i++)
	{
		noCommLines[i].erase(std::remove(noCommLines[i].begin(), noCommLines[i].end(), ' '),noCommLines[i].end());
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
	for(unsigned int i = 0; i < noCommLines.size(); i++)
	{
		if(noCommLines[i] == "sorts")
		{
			sortsWasRead = true;
			continue;
		}

		if(noCommLines[i] == "predicates") break;

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

	Consts = conNames;
	ConstVals = conVals;
	Sorts = sortNames;
	SortElements = sortElements;
	SortArguments = sortArguments;
	SubsortElemIndexes = elemIndexes;
	SubsortArgsIndexes = argsIndexes;
	FileIsLoaded = true;

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











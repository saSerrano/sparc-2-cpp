#include <sparc2cpp.hpp>

bool sp::endsWith(std::string const &fullString, std::string const &ending)
{
	if(fullString.length() >= ending.length())
		return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
	else
		return false;
}

std::vector<std::string> sp::splitStr(std::string const &fullString,std::string const &delimiter)
{
	//Verify that the delimiter is not an empty string
	int dlen = delimiter.length();
	if(dlen <= 0) return std::vector<std::string>();

	std::vector<std::string> tokens;

	//The delimiter does not exist in fullString
	if(fullString.find(delimiter) == -1)
	{
		tokens.push_back(fullString);

		return tokens;
	}

	std::string temp = fullString;
	while(true)
	{
		int index = temp.find(delimiter);
		std::string token;

		//The delimiter does not exist in 'temp'
		if(index == -1)
		{
			//The last token that doesn't end in the 'delimiter'
			token = temp;
			if(token != "") tokens.push_back(token);
			break;
		}
		else if(index == temp.length()-dlen)
		{
			//The index is the last character, therefore, this is the last iteration
			token = temp.substr(0,index);
			if(token != "") tokens.push_back(token);
			break;
		}
		else
		{
			token = temp.substr(0,index);
			if(token != "") tokens.push_back(token);
			temp = temp.substr(index+dlen,-1);
		}
	}

	return tokens;
}

bool sp::isInt(std::string const &inputString)
{
	int slen = inputString.length();
	for(int i = 0; i < slen; i++)
	{
		if(!std::isdigit(inputString[i]))
		{
			if(i == 0 && inputString.substr(0,1) == "-") continue;
			else return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------

void sp::DomainSort::verifyAndLoad()
{
	std::ifstream inputFile;
	inputFile.open(SourceFile,std::ifstream::in);

	//Verify for a reachable file
	if(!inputFile.is_open())
	{
		SourceFile = "";
		FileIsLoaded = false;
		StatusString = "Error: Could not open source file.";
		
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
						StatusString = "Error: Only sorts defined in a single line will be accepted.";
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
			StatusString = "Error: Sort definitions must have a form of a two-sided equation.";
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
	StatusString = "Succes: Source file has been successfully loaded.";

	return;
}

void sp::DomainSort::sortDisplay()
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
	std::cout << "Status string >> " + StatusString << std::endl;
}

sp::DomainSort::DomainSort()
{
	FileIsLoaded = false;
	StatusString = "-- : No file has been loaded.";
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

bool sp::DomainSort::fileIsLoaded()
{
	return FileIsLoaded;
}

std::string sp::DomainSort::getStatus()
{
	return StatusString;
}

std::string sp::DomainSort::getFile()
{
	return SourceFile;
}

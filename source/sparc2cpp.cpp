#include <sparc2cpp.hpp>
#include <PState.hpp>
#include <DomainSort.hpp>

std::string sp::shellCmd(std::string cmd)
{
	std::string data;
	FILE * stream;
	const int max_buffer = 256;
	char buffer[max_buffer];
	cmd.append(" 2>&1");

	stream = popen(cmd.c_str(), "r");
	if(stream)
	{
		while(!feof(stream))
		if(fgets(buffer,max_buffer,stream) != nullptr) data.append(buffer);
		pclose(stream);
	}
	return data;
}

bool sp::endsWith(std::string const &fullString, std::string const &ending)
{
	if(fullString.length() >= ending.length())
		return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
	else
		return false;
}

bool sp::startsWith(std::string const &fullString, std::string const &start)
{
	if(fullString.length() >= start.length())
		return (0 == fullString.compare (0, start.length(), start));
	else
		return false;
}

std::string sp::strBtwn(std::string const &fullString,int leftLimit, int rightLimit)
{
	if(leftLimit < 0 || rightLimit >= fullString.length() || leftLimit >= rightLimit) return std::string();
	else return fullString.substr(leftLimit+1,rightLimit-leftLimit-1);
}

int sp::countOcc(std::string const &fullString, std::string const &s)
{
	int count = 0;
	std::string temp(fullString);
	while(true)
	{
		int index = temp.find(s);
		if(index != -1)
		{
			count++;
			temp = temp.substr(index+1,-1);
		}
		else break;
	}

	return count;
}

std::vector<std::string> sp::correctCommaSplit(std::vector<std::string> commaSplit)
{
	std::vector<std::string> output;
	std::string tempStr("");
	int depth = 0;
	for(unsigned int i = 0; i < commaSplit.size(); i++)
	{
		int left_po = countOcc(commaSplit[i],"(");
		int right_po = countOcc(commaSplit[i],")");
		depth += (left_po - right_po);
		tempStr += (commaSplit[i] + ",");
		if(depth == 0)
		{
			int temp_len = tempStr.length();
			output.push_back(tempStr.substr(0,temp_len-1));
			tempStr = "";
		}
	}

	return output;
}

std::string sp::stripSpaces(std::string const &s)
{
	std::string output("");

	for(int i = 0; i < s.length(); i++)
	{
		if(s.substr(i,1) != " ") output = output + s.substr(i,1);
	}

	return output;
}

std::string sp::stripTabs(std::string const &s)
{
	std::string output("");

	for(int i = 0; i < s.length(); i++)
	{
		if(s.substr(i,1) != "\t") output = output + s.substr(i,1);
	}

	return output;
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

std::vector<sp::PState> sp::pstatesFromFile(std::string const &answerSetsFile)
{
	std::ifstream inputFile;
	inputFile.open(answerSetsFile,std::ifstream::in);

	//Verify for a reachable file
	if(!inputFile.is_open())
	{
		std::cout << "Could not open answer sets file: " + answerSetsFile << std::endl;
		return std::vector<sp::PState>();
	}

	//Get those lines from the file that are not empty
	std::string line;
	std::vector<std::string> lines;
	std::vector<std::string> nonEmptyLines;
	while(std::getline(inputFile,line)) lines.push_back(line);
	inputFile.close();
	for(unsigned int i = 0; i < lines.size() ; i++)
	{
		if(lines[i].find("{") != -1)
		{
			nonEmptyLines.push_back(lines[i]);
		}
	}

	std::vector<sp::PState> pstates;
	for(unsigned int i = 0; i < nonEmptyLines.size(); i++)
	{
		sp::PState ps(nonEmptyLines[i]);
		pstates.push_back(ps);
	}

	return pstates;
}

bool sp::recursiveCheck(sp::DomainSort const &ds, std::string const &input, int sortToCheck)
{
	//Look within the instances of sort 'sortToCheck'
	for(unsigned int i = 0; i < ds.SortElements[sortToCheck].size(); i++)
	{
		if(input == ds.SortElements[sortToCheck][i]) return true;
	}

	//Look within the instances of sub-sorts of 'sortToCheck'
	for(unsigned int i = 0; i < ds.SubsortElemIndexes[sortToCheck].size(); i++)
	{
		if(ds.SubsortElemIndexes[sortToCheck][i] > -1)
		{
			if(recursiveCheck(ds,input,ds.SubsortElemIndexes[sortToCheck][i])) return true;
		}
	}

	//'input' was not found neither in 'sortToCheck' nor any of its sub-sorts
	return false;
}

bool sp::checkDef(sp::DomainSort const &ds, std::string const &def, std::string const &input)
{
	//Verify that 'ds' has loaded a domain's sorts from a file
	if(!ds.fileIsLoaded()) return false;

	//Check if 'input' satisfies definition 'def' with respect 'ds'
	bool definitionSatisfied = false;
	if(def == "X" && input.length() > 0)
	{
		//Any non-empty string satisfies 'X'
		definitionSatisfied = true;
	}
	else if(sp::startsWith(def,"#") && def.length() > 1)
	{
		//'input' must belong to the sort that follows '#' in 'def' or
		//belong to one of its sub-sorts
		std::string sort = def.substr(1,-1);
		for(unsigned int i = 0; i < ds.Sorts.size(); i++)
		{
			if(ds.Sorts[i] == sort)
			{
				definitionSatisfied = sp::recursiveCheck(ds,input,i);
				break;
			}
		}
	}
	else if(def.length() > 0 && def == input)
	{
		//If 'def' has no special characters and is equal to 'input'
		//the definition is sastified.
		definitionSatisfied = true;
	}

	return definitionSatisfied;
}

sp::PState sp::subPstate(sp::DomainSort const &ds, sp::PState const &ps, std::vector<std::string> const &defs)
{
	/*

	INVALID DEFINITIONS NOTE: There are 4 type of syntaxis errors that make an invalid definition.

		i) Empty parenthesis: val(on(),true,3)

		ii) Pair of parenthesis preceeded by empty string: val(on(b0,t),(t,b2),true,1)

		iii) Parenthesis that are not enclosed: val(can_test)on(b1,t)),true,0)

		iv) sub-sub-args with arguments: There can be at most 3 nested parenthesis
			ok:     val(can_test(loc(rb1,c0),true),true,1)
			not ok: val(can_test(loc(rb1,c0(t,false)),true),true,1)

	*/

	//Output set of literals
	sp::PState sub_pstate;

	//Verify that 'ds' has loaded a domain and 'ps'
	if(!ds.fileIsLoaded()) return sub_pstate;
	if(ps.len() == -1) return sub_pstate;
	if(ps.PStateString == "{}")
	{
		sub_pstate.set("{}");
		return sub_pstate;
	}

	// i-th def-name
	std::vector<int> addedLits;
	std::vector<int>::iterator it;
	for(unsigned int i = 0; i < defs.size(); i++)
	{
		int l_count = sp::countOcc(defs[i],"(");
		int r_count = sp::countOcc(defs[i],")");
		int l_par = defs[i].find("(");
		int r_par = defs[i].rfind(")");

		//Check for invalid definition
		if((l_par == -1 && r_par != -1)   || 
		   (l_par != -1 && r_par == -1)   || 
		   (r_par != -1 && l_par > r_par) ||
		   (l_count != r_count))
		{
			continue;
		}
		else if(l_par == -1 && r_par == -1)
		{
			//Check for an empty def-name
			if(sp::stripSpaces(defs[i]) == "")
			{
				continue;
			}

			//A definition that consists of the literal's name only
			for(unsigned int j = 0; j < ps.LitNames.size(); j++)
			{
				it = std::find(addedLits.begin(),addedLits.end(),j);
				if(checkDef(ds,defs[i],ps.LitNames[j]) && it == addedLits.end())
				{
					//Add the literal from 'ps' to 'sub_pstate'
					sub_pstate.LitNames.push_back(ps.LitNames[j]);
					sub_pstate.LitArgs.push_back(ps.LitArgs[j]);
					sub_pstate.LitArgArgs.push_back(ps.LitArgArgs[j]);
					sub_pstate.LitArgArgArgs.push_back(ps.LitArgArgArgs[j]);

					//Save 'ps' literal's index, so it is not added more than once
					addedLits.push_back(j);
				}
			}
		}
		else
		{
			//Extract the components of the definition 'defs[i]'
			//A definition that consists of the literal's name, its arguments and possibly its sub-args and
			//sub-sub-args
			std::vector<std::string> comps = sp::splitStr(sp::strBtwn(defs[i],l_par,r_par),",");
			comps = sp::correctCommaSplit(comps);
			std::string def_name = defs[i].substr(0,l_par); 			// i-th def-name
			std::vector<std::string> def_args;					// i-th def-args
			std::vector<std::vector<std::string>> def_arg_args;			// i-th def-sub-args
			std::vector<std::vector<std::vector<std::string>>> def_arg_arg_args;	// i-th def-sub-sub-args

			//Check for an invalid def-name
			if((sp::stripSpaces(def_name) == "") || (def_name.find(")") != -1))
			{
				continue;
			}

			//Get the argument components of the i-th definition
			bool i_def_is_invalid = false;
			for(unsigned int j = 0; j < comps.size(); j++) // j-th def-arg
			{
				int lp_count = sp::countOcc(comps[j],"(");
				int rp_count = sp::countOcc(comps[j],")");
				int lp_pos = comps[j].find("(");
				int rp_pos = comps[j].rfind(")");

				//Each argument must be valid for the i-th definition to be valid
				if((lp_count != rp_count) || (lp_pos > rp_pos) || (sp::stripSpaces(comps[j]) == ""))
				{
					i_def_is_invalid = true;
					break;
				}

				// j-th def-arg has sub-args
				if(lp_pos > -1)
				{
					//Check for an empty def-arg
					if((sp::stripSpaces(comps[j].substr(0,lp_pos)) == "") || (comps[j].substr(0,lp_pos).find(")") != -1))
					{
						i_def_is_invalid = true;
						break;
					}

					std::vector<std::string> temp_arg_args;
					temp_arg_args = sp::splitStr(sp::strBtwn(comps[j],lp_pos,rp_pos),",");
					temp_arg_args = sp::correctCommaSplit(temp_arg_args);
					std::vector<std::string> tt_arg_args;
					std::vector<std::vector<std::string>> tt_arg_arg_args;
					for(unsigned int k = 0; k < temp_arg_args.size(); k++) // k-th def-sub-arg
					{
						int llp_count = sp::countOcc(temp_arg_args[k],"(");
						int rrp_count = sp::countOcc(temp_arg_args[k],")");
						int llp = temp_arg_args[k].find("(");
						int rrp = temp_arg_args[k].rfind(")");

						//Each sub-arg must be valid for the i-th definition to be valid
						if((llp_count != rrp_count) || (llp > rrp) || (sp::stripSpaces(temp_arg_args[k]) == ""))
						{
							i_def_is_invalid = true;
							break;
						}

						// k-th sub-arg has args
						if(llp > -1)
						{
							//Check for an invalid def-sub-arg
							if((sp::stripSpaces(temp_arg_args[k].substr(0,llp)) == "") || (temp_arg_args[k].substr(0,llp).find(")") != -1))
							{
								i_def_is_invalid = true;
								break;
							}

							std::vector<std::string> ttt_aaa = sp::splitStr(sp::strBtwn(temp_arg_args[k],llp,rrp),",");
							ttt_aaa = sp::correctCommaSplit(ttt_aaa);
							for(unsigned int l = 0; l < ttt_aaa.size(); l++) // l-th def-sub-sub-arg
							{
								int lllp = ttt_aaa[l].find("(");
								int rrrp = ttt_aaa[l].rfind(")");

								//Each sub-sub-arg must be valid for the i-th definition to be valid
								//Each sub-sub-arg cannot have parenthesis
								if((lllp != -1) || (rrrp != -1) || (sp::stripSpaces(ttt_aaa[l]) == ""))
								{
									i_def_is_invalid = true;
									break;
								}
							}

							if(i_def_is_invalid) break;

							tt_arg_args.push_back(temp_arg_args[k].substr(0,llp));
							tt_arg_arg_args.push_back(ttt_aaa);
						}
						else // k-th sub-arg has no args
						{
							tt_arg_args.push_back(temp_arg_args[k]);
							tt_arg_arg_args.push_back(std::vector<std::string>());
						}
					}

					if(i_def_is_invalid) break;

					def_args.push_back(comps[j].substr(0,lp_pos)); // j-th def-arg
					def_arg_args.push_back(tt_arg_args);
					def_arg_arg_args.push_back(tt_arg_arg_args);
				}
				else //This argument does not have arguments
				{
					def_args.push_back(comps[j]);
					def_arg_args.push_back(std::vector<std::string>());
					def_arg_arg_args.push_back(std::vector<std::vector<std::string>>());
				}
			}

			//Avoid evaluating with respect to invalid
			if(i_def_is_invalid) continue;

			//From the extracted definition, evaluate each literal from 'ps'
			for(unsigned int j = 0; j < ps.LitNames.size(); j++)
			{
				//If the j-th literal has already been added to 'sub_pstate'
				//dont bother to evaluate it again
				it = std::find(addedLits.begin(),addedLits.end(),j);
				if(it != addedLits.end()) continue;

				//Check the literal's name
				if(sp::checkDef(ds,def_name,ps.LitNames[j]))
				{
					//Verify that the i-th definition and j-th literal have the same amount of args
					if(def_args.size() != ps.LitArgs[j].size()) continue;

					//Check each literal's arguments, sub-arguments and sub-sub-arguments
					bool allArgsCheck = true;
					for(unsigned int k = 0; k < ps.LitArgs[j].size(); k++)
					{
						//Check the k-th argument
						if(sp::checkDef(ds,def_args[k],ps.LitArgs[j][k]))
						{
							if(def_arg_args[k].size() > 0)
							{
								if(def_arg_args[k].size() != ps.LitArgArgs[j][k].size())
								{
									allArgsCheck = false;
									break;
								}

								//Check the k-th argument's sub-args
								for(unsigned int l = 0; l < ps.LitArgArgs[j][k].size(); l++)
								{
									//Check the l-th sub-argument
									if(sp::checkDef(ds,def_arg_args[k][l],ps.LitArgArgs[j][k][l]))
									{
										if(def_arg_arg_args[k][l].size() > 0)
										{
											if(def_arg_arg_args[k][l].size() != ps.LitArgArgArgs[j][k][l].size())
											{
												allArgsCheck = false;
												break;
											}

											//Check the l-th sub-argument's sub-sub-args
											for(unsigned int m = 0; m < ps.LitArgArgArgs[j][k][l].size(); m++)
											{
												//Check the m-th sub-sub-argument
												if(!sp::checkDef(ds,def_arg_arg_args[k][l][m],ps.LitArgArgArgs[j][k][l][m]))
												{
													allArgsCheck = false;
													break;
												}
											}

											if(!allArgsCheck) break;

										}

									}
									else
									{
										allArgsCheck = false;
										break;
									}
								}

								if(!allArgsCheck) break;
							}
						}
						else
						{
							allArgsCheck = false;
							break;
						}
					}

					//All the argumnets and sub-arguments satisfied the definition
					if(allArgsCheck)
					{
						//Add the literal from 'ps' to 'sub_pstate'
						sub_pstate.LitNames.push_back(ps.LitNames[j]);
						sub_pstate.LitArgs.push_back(ps.LitArgs[j]);
						sub_pstate.LitArgArgs.push_back(ps.LitArgArgs[j]);
						sub_pstate.LitArgArgArgs.push_back(ps.LitArgArgArgs[j]);

						//Save 'ps' literal's index, so it is not added more than once
						addedLits.push_back(j);
					}
				}
				else continue;
			}
		}
	}

	//Build the string 'PStateString' for the sub-Pstate
	std::string ps_string("{");
	for(unsigned int i = 0; i < sub_pstate.LitNames.size(); i++)
	{
		if(i > 0) ps_string += ",";

		ps_string += sub_pstate.LitNames[i];
		if(sub_pstate.LitArgs[i].size() > 0)
		{
			ps_string += "(";
			for(unsigned int j = 0; j < sub_pstate.LitArgs[i].size(); j++)
			{
				if(j > 0) ps_string += ",";

				ps_string += sub_pstate.LitArgs[i][j];
				if(sub_pstate.LitArgArgs[i][j].size() > 0)
				{
					ps_string += "(";
					for(unsigned int k = 0; k < sub_pstate.LitArgArgs[i][j].size(); k++)
					{
						if(k > 0) ps_string += ",";

						ps_string += sub_pstate.LitArgArgs[i][j][k];
						if(sub_pstate.LitArgArgArgs[i][j][k].size() > 0)
						{
							ps_string += "(";
							for(unsigned int l = 0; l < sub_pstate.LitArgArgArgs[i][j][k].size(); l++)
							{
								if(l > 0) ps_string += ",";

								ps_string += sub_pstate.LitArgArgArgs[i][j][k][l];
							}
							ps_string += ")";
						}
					}
					ps_string += ")";
				}
			}
			ps_string += ")";
		}
	}
	ps_string += "}";
	sub_pstate.PStateString = ps_string;

	//Save in 'PStateLength' the amount of literals sub_pstate has
	sub_pstate.PStateLength = sub_pstate.LitNames.size();

	return sub_pstate;
}


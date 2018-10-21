#include <Pomdp.hpp>
#include <sparc2cpp.hpp>
#include <DomainSort.hpp>
#include <PState.hpp>
#include <Action.hpp>
#include <Transition.hpp>

std::tuple<std::vector<int>,std::vector<std::string>> sp::Pomdp::getStats(std::string file,std::vector<std::string> const &actions,std::vector<std::string> const &states,std::vector<std::string> const &obs)
{
	//Output tuple that will hold the stats and reward function
	std::tuple<std::vector<int>,std::vector<std::string>> stats;

	//Initial check
	if(actions.size() == 0) return stats;
	if(states.size() == 0) return stats;
	if(obs.size() == 0) return stats;
	std::ifstream infile;
	infile.open(file,std::ifstream::in);
	if(!infile.is_open()) return stats;

	//Gather the useful lines
	std::string line;
	std::vector<std::string> lines;
	while(std::getline(infile,line))
	{
		//Keep the non-empty lines
		if(line.find("T") != -1) lines.push_back(line);
		else if(line.find("O") != -1) lines.push_back(line);
		else if(line.find("R") != -1) lines.push_back(line);
		else if(line.find("0") != -1) lines.push_back(line);
		else if(line.find("i") != -1) lines.push_back(line);
		else if(line.find("1") != -1) lines.push_back(line);
	}

	//Extract the T, O and R functions from the raw text
	std::vector<int> gathAction;
	std::vector<std::string> gathMat;
	std::string temp_s("");
	unsigned int row_c = 0;
	for(unsigned int i = 0; i < lines.size(); i++)
	{
		//Extract a matrix of prob. for the T and O functions
		if(lines[i].find("T") != -1 || lines[i].find("O") != -1)
		{
			temp_s = "";
			row_c =0;
			std::vector<std::string> sep_vec = sp::splitStr(lines[i],":");
			if(sep_vec.size() != 2 && sep_vec.size() != 4) continue;

			//Determine if its a "T" or "O" matrix
			bool is_t = (lines[i].find("T") != -1);

			//T-matrix in the form of: matrix, "uniform" or "identity", or
			//O-matrix in the form of: matrix or "uniform"
			if(sep_vec.size() == 2)
			{
				//Verify if the action is actually in the list of valid actions,
				//otherwise don't even bother building the matrix
				int action_index = -1;
				sep_vec[1] = sp::stripSpaces(sep_vec[1]);
				for(unsigned int j = 0; j < actions.size(); j++)
				{
					if(sep_vec[1] == actions[j])
					{
						action_index = j;
						break;
					}
				}
				if(action_index == -1) continue;

				//Make sure there is at least 1 line after the i-th one
				if(i < lines.size()-1)
				{
					//It is an identity or an uniform matrix
					if(lines[i+1].find("identity") != -1 || lines[i+1].find("uniform") != -1)
					{
						//This matrix has been accepted
						if(is_t) temp_s = "T: " + actions[action_index];
						else temp_s = "O: " + actions[action_index];

						if(is_t)
						{
							if(lines[i+1].find("identity") != -1) temp_s += "\nidentity";
							else temp_s += "\nuniform";

							//Save the matrix
							gathMat.push_back(temp_s);
							gathAction.push_back(action_index);
						}
						else
						{
							if(lines[i+1].find("uniform") != -1)
							{
								temp_s += "\nuniform";

								//Save the matrix
								gathMat.push_back(temp_s);
								gathAction.push_back(action_index);
							}
						}

						//Increase the index due to the second line of this matrix was read
						i++;
					}
					//It might be a custom matrix
					else
					{
						//Gather the rows for this matrix
						for(unsigned int j = i+1; j < lines.size(); j++)
						{
							std::vector<std::string> temp_vec;
							temp_vec = sp::splitStr(lines[j]," ");
							if(temp_vec.size() == states.size() && is_t)
							{
								//The j-th line is a valid row in the T-matrix that is
								//being built
								temp_s += (lines[j] + "\n");
								row_c++;
							}
							else if(temp_vec.size() == obs.size() && !is_t)
							{
								//The j-th line is a valid row in the O-matrix that is
								//being built
								temp_s += (lines[j] + "\n");
								row_c++;
							}
							else
							{
								//The j-th line is not a row for the matrix, so stop
								//gathering rows for this matrix

								//Increase the index due to the row-lines that were read
								i += row_c;
								break;
							}

							//This matrix has been accepted
							if(row_c == states.size())
							{
								if(is_t) temp_s = ("T: "+actions[action_index]+"\n"+temp_s);
								else temp_s = ("O: "+actions[action_index]+"\n"+temp_s);

								//Save the matrix
								gathMat.push_back(temp_s);
								gathAction.push_back(action_index);

								//Increase the index due to the row-lines that were read
								i += row_c;
								break;
							}
						}
					}

				}

			}
			//It is a 1 line T-matrix
			else
			{
				//Strip the spaces from the first 3 elements of the line
				sep_vec[0] = sp::stripSpaces(sep_vec[0]);
				sep_vec[1] = sp::stripSpaces(sep_vec[1]);
				sep_vec[2] = sp::stripSpaces(sep_vec[2]);
				//Separate by the " " the last element
				std::vector<std::string> last_arg = sp::splitStr(sep_vec[3]," ");

				//Make sure the last element has 2 tokens separated by " ", that
				//the first element is "T" or "O", and the third one is "*"
				if(last_arg.size() == 2 					&&
				  ((sep_vec[0] == "T" && is_t) || (sep_vec[0] == "O" && !is_t)) &&
				   sep_vec[2] == "*")
				{
					//Make sure the prob. assigned for this line is 1
					if(last_arg[1] == "1")
					{
						//Get the indexes of the action
						int action_index = -1;
						int other_index = -1;
						for(unsigned int j = 0; j < actions.size(); j++)
						{
							if(sep_vec[1] == actions[j])
							{
								action_index = j;
								break;
							}
						}

						//Get the index of the ending-state
						if(is_t)
						{
							for(unsigned int j = 0; j < states.size(); j++)
							{
								if(last_arg[0] == states[j])
								{
									other_index = j;
									break;
								}
							}
						}
						//Get the index of the observation
						else
						{
							for(unsigned int j = 0; j < obs.size(); j++)
							{
								if(last_arg[0] == obs[j])
								{
									other_index = j;
									break;
								}
							}
						}

						//This line has been accepted
						if(action_index > -1 && other_index > -1)
						{
							gathMat.push_back(lines[i]);
							gathAction.push_back(action_index);
						}
					}
				}
			}
//-------------------------------------------------------------------------------
		}
		//Extract a part of the R function
		// -- Only the 1-line format is supported --
		else if(lines[i].find("R") != -1)
		{
			//Extract the elements separated by ":"
			std::vector<std::string> temp_vec;
			temp_vec = sp::splitStr(lines[i],":");

			//Verify this line satisfies the the 1-line format
			if(temp_vec.size() != 5) continue;
			for(unsigned int j = 0; j < 4; j++) temp_vec[j] = sp::stripSpaces(temp_vec[j]);
			if(temp_vec[0] != "R") continue;

			//Check for a valid action
			if(temp_vec[1] != "*")
			{
				bool a_found = false;
				for(unsigned int j = 0; j < actions.size(); j++)
				{
					if(temp_vec[1] == actions[j])
					{
						a_found = true;
						break;
					}
				}
				if(!a_found) continue;
			}

			//Check for valid starting and ending states
			int amount_s_found = 0;
			for(int j = 2; j <= 3; j++)
			{
				if(temp_vec[j] == "*")
				{
					amount_s_found++;
					continue;
				}

				for(unsigned int k = 0; k < states.size(); k++)
				{
					if(temp_vec[j] == states[k])
					{
						amount_s_found++;
						break;
					}
				}
			}
			if(amount_s_found < 2) continue;

			//Start building the reward specification
			temp_s = "R: " + temp_vec[1] + " : " + temp_vec[2] + " : " + temp_vec[3] + " : ";

			//Extract the elements separated by a " " from the last element of
			//temp_vec
			temp_vec = sp::splitStr(temp_vec[4]," ");
			if(temp_vec.size() != 2) continue;

			//Check for a valid observation
			if(temp_vec[0] != "*")
			{
				bool o_found = false;
				for(unsigned int j = 0; j < obs.size(); j++)
				{
					if(temp_vec[0] == obs[j])
					{
						o_found = true;
						break;
					}
				}
				if(!o_found) continue;
			}

			//Check for a valid reward value, i.e., a numeric value
			if(!sp::isFloat(temp_vec[1])) continue;

			//Finish building the reward specification and save it
			temp_s += temp_vec[0] + " " + temp_vec[1];
			gathMat.push_back(temp_s);
			gathAction.push_back(-1);
		}
	}

	//Insert the matrix-strings and their respective action-indexes in "stats"
	std::get<0>(stats) = gathAction;
	std::get<1>(stats) = gathMat;

	return stats;
}

sp::Pomdp::Pomdp():
discount(0.95),
values("reward"),
default_prob_T(0.8)
{
}

sp::Pomdp::Pomdp(double discount)
{
	default_prob_T = 0.8;
	values = "reward";
	if(discount >= 0 && discount <= 1) this->discount = discount;
	else this->discount = 0.95;
}

sp::Pomdp::Pomdp(std::string values)
{
	default_prob_T = 0.8;
	discount = 0.95;
	if(values == "reward" || values == "cost") this->values = values;
	else this->values = "reward";
}

sp::Pomdp::Pomdp(double discount,std::string values)
{
	default_prob_T = 0.8;
	if(discount >= 0 && discount <= 1) this->discount = discount;
	else this->discount = 0.95;

	if(values == "reward" || values == "cost") this->values = values;
	else this->values = "reward";
}

bool sp::Pomdp::setDiscount(double discount)
{
	if(discount >= 0 && discount <= 1)
	{
		this->discount = discount;
		return true;
	}
	else return false;

}

bool sp::Pomdp::setValues(std::string values)
{
	if(values == "reward" || values == "cost")
	{
		this->values = values;
		return true;
	}
	else return false;
}

bool sp::Pomdp::generatePomdp(std::vector<sp::PState> &S, 
                              std::vector<sp::PState> &Z, 
                              std::vector<sp::Action> const &nkA, 
                              std::vector<sp::Action> const &kA, 
                              std::vector<sp::Action> const &termA, 
                              std::vector<sp::Transition> &T, 
                              sp::DomainSort const &ds, 
                              std::string const &pomdpFile,
                              std::string const &statFile)
{
	//Input parameters checking
	//if(S.size() == 0) return false;
	//if(nkA.size() == 0) return false;
	//if(kA.size() == 0) return false;
	//if(termA.size() == 0) return false;
	//if(Z.size() == 0) return false;
	//if(T.size() == 0) return false;
	//if(!ds.fileIsLoaded()) return false;

	//Add the "absorbing" state to S
	S.push_back(sp::PState("{absb}"));

	//Delete from Z those states that have "undet" for value
	std::vector<int> obs2delete;
	for(unsigned int i = 0; i < Z.size(); i++)
	{
		if(Z[i].str().find("undet") != -1) obs2delete.push_back(i);
	}
	for(int i = obs2delete.size()-1; i >= 0 ; i--)
	{
		Z.erase(Z.begin() + obs2delete[i]);
	}

	//Add the "None" observation to Z
	Z.push_back(sp::PState("{none}"));

	//Substitute with the "None" observation those states in that have for value "undet"
	//within the vector of transitions T
	for(unsigned int i = 0; i < T.size(); i++)
	{
		if(T[i].ps1.str().find("undet") != -1)
		{
			T[i].ps1.set("{none}");
		}
	}

	//Index the transitions in T with respect to [S,Z] and [nkA,kA,termA]
	std::vector<sp::PState> SZ;
	std::vector<sp::Action> AAA;
	SZ.insert(SZ.end(),S.begin(),S.end());
	SZ.insert(SZ.end(),Z.begin(),Z.end());
	AAA.insert(AAA.end(),nkA.begin(),nkA.end());
	AAA.insert(AAA.end(),kA.begin(),kA.end());
	AAA.insert(AAA.end(),termA.begin(),termA.end());
	sp::Transition::index(T,SZ,AAA);

//debug
std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;
std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;
std::cout << "################  STATES  ##################" << std::endl;
for(unsigned int i = 0; i < SZ.size(); i++)
{
std::cout << "State-" << i << ": " + SZ[i].str() << std::endl;
}
std::cout << "################  ACTIONS  ##################" << std::endl;
for(unsigned int i = 0; i < AAA.size(); i++)
{
std::cout << "Action-" << i << ": " + AAA[i].str() << std::endl;
}
std::cout << "################  TRANSITIONS  ##################" << std::endl;
for(unsigned int i = 0; i < T.size(); i++)
{
T[i].debug();
std::cout << "----------------------------------------" << std::endl;
}


	//Create the pomdp file
	std::ofstream outf;
	outf.open(pomdpFile,std::ofstream::out);
	if(!outf.is_open()) return false;

	outf << "discount: " << discount << std::endl;
	outf << "values: " << values << std::endl;

	//---------------------------------------------------------------------------------------------
	//STATES
	std::vector<std::string> states_vec;
	outf << "states: ";
	for(unsigned int i = 0; i < S.size(); i++)
	{
		int state_len = S[i].str().length();
		std::string all_lits = S[i].str().substr(1,state_len-2);
		std::vector<std::string> lits_vec = splitStr(all_lits,",");
		lits_vec = correctCommaSplit(lits_vec);
		std::string temp_state_str("");
		for(unsigned int j = 0; j <  lits_vec.size(); j++)
		{
			if(j > 0) temp_state_str += "-";
			temp_state_str += lits_vec[j];
		}

		states_vec.push_back(temp_state_str);
		outf << temp_state_str + " ";
	}
	outf << std::endl;
	//---------------------------------------------------------------------------------------------
	//ACTIONS: knA, kA, and termA
	std::vector<std::string> action_vec;
	outf << "actions: ";
	for(unsigned int i = 0; i < AAA.size(); i++)
	{
		std::string temp_a_str("");
		for(unsigned int j = 0; j < AAA[i].ActionNames.size(); j++)
		{
			if(j > 0) temp_a_str += "-";
			temp_a_str += AAA[i].ActionNames[j];
			if(AAA[i].ActionArgs[j].size() > 0)
			{
				temp_a_str += "(";
				for(unsigned int k = 0; k < AAA[i].ActionArgs[j].size(); k++)
				{
					if(k > 0) temp_a_str += ",";
					temp_a_str += AAA[i].ActionArgs[j][k];
					if(AAA[i].ActionArgArgs[j][k].size() > 0)
					{
						temp_a_str += "(";
						for(unsigned int l = 0; l < AAA[i].ActionArgArgs[j][k].size(); l++)
						{
							if(l > 0) temp_a_str += ",";
							temp_a_str += AAA[i].ActionArgArgs[j][k][l];
						}
						temp_a_str += ")";
					}
				}
				temp_a_str += ")";
			}
		}
		action_vec.push_back(temp_a_str);
		outf << temp_a_str;
		outf << " ";
	}
	outf << std::endl;
	//---------------------------------------------------------------------------------------------
	//OBSERVATIONS
	std::vector<std::string> obs_vec;
	outf << "observations: ";
	for(unsigned int i = 0; i < Z.size(); i++)
	{
		int obs_len = Z[i].str().length();
		std::string all_lits = Z[i].str().substr(1,obs_len-2);
		std::vector<std::string> lits_vec = splitStr(all_lits,",");
		lits_vec = correctCommaSplit(lits_vec);
		std::string temp_obs_str("");
		for(unsigned int j = 0; j <  lits_vec.size(); j++)
		{
			if(j > 0) temp_obs_str += "-";
			temp_obs_str += lits_vec[j];
		}

		obs_vec.push_back(temp_obs_str);
		outf << temp_obs_str + " ";
	}
	outf << std::endl << std::endl;

//debug
outf.close();
return true;

	//---------------------------------------------------------------------------------------------
//TRANSITION FUNCTION

//By default the probability assigned to a transition is:
// 0.8 if the ending state corresponds to the state reached by the deterministic causal law, i.e. the correct transition.
// The other 0.2 is distributed over the rest of the ending states reached by that action.

// The transitions in T are indexed over the vectors resulting by:
// - States: [S,Z]
// - Actions: [nkA,kA,termA]


unsigned int lastNKA = nkA.size()-1;
unsigned int lastKA = kA.size() + lastNKA;
for(unsigned int i = 0; i < action_vec.size(); i++)
{
std::cout << "ACTION-" << i << std::endl;
	if(i <= lastNKA) //Non-knowledge producing actions have effect in the T function
	{
		outf << "\n\nT:" + action_vec[i] << std::endl;

		//Iterate over the rows of the matrix
		for(unsigned int j = 0; j < S.size(); j++)
		{
std::cout << "STATES0-" << j << std::endl;
			//Gather all the transitions that start in the j-th state and have the i-th action
			std::vector<int> relT;
			for(unsigned int k = 0; k < T.size(); k++)
			{
std::cout << T[k].iS0() << "," << T[k].iA() << "," << T[k].iS1();

				if(T[k].iA() == static_cast<int>(i) && T[k].iS0() == static_cast<int>(j))
{
					relT.push_back(k);
std::cout << " +++++++" << std::endl;
}
else std::cout << " x" << std::endl;
			}

			//There are no transitions with these S0 & A
			if(relT.size() == 0)
			{
std::cout << "No relevant transitions" << std::endl;
				//The i-th action has no effect over the j-th state
				for(unsigned int k = 0; k < S.size(); k++)
				{
					if(k > 0) outf << " ";

					if(k == j) outf << "1";
					else outf << "0";
				}
				outf << std::endl;

				//The j-th row of the i-th matrix is complete, onto the next one
				continue;
			}

			//This is a deterministic transition
			if(relT.size() == 1)
			{
std::cout << "A single relevant transition" << std::endl;
				for(unsigned int k = 0; k < S.size(); k++)
				{
					if(k > 0) outf << " ";

					if(T[relT[0]].iS1() == static_cast<int>(k)) outf << "1";
					else outf << "0";
				}
				outf << std::endl;

				//The j-th row of the i-th matrix is complete, onto the next one
				continue;
			}

std::cout << relT.size() << " relevant transitions" << std::endl;
			//Create a copy of the original SPARC file to find the deterministic transition
			sp::shellCmd("cp "+ds.SourceFile+" temp_sp_file.sp");
			std::ofstream temf;
			temf.open("temp_sp_file.sp",std::ofstream::app);
			if(!temf.is_open())
			{
				outf.close();
				return false;
			}
			//Add j-th S0 to the copy file
			int state_len = S[j].str().length();
			std::string all_lits = S[j].str().substr(1,state_len-2);
			std::vector<std::string> lits_vec = splitStr(all_lits,",");
			lits_vec = correctCommaSplit(lits_vec);
			for(unsigned int k = 0; k < lits_vec.size(); k++) temf << lits_vec[k]+".\n";
			//Add the i-th action to the copy file
			std::vector<std::string> temp_a_vec = sp::splitStr(action_vec[i],"-");
			for(unsigned int k = 0; k < temp_a_vec.size(); k++)
			{
				temf << "occurs("+temp_a_vec[k]+",0).\n";
			}
			//Add the display command to filter the output
			temf << "display\n";
			temf << "val(F,true,1).\n";
			temf.close();
			//Compute the answer-sets
			sp::shellCmd("java -jar " + std::string(PATH_TO_SPARC) + " temp_sp_file.sp -A > temp_AS.txt");
			//Extract the reached state from the answer-set file
			std::vector<sp::PState> temp_S1 = sp::pstatesFromFile("temp_AS.txt");
			//Delete both temporal files
			sp::shellCmd("rm temp_sp_file.sp temp_AS.txt");

			//From the relevant transitions find the one that matches the deterministic transition
			int detT = -1;
std::cout << ">> Iterating over relevant transitions: ";
			for(unsigned int k = 0; k < relT.size(); k++)
			{
std::cout << relT[k] << ", ";

				//Change the step value of every literal in temp_S1[0] to 0
				for(unsigned int l = 0; l < temp_S1[0].LitNames.size(); l++)
				{
					for(unsigned int m = 0; m < temp_S1[0].LitArgs[l].size(); m++)
					{
						//That last argument that is "1" is changed to "0"
						if((temp_S1[0].LitArgs[l][m] == "1") && (m == temp_S1[0].LitArgs[l].size()-1))
							temp_S1[0].LitArgs[l][m] = "0";
					}
				}


				//Check for the deterministic reached state S1
				if(temp_S1[0].contains(S[T[relT[k]].iS1()]))
				{
					detT = relT[k];
					break;
				}
			}
std::cout << std::endl;
std::cout << ">> detT: " << detT << std::endl;

			//Assign the probabilities for the j-th row
			double others_t_prob = (1 - default_prob_T) / static_cast<double>(relT.size()-1);
			for(unsigned int k = 0; k < S.size(); k++)
			{
				if(k > 0) outf << " ";

				//The det. transition
				if(k == T[detT].iS1())
				{
					outf << default_prob_T;
					continue;
				}

				bool k_is_rel = false;
				for(unsigned int l = 0; l < relT.size(); l++)
				{
					//A non-det. but relevant transition
					if(k == T[relT[l]].iS1())
					{
						outf << others_t_prob;
						k_is_rel = true;
						break;
					}
				}

				//A not relevant transition
				if(!k_is_rel) outf << "0";
			}
			outf << std::endl;
		}
	}
	else if(i <= lastKA) //Knowledge producing actions do not have effect in the T function
	{
		outf << "T:" + action_vec[i] << std::endl;
		outf << "identity" << std::endl << std::endl;
	}
	else //POMDP execution termination actions
	{
		//Tarminate actions always trasit the system to the absorbing state absb
		outf << "T: " + action_vec[i] + " : * : absb 1" << std::endl;
	}
}
	//---------------------------------------------------------------------------------------------
//OBSERVATION FUNCTION

for(unsigned int i = 0; i < action_vec.size(); i++)
{
	if(i >= lastNKA) //Knowledge producing actions have effect in the O function
	{
		//Save those states for which the i-th action cannot generate any observation
		std::vector<int> stateNoObs;

		//Itareate over the states (rows)
		for(unsigned int j = 0; j < S.size(); j++)
		{
			//Gather those transitions of the form S[j]-A[i]-Z[*]
			std::vector<int> relT;
			for(unsigned int k = 0; k < Z.size(); k++)
			{
				if(T[k].iS0() == static_cast<int>(j) &&
				   T[k].iA() == static_cast<int>(i))
					relT.push_back(k);
			}

			//No observation can be generated from state-j and action-i
			if(relT.size() == 0)
			{
				//The values of this row will be later overwritten
				stateNoObs.push_back(j);
				for(unsigned k = 0; k < Z.size(); k++)
				{
					if(k > 0) outf << " ";

					if(k == 0) outf << "1";
					else outf << "0";
				}
				outf << std::endl;
			}
			else
			{
				//Compute the probability to be assigned to each valid transition
				double saz_prob = 1 / static_cast<double>(relT.size());
				for(unsigned int k = 0; k < Z.size(); k++)
				{
					if(k > 0) outf << " ";

					//Check if S[j]-A[i]-Z[k] is in relT
					bool saz_is_valid = false;
					for(unsigned int l = 0; l < relT.size(); l++)
					{
						//k + S.size(), offset is required due to T is indexed over [S + Z]
						if(T[relT[l]].iS1() == (k + S.size()))
						{
							saz_is_valid = true;
							break;
						}
					}

					//Assign the probability
					if(saz_is_valid) outf << saz_prob;
					else outf << "0";
				}
				outf << std::endl;
			}
		}

		// For those states that the i-th action cannot generate any observation,
		// overwrite their row of probabilities with O: i-th-action : j-th-state : none 1
		for(unsigned int j = 0; j < stateNoObs.size(); j++)
		{
			outf << "O: " + action_vec[i] + " : " + states_vec[stateNoObs[j]] + " : none 1";
			outf << std::endl;
		}
	}
	else //Non-knowledge producing actions do not have effect in the O function
	{
		// The probability of generating "none" observation by executing a
		// non-knowledge producing action from any state, is 1.
		// That is, non-know. producing actions do not generate observations
		// O: kn-action : * : none 1
		outf << "O: " + action_vec[i] + " : * : none 1" << std::endl << std::endl;
	}
}

//debug
outf.close();
return true;
}






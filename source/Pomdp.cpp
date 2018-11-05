#include <Pomdp.hpp>
#include <sparc2cpp.hpp>
#include <DomainSort.hpp>
#include <PState.hpp>
#include <Action.hpp>
#include <Transition.hpp>
#include <iomanip>

std::tuple<std::vector<int>,std::vector<std::string>> sp::Pomdp::getStats(std::string file,
							       std::vector<std::string> const &actions,
							       std::vector<std::string> const &states,
							       std::vector<std::string> const &obs)
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
		else if(line.find("s") != -1) lines.push_back(line);
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
		//Extract a starting belief state
		if(sp::startsWith(lines[i],"start:") 	     ||
		   sp::startsWith(lines[i],"start include:") ||
		   sp::startsWith(lines[i],"start exclude:")
		)
		{
			//Split with ':' as delimiter
			std::vector<std::string> tok_vec = sp::splitStr(lines[i],":");
			if(tok_vec.size() != 2) continue;
			tok_vec = sp::splitStr(tok_vec[1]," ");

			//Empty definitions are not accepted
			if(tok_vec.size() == 0) continue;
			//Single element definitions
			else if(tok_vec.size() == 1)
			{
				//Check for a specific starting-state using index
				if(sp::isInt(tok_vec[0]))
				{
					int temp_int = std::stoi(tok_vec[0]);
					if(temp_int >= 0 && temp_int < static_cast<int>(states.size()))
					{
						gathAction.push_back(-2);
						gathMat.push_back(lines[i]);
					}
				}
				//Check for the uniform distribution command
				else if(tok_vec[0] == "uniform" && sp::startsWith(lines[i],"start:"))
				{
					gathAction.push_back(-2);
					gathMat.push_back(lines[i]);
				}
				//Check for a specific starting-state using mnemonic state name
				else
				{
					bool state_found = false;
					for(unsigned int j = 0; j < states.size(); j++)
					{
						if(states[j] == tok_vec[0])
						{
							state_found = true;
							break;
						}
					}

					if(state_found)
					{
						gathAction.push_back(-2);
						gathMat.push_back(lines[i]);
					}
				}
			}
			//Multiple elements definition
			else
			{
				bool all_pos_float = true;
				bool non_float = true;
				double total_sum = 0;
				for(unsigned int j = 0; j < tok_vec.size(); j++)
				{
					if(sp::isFloat(tok_vec[j]))
					{
						non_float = false;
						double temp_d = std::stod(tok_vec[j]);
						if(temp_d >= 0) total_sum += temp_d;
						else all_pos_float = false;
					}
					else all_pos_float = false;
				}

				//Check an enumeration of probabilities for each state
				if(all_pos_float && (total_sum == 1) && (tok_vec.size() == states.size()) && sp::startsWith(lines[i],"start:"))
				{
					gathAction.push_back(-2);
					gathMat.push_back(lines[i]);
				}
				//Check for an enumeration of states
				else if(non_float && (tok_vec.size() <= states.size()))
				{
					std::vector<int> int_state;
					std::vector<int>::iterator i_it;
					std::vector<std::string> str_state;
					std::vector<std::string>::iterator s_it;
					for(unsigned int j = 0; j < tok_vec.size(); j++)
					{
						//Check for states' indexes
						if(sp::isInt(tok_vec[j]))
						{
							int temp_int = std::stoi(tok_vec[j]);
							i_it = std::find(int_state.begin(),int_state.end(),temp_int);
							if((temp_int >= 0) && (temp_int < states.size()) && (i_it == int_state.end()))
							{
								int_state.push_back(temp_int);
							}
						}
						//Check for states' mnemonics
						else
						{
							s_it = std::find(str_state.begin(),str_state.end(),tok_vec[j]);
							if(s_it == str_state.end())
							{
								bool state_found = false;
								for(unsigned int k = 0; k < states.size(); k++)
								{
									if(tok_vec[j] == states[k])
									{
										state_found = true;
										break;
									}
								}
								if(state_found) str_state.push_back(tok_vec[j]);
							}
						}
					}

					//If all the elements in tok_vec are non-repeated integers within [0,states.size())
					//or non-repeated strings that are in 'states', is a valid enumeration
					//Also, for the 'start exclude:' the additional restriction of: at least one state must be left out of the enumeration,
					//that is, you cannot exclude all of the states.
					if((int_state.size() == tok_vec.size() || str_state.size() == tok_vec.size()) &&
					   (sp::startsWith(lines[i],"start include:") || (sp::startsWith(lines[i],"start exclude:") && tok_vec.size() < states.size()))
					)
					{
						gathAction.push_back(-2);
						gathMat.push_back(lines[i]);
					}
				}
			}
		}
		//Extract a matrix of prob. for the T and O functions
		else if(lines[i].find("T") != -1 || lines[i].find("O") != -1)
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

std::string sp::Pomdp::fluent2Cass(std::string const &str)
{
	std::string output("");

	if(sp::startsWith(str,"val"))
	{
		//Initial checking for literals with 'val(F,B,I)' format
		int lpc = sp::countOcc(str,"(");
		int rpc = sp::countOcc(str,")");
		int lp = str.find("(");
		int rp = str.rfind(")");
		if(lpc == 0 || lpc != rpc || rp <= lp) return output;

		//Keep the fluent component of the literal
		std::vector<std::string> t_vec;
		t_vec = sp::splitStr(sp::strBtwn(str,lp,rp),",");
		t_vec = sp::correctCommaSplit(t_vec);
		if(t_vec.size() == 3) output = t_vec[0];
		else return output;

	}
	else output = str;

	//Substitute the '(', ')', and commas for '_'
	int len = output.length();
	for(int i = 0; i < len; i++)
	{
		if(output[i] == '(' || output[i] == ')' || output[i] == ',') output[i] = '_';
	}

	return output;
}

sp::Pomdp::Pomdp():
discount(0.95),
values("reward"),
default_prob_T(0.8),
default_prob_O(0.8),
float_precision(8)
{
}

sp::Pomdp::Pomdp(double discount)
{
	default_prob_T = 0.8;
	default_prob_O = 0.8;
	float_precision = 8;
	values = "reward";
	if(discount >= 0 && discount <= 1) this->discount = discount;
	else this->discount = 0.95;
}

sp::Pomdp::Pomdp(std::string values)
{
	default_prob_T = 0.8;
	default_prob_O = 0.8;
	float_precision = 8;
	discount = 0.95;
	if(values == "reward" || values == "cost") this->values = values;
	else this->values = "reward";
}

sp::Pomdp::Pomdp(double discount,std::string values)
{
	default_prob_T = 0.8;
	default_prob_O = 0.8;
	float_precision = 8;
	if(discount >= 0 && discount <= 1) this->discount = discount;
	else this->discount = 0.95;

	if(values == "reward" || values == "cost") this->values = values;
	else this->values = "reward";
}

double sp::Pomdp::getDiscount() const
{
	return discount;
}

std::string sp::Pomdp::getValues() const
{
	return values;
}

bool sp::Pomdp::setDiscount(double const &discount)
{
	if(discount >= 0 && discount <= 1)
	{
		this->discount = discount;
		return true;
	}
	else return false;

}

bool sp::Pomdp::setValues(std::string const &values)
{
	if(values == "reward" || values == "cost")
	{
		this->values = values;
		return true;
	}
	else return false;
}

double sp::Pomdp::getT() const
{
	return default_prob_T;
}

double sp::Pomdp::getO() const
{
	return default_prob_O;
}

bool sp::Pomdp::setT(double const &default_prob_T)
{
	bool set_success = false;
	if(default_prob_T >= 0 && default_prob_T <= 1)
	{
		this->default_prob_T = default_prob_T;
		set_success = true;
	}

	return set_success;
}

bool sp::Pomdp::setO(double const &default_prob_O)
{
	bool set_success = false;
	if(default_prob_O >= 0 && default_prob_O <= 1)
	{
		this->default_prob_O = default_prob_O;
		set_success = true;
	}

	return set_success;
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
	if(S.size() == 0) return false;
	if(Z.size() == 0) return false;
	if(nkA.size() == 0) return false;
	if(kA.size() == 0) return false;
	if(termA.size() == 0) return false;
	if(T.size() == 0) return false;
	if(!ds.fileIsLoaded()) return false;
	if(pomdpFile.length() <= 0) return false;

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
			temp_state_str += fluent2Cass(lits_vec[j]);
		}
		states_vec.push_back(temp_state_str);
		outf << temp_state_str + " ";
	}
	outf << std::endl;
	//---------------------------------------------------------------------------------------------
	//ACTIONS: knA, kA, and termA

	std::vector<std::string> action_vec;
	std::vector<std::string> form_action_vec;
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
		std::string form_temp_a_str(fluent2Cass(temp_a_str));
		form_action_vec.push_back(form_temp_a_str);
		action_vec.push_back(temp_a_str);
		outf << form_temp_a_str;
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
			temp_obs_str += fluent2Cass(lits_vec[j]);
		}

		obs_vec.push_back(temp_obs_str);
		outf << temp_obs_str + " ";
	}
	outf << std::endl;
	//---------------------------------------------------------------------------------------------
	//STARTING BELIEF STATE

	//Extract from the statFile the statistic and value specifications for the T, O, and R functions
	std::tuple<std::vector<int>,std::vector<std::string>> stats;
	stats = getStats(statFile,form_action_vec,states_vec,obs_vec);

	//Get the last start belief state definition gathered from the statistics file
	int sbs = -1;
	for(int i = std::get<0>(stats).size()-1; i >= 0 ; i--)
	{
		if(std::get<0>(stats)[i] == -2)
		{
			sbs = i;
			break;
		}
	}

	//Use the SBS definition provided from the statistics file
	if(sbs > -1) outf << std::get<1>(stats)[sbs] << std::endl << std::endl;
	//Use the default SBS definition
	else outf << "start: uniform" << std::endl << std::endl;
	//---------------------------------------------------------------------------------------------
	//TRANSITION FUNCTION

	//By default the probability assigned to a transition is:
	// 0.8 if the ending state corresponds to the state reached by the deterministic causal law, i.e. the correct transition.
	// The other 0.2 is distributed over the rest of the ending states reached by that action.

	// The transitions in T are indexed over the vectors resulting by:
	// - States: [S,Z]
	// - Actions: [nkA,kA,termA]

	//Build the T-transition matrix for each action
	unsigned int lastNKA = nkA.size()-1;
	unsigned int lastKA = kA.size() + lastNKA;
	for(unsigned int i = 0; i < action_vec.size(); i++)
	{
		//Check if the i-th action's matrix for the T-function was extracted from the statFile
		bool action_from_file = false;
		for(unsigned int j = 0; j < std::get<0>(stats).size(); j++)
		{
			//The j-th index corresponds to the i-th action
			if(i == std::get<0>(stats)[j])
			{
				//The matrix is for the T-function
				std::string temp_T = std::get<1>(stats)[j];
				if(temp_T.substr(0,1) == "T")
				{
					if(temp_T.find("identity") != -1 || temp_T.find("uniform") != -1)
					{
						std::vector<std::string> temp_vec = sp::splitStr(temp_T,"\n");
						temp_vec = sp::splitStr(temp_vec[0],":");
						std::string temp_action_str = sp::stripSpaces(temp_vec[1]);

						outf << "T: " + temp_action_str << std::endl;
						if(temp_T.find("identity") != -1)
						{
							for(unsigned int k = 0; k < S.size(); k++)
							{
								for(unsigned int l = 0; l < S.size(); l++)
								{
									if(l > 0) outf << " ";

									if(k == l) outf << "1";
									else outf << "0";
								}
								outf << std::endl;
							}
						}
						else
						{
							double uniform_prob = 1 / static_cast<double>(S.size());
							for(unsigned int k = 0; k < S.size(); k++)
							{
								for(unsigned int l = 0; l < S.size(); l++)
								{
									if(l > 0) outf << " ";

									outf << uniform_prob;
								}
								outf << std::endl;
							}

						}
						outf << std::endl;
					}
					else
					{
						//Write the matrix in the POMDP file
						outf << std::get<1>(stats)[j] << "\n";
					}

					action_from_file = true;
					break;
				}
			}
		}
		if(action_from_file) continue;

		//Non-knowledge producing actions have effect in the T function
		if(i <= lastNKA) 
		{
			outf << "T: " + form_action_vec[i] << std::endl;

			//Iterate over the rows of the matrix
			for(unsigned int j = 0; j < S.size(); j++)
			{
				//Gather all the transitions that start in the j-th state and have the i-th action
				std::vector<int> relT;
				for(unsigned int k = 0; k < T.size(); k++)
				{

					if(T[k].iA() == static_cast<int>(i) && T[k].iS0() == static_cast<int>(j))
					{
						relT.push_back(k);
					}
				}

				//There are no transitions with these S0 & A
				if(relT.size() == 0)
				{
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

				//Write the default probabilities using the detemrinistic causal laws
				//and the default_prob_T private memeber 
				std::vector<sp::PState> emptyZ;
				writeDefMat(ds.SourceFile,j,action_vec[i],S,emptyZ,T,relT,outf);
			}

			outf << std::endl;
		}
		//Knowledge producing actions do not have effect in the T function
		else if(i <= lastKA) 
		{
			outf << "T: " + form_action_vec[i] << std::endl;

			//Write the identity matrix explicitly, given that AI-Toolbox does not
			//support the "uniform" and "identity" keywords
			for(unsigned int j = 0; j < S.size(); j++)
			{
				for(unsigned int k = 0; k < S.size(); k++)
				{
					if(k > 0) outf << " ";

					if(j == k) outf << "1";
					else outf << "0";
				}
				outf << std::endl;
			}
			outf << std::endl;
		}
		//POMDP execution termination actions
		else 
		{
			//Tarminate actions always trasit the system to the absorbing state absb
			outf << "T: " + form_action_vec[i] + " : * : absb 1" << std::endl << std::endl;
		}
	}

	//---------------------------------------------------------------------------------------------
	//OBSERVATION FUNCTION

	for(unsigned int i = 0; i < action_vec.size(); i++)
	{
		//Check if the i-th action's matrix for the T-function was extracted from the statFile
		bool action_from_file = false;
		for(unsigned int j = 0; j < std::get<0>(stats).size(); j++)
		{
			//The j-th index corresponds to the i-th action
			if(i == std::get<0>(stats)[j])
			{
				//The matrix is for the O-function
				std::string temp_O = std::get<1>(stats)[j];
				if(temp_O.substr(0,1) == "O")
				{
					if(temp_O.find("uniform") != -1)
					{
						std::vector<std::string> temp_vec = sp::splitStr(temp_O,"\n");
						temp_vec = sp::splitStr(temp_vec[0],":");
						std::string temp_action_str = sp::stripSpaces(temp_vec[1]);

						outf << "O: " + temp_action_str << std::endl;
						double uniform_prob = 1 / static_cast<double>(Z.size());
						for(unsigned int k = 0; k < S.size(); k++)
						{
							for(unsigned int l = 0; l < Z.size(); l++)
							{
								if(l > 0) outf << " ";

								outf << uniform_prob;
							}
							outf << std::endl;
						}
						outf << std::endl;
					}
					else
					{
						//Write the matrix in the POMDP file
						outf << std::get<1>(stats)[j] << "\n";
					}

					action_from_file = true;
					break;
				}
			}
		}
		if(action_from_file) continue;

		//Knowledge producing actions have effect in the O-function
		if(i > lastNKA && i <= lastKA)
		{
			outf << "O: " + form_action_vec[i] << std::endl;

			//Itareate over the states (rows)
			for(unsigned int j = 0; j < S.size(); j++)
			{
				//Gather those transitions of the form S[j]-A[i]-Z[*]
				std::vector<int> relT;
				for(unsigned int k = 0; k < T.size(); k++)
				{
					if(T[k].iS0() == static_cast<int>(j) &&
					   T[k].iA() == static_cast<int>(i))
						relT.push_back(k);
				}

				//No observation can be generated from state-j and action-i
				if(relT.size() == 0)
				{
					for(unsigned k = 0; k < Z.size(); k++)
					{
						if(k > 0) outf << " ";

						//Observation 'none' gets 1 of probability, that is,
						//the last element of 'Z'
						if(k == (Z.size()-1)) outf << "1";
						else outf << "0";
					}
					outf << std::endl;
					continue;
				}

				//A single observation is generated deterministially
				//from state-j and action-i
				if(relT.size() == 1)
				{
					for(unsigned int k = 0; k < Z.size(); k++)
					{
						if(k > 0) outf << " ";

						//Given that 'T' was indexed over [S,Z], the size of 'S'
						//must be substracted in order for an index of 'Z' to
						// match
						int rel_index = T[relT[0]].iS1() - S.size();

						if(rel_index == static_cast<int>(k)) outf << "1";
						else outf << "0";
					}
					outf << std::endl;
					continue;
				}

				//Write the default probabilities using the detemrinistic causal laws
				//and the default_prob_O private memeber 
				writeDefMat(ds.SourceFile,j,action_vec[i],S,Z,T,relT,outf);
			}

			outf << std::endl;
		}
		//Non-knowledge producing and termination actions do not have effect in the O function
		else 
		{
			// The probability of generating "none" observation by executing a
			// non-knowledge producing action or a termination from any state, is 1.
			// That is, non-know. producing actions do not generate observations
			// O: kn-action : * : none 1
			outf << "O: " + form_action_vec[i] + " : * : none 1" << std::endl << std::endl;
		}
	}

	//---------------------------------------------------------------------------------------------
	//REWARD FUNCTION

	for(unsigned int i = 0; i < std::get<0>(stats).size(); i++)
	{
		//Get each reward function specification
		if(std::get<0>(stats)[i] == -1)
		{
			outf << std::get<1>(stats)[i] << "\n\n";
		}
	}
	//---------------------------------------------------------------------------------------------

	//Close the pomdp file
	outf.close();

	return true;
}

void sp::Pomdp::writeDefMat(std::string const &sparcF,
			    int const &s0,
			    std::string const &a_str,
			    std::vector<PState> const &S,
			    std::vector<PState> const &Z,
			    std::vector<Transition> const &T,
			    std::vector<int> const &relT,
			    std::ofstream &outf)
{
	//Create a copy of the original SPARC file to find the deterministic transition
	sp::shellCmd("rm temp_sp_file.sp");
	sp::shellCmd("cp "+sparcF+" temp_sp_file.sp");
	std::ofstream temf;
	temf.open("temp_sp_file.sp",std::ofstream::app);
	if(!temf.is_open())
	{
		return;
	}

	//Add the S0 state to the copy file
	int state_len = S[s0].str().length();
	std::string all_lits = S[s0].str().substr(1,state_len-2);
	std::vector<std::string> lits_vec = sp::splitStr(all_lits,",");
	lits_vec = sp::correctCommaSplit(lits_vec);
	for(unsigned int i = 0; i < lits_vec.size(); i++) temf << lits_vec[i]+".\n";

	//Add the action to the copy file
	std::vector<std::string> temp_a_vec = sp::splitStr(a_str,"-");
	for(unsigned int i = 0; i < temp_a_vec.size(); i++)
	{
		temf << "occurs("+temp_a_vec[i]+",0).\n";
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

	//Change the step value of every literal in temp_S1[0] to 0
	for(unsigned int j = 0; j < temp_S1[0].LitNames.size(); j++)
	{
		for(unsigned int k = 0; k < temp_S1[0].LitArgs[j].size(); k++)
		{
			//That last argument that is "1" is changed to "0"
			if((temp_S1[0].LitArgs[j][k] == "1") && (k == temp_S1[0].LitArgs[j].size()-1))
			{
				temp_S1[0].LitArgs[j][k] = "0";
			}
		}
	}

	//From the relevant transitions find the one that matches the deterministic transition
	int detT = -1;
	for(unsigned int i = 0; i < relT.size(); i++)
	{
		//Check for the deterministic reached state S1
		if(Z.size() == 0)
		{
			if(temp_S1[0].contains(S[T[relT[i]].iS1()]))
			{
				detT = relT[i];
				break;
			}
		}
		else
		{
			if(temp_S1[0].contains(Z[T[relT[i]].iS1() - S.size()]))
			{
				detT = relT[i];
				break;
			}
		}
	}

	double default_prob = 0;
	double others_prob = 0;
	int nCols = 0;
	int offset = 0;
	//T-function is being built
	if(Z.size() == 0)
	{
		default_prob = default_prob_T;
		others_prob = (1 - default_prob_T) / static_cast<double>(relT.size()-1);
		nCols = S.size();
	}
	//O-function is being built
	else
	{
		default_prob = default_prob_O;
		others_prob = (1 - default_prob_O) / static_cast<double>(relT.size()-1);
		nCols = Z.size();
		offset = S.size();
	}

	//Assign the probaibilities for the s0-state's row
	outf << std::fixed << std::setprecision(float_precision);
	for(int i = 0; i < nCols; i++)
	{
		if(i > 0) outf << " ";

		//The det. transition
		if((i + offset) == T[detT].iS1())
		{
			outf << default_prob;
			continue;
		}

		bool i_is_rel = false;
		for(unsigned int l = 0; l < relT.size(); l++)
		{
			//A non-det. but relevant transition
			if((i + offset) == T[relT[l]].iS1())
			{
				outf << others_prob;
				i_is_rel = true;
				break;
			}
		}

		//A not relevant transition
		if(!i_is_rel) outf << "0";
	}
	outf << std::endl;

	return;
}


bool sp::Pomdp::saveTD(std::vector<PState> const &S,
	    	       std::vector<PState> const &Z,
	    	       std::vector<Action> const &nkA,
	    	       std::vector<Action> const &kA,
	    	       std::vector<Action> const &termA,
	    	       std::vector<Transition> const &T,
	    	       std::string const &tdFile)
{
	//Create the transition-diagram file
	std::ofstream outf;
	outf.open(tdFile,std::ofstream::out);

	//Initial verification
	if(!outf.is_open()   ||
	   S.size()     == 0 ||
	   Z.size()     == 0 ||
	   nkA.size()   == 0 ||
	   kA.size()    == 0 ||
	   termA.size() == 0 ||
	   T.size()     == 0
	)
	{
		return false;
	}

	std::vector<std::string> headlines;
	headlines.push_back("TD - STATES:");
	headlines.push_back("TD - OBSERVATIONS:");
	headlines.push_back("TD - NON-KNOWLEGDE-PROD-ACTIONS:");
	headlines.push_back("TD - KNOWLEGDE-PROD-ACTIONS:");
	headlines.push_back("TD - TERMINATION-ACTIONS:");
	headlines.push_back("TD - TRANSITIONS:");
	std::vector<unsigned long int> headlinesNum;
	headlinesNum.push_back(1);
	headlinesNum.push_back(headlinesNum[headlinesNum.size()-1] + 1 + S.size());
	headlinesNum.push_back(headlinesNum[headlinesNum.size()-1] + 1 + Z.size());
	headlinesNum.push_back(headlinesNum[headlinesNum.size()-1] + 1 + nkA.size());
	headlinesNum.push_back(headlinesNum[headlinesNum.size()-1] + 1 + kA.size());
	headlinesNum.push_back(headlinesNum[headlinesNum.size()-1] + 1 + termA.size());

	//Start computing the file's checksum
	unsigned long int cs = 0;
	for(unsigned int i = 0; i < headlines.size(); i++)
	{
		cs += (headlinesNum[i] * sp::checkSum(headlines[i]));
	}

	//Save all states in 'S'
	outf << headlines[0] << std::endl;
	for(unsigned int i = 0; i < S.size(); i++)
	{
		outf << S[i].str() << std::endl;

		//Compute checksum for each line
		cs += ((headlinesNum[0]+i+1) * sp::checkSum(S[i].str()));
	}

	//Save all observations in 'Z'
	outf << headlines[1] << std::endl;
	for(unsigned int i = 0; i < Z.size(); i++)
	{
		outf << Z[i].str() << std::endl;

		//Compute checksum for each line
		cs += ((headlinesNum[1]+i+1) * sp::checkSum(Z[i].str()));
	}

	//Save all non-knowledge producing actions in 'nkA'
	outf << headlines[2] << std::endl;
	for(unsigned int i = 0; i < nkA.size(); i++)
	{
		outf << nkA[i].str() << std::endl;

		//Compute checksum for each line
		cs += ((headlinesNum[2]+i+1) * sp::checkSum(nkA[i].str()));
	}

	//Save all knowledge producing actions in 'kA'
	outf << headlines[3] << std::endl;
	for(unsigned int i = 0; i < kA.size(); i++)
	{
		outf << kA[i].str() << std::endl;

		//Compute checksum for each line
		cs += ((headlinesNum[3]+i+1) * sp::checkSum(kA[i].str()));
	}

	//Save all execution termination actions in 'termA'
	outf << headlines[4] << std::endl;
	for(unsigned int i = 0; i < termA.size(); i++)
	{
		outf << termA[i].str() << std::endl;

		//Compute checksum for each line
		cs += ((headlinesNum[4]+i+1) * sp::checkSum(termA[i].str()));
	}

	//Save all the transitions in 'T'
	outf << headlines[5] << std::endl;
	for(unsigned int i = 0; i < T.size(); i++)
	{
		outf << T[i].ps0.str() << std::endl;
		outf << T[i].SetofActions.str() << std::endl;
		outf << T[i].ps1.str() << std::endl;

		//Compute checksum for each line
		cs += ((headlinesNum[5]+(i*3)+1) * sp::checkSum(T[i].ps0.str()));
		cs += ((headlinesNum[5]+(i*3)+2) * sp::checkSum(T[i].SetofActions.str()));
		cs += ((headlinesNum[5]+(i*3)+3) * sp::checkSum(T[i].ps1.str()));
	}

	//Save the resulting checksum value
	outf << cs;

	//Close the transition-diagram file
	outf.close();

	return true;
}

bool sp::Pomdp::loadTD(std::vector<PState> &S,
		       std::vector<PState> &Z,
	    	       std::vector<Action> &nkA,
	    	       std::vector<Action> &kA,
	    	       std::vector<Action> &termA,
	    	       std::vector<Transition> &T,
	    	       std::string const &tdFile)
{
	//Check for a valid file
	std::ifstream infile;
	infile.open(tdFile,std::ifstream::in);
	if(!infile.is_open()) return false;

	//Read the file
	std::string line;
	std::vector<std::string> lines;
	while(std::getline(infile,line)) lines.push_back(line);
	infile.close();

	//Perform the checksum verification
	std::string cs_str = lines[lines.size()-1];
	if(!sp::isInt(cs_str)) return false;
	unsigned long int cs = std::stoul(cs_str);
	unsigned long int file_cs = 0;
	for(unsigned int i = 0; i < lines.size()-1; i++)
	{
		file_cs += ((i+1) * sp::checkSum(lines[i]));
	}
	if(cs != file_cs) return false;

	//Clean-up the output vectors
	S.clear();
	Z.clear();
	nkA.clear();
	kA.clear();
	termA.clear();
	T.clear();

	//Extract the transition diagram
	int phase = 0;
	sp::PState t_ps0;
	sp::Action t_action;
	int t_count = 0;
	for(unsigned int i = 0; i < lines.size(); i++)
	{
		if(lines[i] == "TD - STATES:")
		{
			phase = 1;
			continue;
		}
		else if(lines[i] == "TD - OBSERVATIONS:")
		{
			phase = 2;
			continue;
		}
		else if(lines[i] == "TD - NON-KNOWLEGDE-PROD-ACTIONS:")
		{
			phase = 3;
			continue;
		}
		else if(lines[i] == "TD - KNOWLEGDE-PROD-ACTIONS:")
		{
			phase = 4;
			continue;
		}
		else if(lines[i] == "TD - TERMINATION-ACTIONS:")
		{
			phase = 5;
			continue;
		}
		else if(lines[i] == "TD - TRANSITIONS:")
		{
			phase = 6;
			continue;
		}

		//States or Observations
		if(phase == 1 || phase == 2)
		{
			sp::PState temp_ps(lines[i]);
			if(phase == 1) S.push_back(temp_ps);//States are being readed
			else if(phase == 2) Z.push_back(temp_ps);//Observations are being readed
		}
		//NK, K, or Term. actions
		else if(phase >= 3 && phase <= 5)
		{
			int lb = lines[i].find("{");
			int rb = lines[i].find("}");
			std::string lits_str = sp::strBtwn(lines[i],lb,rb);
			std::vector<std::string> a_vec = sp::splitStr(lits_str,",");
			a_vec = sp::correctCommaSplit(a_vec);
			sp::Action temp_a(a_vec);

			if(phase == 3) nkA.push_back(temp_a);//NK-actions are being readed
			else if(phase == 4) kA.push_back(temp_a);//K-actions are being readed
			else if(phase == 5) termA.push_back(temp_a);//Termination-actions are being readed
		}
		//Transitions are being readed
		else if(phase == 6 && sp::startsWith(lines[i],"{"))
		{
			//Read state-0
			if(t_count == 0)
			{
				t_count++;
				t_ps0.set(lines[i]);
			}
			//Read action
			else if(t_count == 1)
			{
				t_count++;
				int lb = lines[i].find("{");
				int rb = lines[i].find("}");
				std::string lits_str = sp::strBtwn(lines[i],lb,rb);
				std::vector<std::string> a_vec = sp::splitStr(lits_str,",");
				a_vec = sp::correctCommaSplit(a_vec);
				t_action.set(a_vec);
			}
			//Read state-1
			else
			{
				t_count = 0;
				sp::PState temp_ps1(lines[i]);
				sp::Transition temp_t(t_action,t_ps0,temp_ps1);
				T.push_back(temp_t);
			}
		}
	}

	return true;
}

bool sp::Pomdp::whatStats(std::vector<PState> const &Si,
		          std::vector<PState> const &Zi,
	    	          std::vector<Action> const &nkA,
	    	          std::vector<Action> const &kA,
	    	          std::vector<Transition> &T,
	    	          std::string const &guideFile)
{
	//Open output file
	std::ofstream outf;
	outf.open(guideFile,std::ofstream::out);

	//Initial verification
	if(!outf.is_open()   ||
	   Si.size()    == 0 ||
	   Zi.size()    == 0 ||
	   nkA.size()   == 0 ||
	   kA.size()    == 0 ||
	   T.size()     == 0
	)
	{
		return false;
	}

	//Create vector of alphabetic indexes
	std::vector<char> index;
	for(int i = 97; i <= 122; i++) index.push_back(static_cast<char>(i));

	//Create a vector with the action names as they will appear in the Cassandra file
	std::vector<sp::Action> AA;
	AA.insert(AA.end(),nkA.begin(),nkA.end());
	AA.insert(AA.end(),kA.begin(),kA.end());
	std::vector<std::string> read_vec;
	std::vector<std::string> cass_vec;
	for(unsigned int i = 0; i < AA.size(); i++)
	{
		std::string temp_a_str("");
		for(unsigned int j = 0; j < AA[i].ActionNames.size(); j++)
		{
			if(j > 0) temp_a_str += "-";
			temp_a_str += AA[i].ActionNames[j];
			if(AA[i].ActionArgs[j].size() > 0)
			{
				temp_a_str += "(";
				for(unsigned int k = 0; k < AA[i].ActionArgs[j].size(); k++)
				{
					if(k > 0) temp_a_str += ",";
					temp_a_str += AA[i].ActionArgs[j][k];
					if(AA[i].ActionArgArgs[j][k].size() > 0)
					{
						temp_a_str += "(";
						for(unsigned int l = 0; l < AA[i].ActionArgArgs[j][k].size(); l++)
						{
							if(l > 0) temp_a_str += ",";
							temp_a_str += AA[i].ActionArgArgs[j][k][l];
						}
						temp_a_str += ")";
					}
				}
				temp_a_str += ")";
			}
		}
		read_vec.push_back(temp_a_str);
		cass_vec.push_back(fluent2Cass(temp_a_str));
	}

	//Create copy of States and Observation vectors
	std::vector<PState> S = Si;
	std::vector<PState> Z = Zi;

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

	//Index vector 'T' over ['S','Z'] and ['nkA','kA']
	std::vector<sp::PState> SZ;
	SZ.insert(SZ.end(),S.begin(),S.end());
	SZ.insert(SZ.end(),Z.begin(),Z.end());
	sp::Transition::index(T,SZ,AA);

	//Instructions on how to read the output file
	outf << "-----------------------------------" << std::endl;
	outf << "WHICH-STATS?: HOW TO READ THIS FILE" << std::endl;
	outf << "-----------------------------------" << std::endl;
	outf << "This file is intended to illustrate the POMDP designer which statistics are re-" << std::endl;
	outf << "quired, in order to provide more realistic transition probabilities than those" << std::endl;
	outf << "used in the default version." << std::endl;
	outf << "" << std::endl;
	outf << "This file is made up two main sections, the Transition function and Observation " << std::endl;
	outf << "function. Each section is constituted by the action-matrices for which their sta-" << std::endl;
	outf << "tistics are required." << std::endl;
	outf << "" << std::endl;
	outf << "Above each action matrix, one can find the transitions that require their tran-" << std::endl;
	outf << "sition probability, along with an alphabetic index, i.e., 'a', 'b', etc. These" << std::endl;
	outf << "alphabetic indexes can also be found in the immediate next action-matrix. Once" << std::endl;
	outf << "the probability of a transition has been computed, this one should replace its" << std::endl;
	outf << "corresponding alphabetic index within the action matrix, in the 'stats' file " << std::endl;
	outf << "that is passed to the sp::Pomdp::generatePomdp method." << std::endl;

	for(unsigned int i = 0; i < read_vec.size(); i++)
	{
		if(i == 0) outf << "############################# TRANSITION FUNCTION #############################" << std::endl;
		if(i == nkA.size()) outf << "############################# OBSERVATION FUNCTION #############################" << std::endl;

		if(i > 0 && i != nkA.size()) outf <<  "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;

		outf << "READABLE  FORMAT ACTION: " + read_vec[i] << std::endl;
		outf << "CASSANDRA FORMAT ACTION: " + cass_vec[i] << std::endl << std::endl;

		//Gather the transitions that occur with the i-th action
		std::vector<unsigned int> relT;
		for(unsigned int j = 0; j < T.size(); j++)
		{
			if(static_cast<unsigned int>(T[j].iA()) == i)
			{
				if((i < nkA.size() && T[j].iS1() < S.size())  || //Gather a T-function transition
				   (i >= nkA.size() && T[j].iS1() >= S.size()) ) //Gather a O-function transition
				{
					relT.push_back(j);
				}
			}
		}

		//Print the gathered transitions
		std::vector<std::string> a_indexes;
		for(unsigned int j = 0; j < relT.size(); j++)
		{
			std::string s_s0, s_s1;
			s_s0 = SZ[T[relT[j]].iS0()].str(); //Starting state string
			s_s1 = SZ[T[relT[j]].iS1()].str(); //Ending state string

			//Construct the transition's alphabetic index
			std::string alpha_i("");
			int div = j / index.size();
			int res = j % index.size();
			alpha_i.append((div+1),index[res]);
			a_indexes.push_back(alpha_i);

			//Write the transition
			if(j > 0) outf << "---------------------------" << std::endl;
			outf << s_s0 << std::endl;
			outf << "     |" << std::endl;
			outf << "     " + alpha_i << std::endl;
			outf << "     |" << std::endl;
			outf << "     v" << std::endl;
			outf << s_s1 << std::endl;
		}

		//Print the matrix for the i-th action, using the alphabetic indexes to show the position of the matrix
		int cols;
		std::string fun_type;
		if(i < nkA.size())
		{
			cols = S.size();
			fun_type = "T: ";
		}
		else
		{
			cols = Z.size();
			fun_type = "O: ";
		}
		outf << "---------------------------" << std::endl << fun_type + cass_vec[i] << std::endl;
		for(unsigned int j = 0; j < S.size(); j++)
		{
			//Gather the transitions that start in the j-th state
			std::vector<int> temp;
			for(unsigned int k = 0; k < relT.size(); k++)
			{
				if(j == T[relT[k]].iS0()) temp.push_back(k);
			}

			//Print the j-th row
			if(temp.size() == 0)
			{
				//There are no transitions that start in the j-th state
				if(i < nkA.size())
				{
					//For T-mats, a 1 in the diagonal
					for(int k = 0; k < cols; k++)
					{
						if(k > 0) outf << " ";

						if(k == j) outf << "1";
						else outf << "0";
					}
				}
				else
				{
					//For O-mats, a 1 in the final element, which corresponds to the 'none' observation
					for(int k = 0; k < (cols-1); k++)
					{
						if(k > 0) outf << " ";
						outf << "0";
					}
					outf << " 1";
				}
			}
			else
			{
				//There are transitions that start in the j-th state
				for(int k = 0; k < cols; k++)
				{
					if(k > 0) outf << " ";

					//Offset required when comparing observation-indexes to 'k'
					int offset = 0;
					if(i >= nkA.size()) offset = S.size();

					bool trans_found = false;
					for(unsigned int l = 0; l < temp.size(); l++)
					{
						if(k == (T[relT[temp[l]]].iS1() - offset))
						{
							outf << a_indexes[temp[l]];
							trans_found = true;
							break;
						}
					}
					if(!trans_found) outf << "0";

				}
			}
			outf << std::endl;
		}
	}

	//Close the output file
	outf.close();

	return true;
}

std::vector<std::vector<std::string>> sp::Pomdp::readSAZ(std::string const &pomdpFile)
{
	//Output lists
	std::vector<std::vector<std::string>> sazLists;

	//Check for a valid file
	std::ifstream infile;
	infile.open(pomdpFile,std::ifstream::in);
	if(!infile.is_open()) return sazLists;

	//Read the file
	std::string line;
	std::vector<std::string> lines;
	while(std::getline(infile,line)) lines.push_back(line);
	infile.close();

	//Keep those lines that correspond to states, actions and observations
	std::string read_state("");
	std::vector<std::string> s_vec;
	std::vector<std::string> a_vec;
	std::vector<std::string> z_vec;
	for(unsigned int i = 0; i < lines.size(); i++)
	{
		if(lines[i].find(":") != -1)
		{
			//States, actoins, and observations were already read
			if(!sp::startsWith(lines[i],"states")       &&
			   !sp::startsWith(lines[i],"actions")      &&
			   !sp::startsWith(lines[i],"observations") &&
			   s_vec.size() > 0			    &&
			   a_vec.size() > 0			    &&
			   z_vec.size() > 0)
				break;

			//Determine which list is going to be read
			if(sp::startsWith(lines[i],"states")) read_state = "states";
			else if(sp::startsWith(lines[i],"actions")) read_state = "actions";
			else if(sp::startsWith(lines[i],"observations")) read_state = "observations";
			else read_state = "";
		}

		//Keep those lines of interest in their respective vector
		if(read_state == "states" && lines[i].length() > 0) s_vec.push_back(lines[i]);
		else if(read_state == "actions" && lines[i].length() > 0) a_vec.push_back(lines[i]);
		else if(read_state == "observations" && lines[i].length() > 0) z_vec.push_back(lines[i]);
	}

	//Extract the list of states
	std::vector<std::string> s_list;
	for(unsigned int i = 0; i < s_vec.size(); i++)
	{
		std::vector<std::string> temp_vec;

		//The line containing the "states:" keyword
		if(s_vec[i].find(":") != -1)
		{
			temp_vec = sp::splitStr(s_vec[i],":");
			temp_vec = sp::splitStr(temp_vec[1]," ");
			s_list.insert(s_list.end(),temp_vec.begin(),temp_vec.end());
		}
		else
		{
			temp_vec = sp::splitStr(s_vec[i]," ");
			s_list.insert(s_list.end(),temp_vec.begin(),temp_vec.end());
		}
	}

	//Extract the list of actions
	std::vector<std::string> a_list;
	for(unsigned int i = 0; i < a_vec.size(); i++)
	{
		std::vector<std::string> temp_vec;

		//The line containing the "states:" keyword
		if(a_vec[i].find(":") != -1)
		{
			temp_vec = sp::splitStr(a_vec[i],":");
			temp_vec = sp::splitStr(temp_vec[1]," ");
			a_list.insert(a_list.end(),temp_vec.begin(),temp_vec.end());
		}
		else
		{
			temp_vec = sp::splitStr(a_vec[i]," ");
			a_list.insert(a_list.end(),temp_vec.begin(),temp_vec.end());
		}
	}

	//Extract the list of observations
	std::vector<std::string> z_list;
	for(unsigned int i = 0; i < z_vec.size(); i++)
	{
		std::vector<std::string> temp_vec;

		//The line containing the "states:" keyword
		if(z_vec[i].find(":") != -1)
		{
			temp_vec = sp::splitStr(z_vec[i],":");
			temp_vec = sp::splitStr(temp_vec[1]," ");
			z_list.insert(z_list.end(),temp_vec.begin(),temp_vec.end());
		}
		else
		{
			temp_vec = sp::splitStr(z_vec[i]," ");
			z_list.insert(z_list.end(),temp_vec.begin(),temp_vec.end());
		}
	}

	//Build the final vector of lists
	sazLists.push_back(s_list);
	sazLists.push_back(a_list);
	sazLists.push_back(z_list);

	return sazLists;
}























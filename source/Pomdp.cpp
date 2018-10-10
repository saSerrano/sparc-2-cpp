#include <Pomdp.hpp>
#include <sparc2cpp.hpp>
#include <DomainSort.hpp>
#include <PState.hpp>
#include <Action.hpp>
#include <Transition.hpp>
#include <tuple>

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
                              std::vector<sp::Action> const &nkA, 
                              std::vector<sp::Action> const &kA, 
                              std::vector<sp::Action> const &termA, 
                              std::vector<sp::PState> &Z, 
                              std::vector<sp::Transition> const &T, 
                              sp::DomainSort const &ds, 
                              std::string const &pomdpFile)
{
	//Input parameters checking
	//if(S.size() == 0) return false;
	//if(nkA.size() == 0) return false;
	//if(kA.size() == 0) return false;
	//if(termA.size() == 0) return false;
	//if(Z.size() == 0) return false;
	//if(T.size() == 0) return false;
	//if(!ds.fileIsLoaded()) return false;

	//Verify if the "absorbing" state is within S, if not, add it
	sp::PState absb_state("{absb}");
	bool absb_is_included = false;
	for(unsigned int i = 0; i < S.size(); i++)
	{
		if(S[i].equals(absb_state))
		{
			absb_is_included = true;
			break;
		}
	}
	if(!absb_is_included) S.push_back(absb_state);

	//Verify if the "none" observation is within Z, if not, add it
	sp::PState none_obs("{none}");
	bool none_is_included = false;
	for(unsigned int i = 0; i < Z.size(); i++)
	{
		if(Z[i].equals(none_obs))
		{
			none_is_included = true;
			break;
		}
	}
	if(!none_is_included) Z.push_back(none_obs);

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
		std::string temp_state_str("");
		if(S[i].len() == 1)
		{
			//For single-literal states
			temp_state_str = S[i].str().substr(1,state_len-2);
			states_vec.push_back(temp_state_str);
			outf << S[i].str().substr(1,state_len-2) + " ";
		}
		else
		{
			//For states with more than 1 literal
			std::string all_lits = S[i].str().substr(1,state_len-2);
			std::vector<std::string> lits_vec = splitStr(all_lits,",");
			lits_vec = correctCommaSplit(lits_vec);
			for(unsigned int j = 0; j <  lits_vec.size(); j++)
			{
				if(j > 0) temp_state_str += "-";
				temp_state_str += lits_vec[j];
			}
		}
		states_vec.push_back(temp_state_str);
		outf << temp_state_str + " ";
	}
	outf << std::endl;
	//---------------------------------------------------------------------------------------------
	//ACTIONS
	//Non-knowledge producing actions
	std::vector<std::string> action_vec;
	outf << "actions: ";
	for(unsigned int i = 0; i < nkA.size(); i++)
	{
		std::string temp_a_str("");
		unsigned int conc_actions = nkA[i].ActionNames.size();
		for(unsigned int j = 0; j < conc_actions; j++)
		{
			if(j > 0) temp_a_str += "-";
			temp_a_str += nkA[i].ActionNames[j];
			if(nkA[i].ActionArgs[j].size() > 0)
			{
				temp_a_str += "(";
				for(unsigned int k = 0; k < nkA[i].ActionArgs[j].size(); k++)
				{
					if(k > 0) temp_a_str += ",";
					temp_a_str += nkA[i].ActionArgs[j][k];
				}
				temp_a_str += ")";
			}
		}
		action_vec.push_back(temp_a_str);
		outf << temp_a_str;
		outf << " ";
	}
	//Knowledge producing actions
	for(unsigned int i = 0; i < kA.size(); i++)
	{
		std::string temp_a_str("");
		unsigned int conc_actions = kA[i].ActionNames.size();
		for(unsigned int j = 0; j < conc_actions; j++)
		{
			if(j > 0) temp_a_str += "-";
			temp_a_str += kA[i].ActionNames[j];
			if(kA[i].ActionArgs[j].size() > 0)
			{
				temp_a_str += "(";
				for(unsigned int k = 0; k < kA[i].ActionArgs[j].size(); k++)
				{
					if(k > 0) temp_a_str += ",";
					temp_a_str += kA[i].ActionArgs[j][k];
				}
				temp_a_str += ")";
			}
		}
		action_vec.push_back(temp_a_str);
		outf << temp_a_str;
		outf << " ";
	}
	//POMDP terminate execution actions
	for(unsigned int i = 0; i < termA.size(); i++)
	{
		std::string temp_a_str("");
		unsigned int conc_actions = termA[i].ActionNames.size();
		for(unsigned int j = 0; j < conc_actions; j++)
		{
			if(j > 0) temp_a_str += "-";
			temp_a_str += termA[i].ActionNames[j];
			if(termA[i].ActionArgs[j].size() > 0)
			{
				temp_a_str += "(";
				for(unsigned int k = 0; k < termA[i].ActionArgs[j].size(); k++)
				{
					if(k > 0) temp_a_str += ",";
					temp_a_str += termA[i].ActionArgs[j][k];
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
	outf << "observations: ";
	for(unsigned int i = 0; i < Z.size(); i++)
	{
		int state_len = Z[i].str().length();
		if(Z[i].len() == 1)
		{
			//For single-literal observations
			outf << Z[i].str().substr(1,state_len-2) + " ";
		}
		else
		{
			//For observations with more than 1 literal
			std::string all_lits = Z[i].str().substr(1,state_len-2);
			std::vector<std::string> lits_vec = splitStr(all_lits,",");
			lits_vec = correctCommaSplit(lits_vec);
			for(unsigned int j = 0; j <  lits_vec.size(); j++)
			{
				if(j > 0) outf << "-";
				outf << lits_vec[j];
			}
			outf << " ";
		}
	}
	outf << std::endl << std::endl;
	//---------------------------------------------------------------------------------------------
//TRANSITION FUNCTION

//By default the probability assigned to a transition is:
// 0.8 if the ending state corresponds to the state reached by the deterministic causal law, i.e. the correct transition.
// The other 0.2 is distributed over the rest of the ending states reached by that action.

// The transitions in T are indexed over the vectors resulting by:
// - States: concatenating Z at the end of S
// - Actions: concatenating kA at the end of nkA

//debug
std::cout << "++++Transitions++++" << std::endl;
for(unsigned int i = 0; i < T.size(); i++)
{
std::cout << T[i].iS0() << "," << T[i].iA() << "," << T[i].iS1() << std::endl;
}
std::cout << "+++++++++++++++++++" << std::endl;

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

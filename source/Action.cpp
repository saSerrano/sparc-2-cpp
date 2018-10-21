#include <Action.hpp>
#include <sparc2cpp.hpp>


bool sp::Action::exractActionsFromString(std::vector<std::string> actionStrings)
{
	/*
	In order for a string to be accepted as a valid action it must be of the form:

		occurs(A,I)
		A: actionName(FirstArg,SecondArg,ThirdArg, ...)
		I: a positive integer

	The predicate 'occurs' must have only 2 arguments, where  the second one has to
	be a positive integer. The first argument is a predicate that can have as many
	arguments are desired.

	Only if all the action strings in 'actionStrings' are accepted, they will be
	stored in this transition.
	*/

	std::vector<std::string> actionNames;
	std::vector<std::vector<std::string>> actionArgs;
	std::vector<std::vector<std::vector<std::string>>> actionArgArgs;
	int timeStamp;
	for(unsigned int i = 0; i < actionStrings.size(); i++)
	{
		//Empty strings are not valid
		if(actionStrings[i].length() == 0) return false;

		//Strip spaces away
		actionStrings[i] = sp::stripSpaces(actionStrings[i]);

		//Initial checkings
		if(!sp::startsWith(actionStrings[i],"occurs")) return false;
		int lp_count = sp::countOcc(actionStrings[i],"(");
		int rp_count = sp::countOcc(actionStrings[i],")");
		if(lp_count < 1 || rp_count < 1 || lp_count != rp_count) return false;

		//Extract the components inside 'occurs'
		int lp_index = actionStrings[i].find("(");
		int rp_index = actionStrings[i].rfind(")");
		std::string arg_str = sp::strBtwn(actionStrings[i],lp_index,rp_index);
		std::vector<std::string> args = sp::splitStr(arg_str,",");
		args = sp::correctCommaSplit(args);
		if(args.size() != 2) return false;

		//Check for a valid time stamp
		if(!sp::isInt(args[1])) return false;
		int t_stamp = std::stoi(args[1]);
		if(t_stamp < 0) return false;
		if(i == 0) timeStamp = t_stamp;
		else if(timeStamp != t_stamp) return false;

		//Check for a valid action
		int alp_count = sp::countOcc(args[0],"(");
		int arp_count = sp::countOcc(args[0],")");
		if(alp_count != arp_count) return false;

		//An action without arguments
		if(alp_count == 0)
		{
			actionNames.push_back(args[0]);
			actionArgs.push_back(std::vector<std::string>());
			actionArgArgs.push_back(std::vector<std::vector<std::string>>());
			continue;
		}

		int alp_index = args[0].find("(");
		int arp_index = args[0].rfind(")");
		if(alp_index > arp_index) return false;
		if(alp_index == 0) return false;

		//Extract the action name, its arguments & sub-arguments
		std::vector<std::string> temp_vec = sp::splitStr(sp::strBtwn(args[0],alp_index,arp_index),",");
		temp_vec = sp::correctCommaSplit(temp_vec);
		std::vector<std::string> t_args;
		std::vector<std::vector<std::string>> t_arg_args;
		for(unsigned int j = 0; j < temp_vec.size(); j++)
		{
			int llp = temp_vec[j].find("(");
			int rrp = temp_vec[j].rfind(")");
			int llc = sp::countOcc(temp_vec[j],"(");
			int rrc = sp::countOcc(temp_vec[j],")");

			// j-th argument has no arguments
			if((llp == -1) && (rrp == -1))
			{
				t_args.push_back(temp_vec[j]);
				t_arg_args.push_back(std::vector<std::string>());
			}
			// j-th argument has arguments
			else if(
			(llc == 1) 	&& // There is a single '('
			(rrc == 1) 	&& // There is a single ')'
			(llp < rrp) 	&& // '(' is before ')'
			(llp > 0) 	&& // There are characters before '('
			(rrp-llp > 1))	   // There are characters between '(' and ')'
			{
				std::vector<std::string> temptemp_vec;
				temptemp_vec = sp::splitStr(sp::strBtwn(temp_vec[j],llp,rrp),",");
				for(unsigned int k = 0; k < temptemp_vec.size(); k++)
				{
					// Empty sub-args are not accepted
					if(temptemp_vec[k] == "") return false;

					//Sub-args can't have parenthesis
					if(temptemp_vec[k].find("(") != -1) return false;
					if(temptemp_vec[k].find(")") != -1) return false;
				}

				t_args.push_back(temp_vec[j].substr(0,llp));
				t_arg_args.push_back(temptemp_vec);
			}
			else return false; // An invalid action-string
		}

		//Save the action name and its arguments
		actionNames.push_back(args[0].substr(0,alp_index));
		actionArgs.push_back(t_args);
		actionArgArgs.push_back(t_arg_args);
	}

	//Build the string that contains all of the individual actions
	std::string temp_wa("");
	temp_wa += "{";
	for(unsigned int i = 0; i < actionNames.size(); i++)
	{
		if(i > 0) temp_wa += ",";
		temp_wa += ("occurs("+actionNames[i]);
		if(actionArgs[i].size() > 0) temp_wa += "(";
		for(unsigned int j = 0; j < actionArgs[i].size(); j++)
		{
			if(j > 0) temp_wa += ",";
			temp_wa += actionArgs[i][j];
			if(actionArgArgs[i][j].size() > 0) temp_wa += "(";
			for(unsigned int k = 0; k < actionArgArgs[i][j].size(); k++)
			{
				if(k > 0) temp_wa += ",";
				temp_wa += actionArgArgs[i][j][k];
			}
			if(actionArgArgs[i][j].size() > 0) temp_wa += ")";
		}
		if(actionArgs[i].size() > 0) temp_wa += ")";

		temp_wa += (","+std::to_string(timeStamp)+")");
	}
	temp_wa += "}";

	WholeAction = temp_wa;
	ActionNames = actionNames;
	ActionArgs = actionArgs;
	ActionArgArgs = actionArgArgs;
	TimeStamp = timeStamp;

	return true;
}

sp::Action::Action():
WholeAction(""),
ActionNames(std::vector<std::string>()),
ActionArgs(std::vector<std::vector<std::string>>()),
ActionArgArgs(std::vector<std::vector<std::vector<std::string>>>()),
TimeStamp(-1)
{

}

sp::Action::Action(std::vector<std::string> const &actionStrings)
{
	Action();
	if(!exractActionsFromString(actionStrings)) TimeStamp = -1;
}

bool sp::Action::set(std::vector<std::string> const &actionStrings)
{
	return exractActionsFromString(actionStrings);
}

int sp::Action::step() const
{
	return TimeStamp;
}

std::string sp::Action::str() const
{
	return WholeAction;
}

bool sp::Action::shareA(Action const &a) const
{
	if(ActionNames.size() != a.ActionNames.size()) return false;

	std::vector<int> matched_actions;
	std::vector<int>::iterator it;
	for(unsigned int i = 0; i < ActionNames.size(); i++)
	{
		//String for a quick comparison
		std::string action_i(ActionNames[i]);
		for(unsigned int j = 0; j < ActionArgs[i].size(); j++)
		{
			action_i += ActionArgs[i][j];
			for(unsigned int k = 0; k < ActionArgArgs[i][j].size(); k++)
				action_i += ActionArgArgs[i][j][k];
		}

		for(unsigned int j = 0; j < a.ActionNames.size(); j++)
		{
			//Avoid checking action from 'a' that were already matched
			it = std::find(matched_actions.begin(),matched_actions.end(),j);
			if(it != matched_actions.end()) continue;

			//String for a quick comparison
			std::string action_j(a.ActionNames[j]);
			for(unsigned int k = 0; k < a.ActionArgs[j].size(); k++)
			{
				action_j += a.ActionArgs[j][k];
				for(unsigned int l = 0; l < a.ActionArgArgs[j][k].size(); l++)
					action_j += a.ActionArgArgs[j][k][l];
			}

			if(action_i != action_j) continue;

			//Save the index of the j-th matched action
			matched_actions.push_back(j);
			break;
		}
	}

	//All actions must be matched in order to return true
	return (ActionNames.size() == matched_actions.size());
}

void sp::Action::debug() const
{
	std::cout << "TimeStamp: " << TimeStamp << std::endl;
	std::cout << "Actions, arguments and sub-arguments: " << std::endl;
	for(unsigned int i = 0; i < ActionNames.size(); i++)
	{
		std::cout << ActionNames[i] << std::endl;
		for(unsigned int j = 0; j < ActionArgs[i].size(); j++)
		{
			std::cout << "\t" + ActionArgs[i][j] << std::endl;
			for(unsigned int k = 0; k < ActionArgArgs[i][j].size(); k++)
			{
				std::cout << "\t\t" + ActionArgArgs[i][j][k] << std::endl;
			}
		}
	}
}

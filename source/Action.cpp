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
		if(lp_count != 2 || rp_count != 2) return false;

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
		if(alp_count != 1 || arp_count != 1) return false;
		int alp_index = args[0].find("(");
		int arp_index = args[0].find(")");
		if(alp_index > arp_index) return false;
		if(alp_index == 0) return false;

		//Extract the action name and its arguments
		std::string an = args[0].substr(0,alp_index);
		std::vector<std::string> temp_args;
		temp_args = sp::splitStr(sp::strBtwn(args[0],alp_index,arp_index),",");
		temp_args = sp::correctCommaSplit(temp_args);

		//Save the action name and its arguments
		actionNames.push_back(an);
		actionArgs.push_back(temp_args);
	}

	ActionNames = actionNames;
	ActionArgs = actionArgs;
	TimeStamp = timeStamp;

	return true;
}

sp::Action::Action():
ActionNames(std::vector<std::string>()),
ActionArgs(std::vector<std::vector<std::string>>()),
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

bool sp::Action::shareA(Action const &a) const
{
	if(ActionNames.size() != a.ActionNames.size()) return false;

	std::vector<int> matched_actions;
	std::vector<int>::iterator it;
	for(unsigned int i = 0; i < ActionNames.size(); i++)
	{
		//String for a quick comparison
		std::string action_i(ActionNames[i]);
		for(unsigned int j = 0; j < ActionArgs[i].size(); j++) action_i += ActionArgs[i][j];

		for(unsigned int j = 0; j < a.ActionNames.size(); j++)
		{
			//Avoid checking action from 'a' that were already matched
			it = std::find(matched_actions.begin(),matched_actions.end(),j);
			if(it != matched_actions.end()) continue;

			//String for a quick comparison
			std::string action_j(a.ActionNames[j]);
			for(unsigned int k = 0; k < a.ActionArgs[j].size(); k++) action_j += a.ActionArgs[j][k];

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
	std::cout << "Actions and their Arguments: " << std::endl;
	for(unsigned int i = 0; i < ActionNames.size(); i++)
	{
		std::cout << ActionNames[i] << std::endl;
		for(unsigned int j = 0; j < ActionArgs[i].size(); j++)
		{
			std::cout << "\t" + ActionArgs[i][j] << std::endl;
		}
	}
}

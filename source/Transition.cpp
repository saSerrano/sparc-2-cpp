#include <Transition.hpp>
#include <PState.hpp>
#include <sparc2cpp.hpp>

bool sp::Transition::exractActionsFromString(std::vector<std::string> actionStrings)
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

sp::Transition::Transition():
ps0("{}"),
ps1("{}"),
IndexPs0(-1),
IndexPs1(-1),
TimeStamp(-1)
{

}

sp::Transition::Transition(std::vector<std::string> actionStrings, PState ps0, PState ps1)
{
	if(exractActionsFromString(actionStrings))
	{
		this->ps0 = ps0;
		this->ps1 = ps1;
	}
	else
	{
		this->ps0.set("{}");
		this->ps1.set("{}");
		TimeStamp = -1;
	}

	IndexPs0 = -1;
	IndexPs1 = -1;
}

void sp::Transition::debug() const
{
	std::cout << "IndexPs0: " << IndexPs0 << std::endl;
	std::cout << "IndexPs1: " << IndexPs1 << std::endl;
	std::cout << "P-STATE-0:";
	ps0.debug();
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
	std::cout << "P-STATE-1:";
	ps1.debug();
}

bool sp::Transition::set(std::vector<std::string> actionStrings, sp::PState ps0, sp::PState ps1)
{
	if(exractActionsFromString(actionStrings))
	{
		this->ps0 = ps0;
		this->ps1 = ps1;
		return true;
	}
	else return false;
}

bool sp::Transition::setStatesID(std::vector<sp::PState> const &setOfStates)
{
	if(setOfStates.size() == 0) return false;

	int index_s0 = -1;
	int index_s1 = -1;
	for(unsigned int i = 0; i < setOfStates.size(); i++)
	{
		if(index_s0 == -1 && ps0.equals(setOfStates[i])) index_s0 = i;

		if(index_s1 == -1 && ps1.equals(setOfStates[i])) index_s1 = i;

		if(index_s0 > -1 && index_s1 > -1) break;
	}

	if(index_s0 > -1 && index_s1 > -1)
	{
		IndexPs0 = index_s0;
		IndexPs1 = index_s1;
		return true;
	}
	else return false;
}

bool sp::Transition::shareS0(sp::Transition const &t) const
{
	return ps0.equals(t.ps0);
}

bool sp::Transition::shareS1(sp::Transition const &t) const
{
	return ps1.equals(t.ps1);
}

bool sp::Transition::shareAc(sp::Transition const &t) const
{
	if(ActionNames.size() != t.ActionNames.size()) return false;

	std::vector<int> matched_actions;
	std::vector<int>::iterator it;
	for(unsigned int i = 0; i < ActionNames.size(); i++)
	{
		//String for a quick comparison
		std::string action_i(ActionNames[i]);
		for(unsigned int j = 0; j < ActionArgs[i].size(); j++) action_i += ActionArgs[i][j];

		for(unsigned int j = 0; j < t.ActionNames.size(); j++)
		{
			//Avoid checking action from 't' that were already matched
			it = std::find(matched_actions.begin(),matched_actions.end(),j);
			if(it != matched_actions.end()) continue;

			//String for a quick comparison
			std::string action_j(t.ActionNames[j]);
			for(unsigned int k = 0; k < t.ActionArgs[j].size(); k++) action_j += t.ActionArgs[j][k];

			if(action_i != action_j) continue;

			//Save the index of the j-th matched action
			matched_actions.push_back(j);
			break;
		}
	}

	//All actions must be matched in order to return true
	return (ActionNames.size() == matched_actions.size());
}

bool sp::Transition::beforeThan(sp::Transition const &t) const
{
	return (TimeStamp < t.TimeStamp);
}

bool sp::Transition::afterThan(sp::Transition const &t) const
{
	return (TimeStamp > t.TimeStamp);
}

bool sp::Transition::atSameTime(sp::Transition const &t) const
{
	return (TimeStamp == t.TimeStamp);
}


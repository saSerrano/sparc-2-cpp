#include <Transition.hpp>
#include <PState.hpp>
#include <Action.hpp>
#include <sparc2cpp.hpp>

sp::Transition::Transition():
ps0("{}"),
ps1("{}"),
IndexPs0(-1),
IndexPs1(-1),
SetofActions(),
IndexAction(-1)
{

}

sp::Transition::Transition(sp::Action SetofActions, sp::PState ps0, sp::PState ps1)
{
	this->SetofActions = SetofActions;
	this->ps0 = ps0;
	this->ps1 = ps1;
	IndexPs0 = -1;
	IndexPs1 = -1;
	IndexAction = -1;
}

void sp::Transition::debug() const
{
	std::cout << "IndexPs0: " << IndexPs0 << std::endl;
	std::cout << "IndexPs1: " << IndexPs1 << std::endl;
	std::cout << "IndexAction: " << IndexAction << std::endl;
	std::cout << "P-STATE-0: " + ps0.str() << std::endl;
	std::cout << "Action:" + SetofActions.str() << std::endl;
	std::cout << "P-STATE-1: " + ps1.str() << std::endl;
}

void sp::Transition::set(sp::Action SetofActions, sp::PState ps0, sp::PState ps1)
{
	this->SetofActions = SetofActions;
	this->ps0 = ps0;
	this->ps1 = ps1;
	IndexPs0 = -1;
	IndexPs1 = -1;
	IndexAction = -1;
}

bool sp::Transition::setStatesID(std::vector<sp::PState> const &setOfStates)
{
	if(setOfStates.size() == 0) return false;

	int index_s0 = -1;
	int index_s1 = -1;
	for(unsigned int i = 0; i < setOfStates.size(); i++)
	{
		if(index_s0 == -1 && ps0.equals(setOfStates[i])) index_s0 = i;

		if(index_s1 == -1 && ps1.equalsExcLastArg(setOfStates[i])) index_s1 = i;

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
	return (SetofActions.shareA(t.SetofActions));
}

bool sp::Transition::beforeThan(sp::Transition const &t) const
{
	return (SetofActions.step() < t.SetofActions.step());
}

bool sp::Transition::afterThan(sp::Transition const &t) const
{
	return (SetofActions.step() > t.SetofActions.step());
}

bool sp::Transition::atSameTime(sp::Transition const &t) const
{
	return (SetofActions.step() == t.SetofActions.step());
}

bool sp::Transition::setActionID(std::vector<sp::Action> const &listOfActions)
{
	bool foundInVector = false;

	for(unsigned int i = 0; i < listOfActions.size(); i++)
	{
		if(SetofActions.shareA(listOfActions[i]))
		{
			IndexAction = i;
			foundInVector = true;
			break;
		}
	}

	return foundInVector;
}

int sp::Transition::iS0() const
{
	return IndexPs0;
}

int sp::Transition::iS1() const
{
	return IndexPs1;
}

int sp::Transition::iA() const
{
	return IndexAction;
}

void sp::Transition::index(std::vector<sp::Transition> &T, std::vector<sp::PState> const &S, std::vector<sp::Action> const &A)
{
	for(unsigned int i = 0; i < T.size(); i++)
	{
		T[i].setStatesID(S);
		T[i].setActionID(A);
	}
}

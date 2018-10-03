#include <iostream>
#include <sparc2cpp.hpp>
#include <DomainSort.hpp>
#include <PState.hpp>
#include <Transition.hpp>
#include <Action.hpp>

using namespace std;
using namespace sp;


int main(int argc, char** argv)
{
	//Load the Domain description via a SPARC file
	string sparcFile("/home/zero/Dropbox/Tesis/SPARC-Learn/2pomdp/blocks-orig.sp");
	DomainSort ds;
	ds.loadFile(sparcFile);

	//Generate all the states, in this case only inertial fluents are considered
	vector<string> defs;
	defs.push_back("val(#inertial_f,true,X)");
	vector<vector<PState>> all_states = ds.generateStates("#inertial_f",defs);
	vector<PState> i_states = all_states[0];

	//Generate the list of actions
	vector<Action> i_actions = ds.generateActions("#action");

	//Generate the transitions the result from the non-deterministic excution
	//of actions in i_actions over the states in i_states
	cout << "# actions: " << i_actions.size() << endl;
	vector<Transition> i_trans = ds.generateTransitions(i_states,i_actions,"#inertial_f");
	cout << "# actions: " << i_actions.size() << endl;
	cout << "# transitions: " << i_trans.size() << endl;

	return 0;
}




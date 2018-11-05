#include <iostream>
#include <sparc2cpp.hpp>
#include <DomainSort.hpp>
#include <PState.hpp>
#include <Transition.hpp>
#include <Action.hpp>
#include <Pomdp.hpp>

using namespace std;
using namespace sp;


int main(int argc, char** argv)
{
	//Load the Domain description via a SPARC file
	string sparcFile("/home/zero/Dropbox/Tesis/SPARC-Learn/2pomdp/full-rob-navig-orig.sp");
	DomainSort ds;
	ds.loadFile(sparcFile);

	//Generate all the states, in this case only inertial fluents are considered
	vector<string> defs;
	defs.push_back("val(#ink_f,true,X)");
	defs.push_back("val(#ik_f,true,X)");
	vector<vector<PState>> all_states = ds.generateSZ("#non_static_f","#ik_f",defs);
	vector<PState> S = all_states[0];
	vector<PState> Z = all_states[1];

	//Generate the list of actions
	vector<Action> NKA = ds.generateActions("#nk_a",S);
	vector<Action> KA = ds.generateActions("#k_a",S);

	//Generate the transitions the result from the non-deterministic excution
	//of actions in i_actions over the states in i_states
	vector<Transition> T_trans = ds.generateTransitions(S,NKA,"#ink_f");
	vector<Transition> O_trans = ds.generateTransitions(S,KA,"#ik_f");

	//Create the execution-terminating actions
	vector<string> ttt;
	ttt.push_back("occurs(finish,0)");
	Action ta(ttt);
	vector<Action> termA;
	termA.push_back(ta);

	//Concatenate T-transition and O-transition vectors
	vector<Transition> TO_trans;
	TO_trans.insert(TO_trans.end(),T_trans.begin(),T_trans.end());
	TO_trans.insert(TO_trans.end(),O_trans.begin(),O_trans.end());

	Pomdp p;
	p.saveTD(S,Z,NKA,KA,termA,TO_trans,"full-rob-navig.td");
	p.generatePomdp(S,Z,NKA,KA,termA,TO_trans,ds,"full-rob-navig.pomdp","stats.txt");

	return 0;
}




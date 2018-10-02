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
//PState ps0,ps1;
//
//vector<string> vec1;
//vec1.push_back("occurs(put(b3,ty),6)");
//vec1.push_back("occurs(put(b1,t),6)");
//vector<string> vec2;
//vec2.push_back("occurs(put(b1,t),6)");
//vec2.push_back("occurs(put(b3,ty),6)");
//vector<string> vec3;
//vec3.push_back("occurs(put(b9,t),6)");
//vec3.push_back("occurs(put(b7,ty),6)");
//vector<string> vec4;
//vec4.push_back("occurs(put(b9,t),45)");
//vec4.push_back("occurs(put(b7,ty),45)");
//
//Action A1(vec1);
//Action A2(vec2);
//Action A3(vec3);
//Action A4(vec4);
//
//vector<Action> list_of_actions;
//list_of_actions.push_back(A1); 
//list_of_actions.push_back(A2); 
//list_of_actions.push_back(A3); 
//
//Transition T(A4,ps0,ps1);
//T.debug();
//T.listActionID(list_of_actions);
//T.debug();


//
//Transition t1,t2;
//t1.set(a1,s1,s2);
//t2.set(vec1,s1,s2);
//
//cout << t1.beforeThan(t2) << endl;
//cout << t2.beforeThan(t1) << endl;
//cout << t1.atSameTime(t1) << endl;

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

cout << "# actions: " << i_actions.size() << endl;
	//Generate the transitions the result from the non-deterministic excution
	//of actions in i_actions over the states in i_states
	vector<Transition> i_trans = ds.generateTransitions(i_states,i_actions,"#inertial_f");
cout << "# actions: " << i_actions.size() << endl;
cout << "# transitions: " << i_trans.size() << endl;

//vector<PState> setS0;
//PState ps1("{loc(r1,c5),loc(r2,c6)}");
//PState ps2("{loc(door,c7),loc(toilet,c3)}");
//setS0.push_back(ps1);
//setS0.push_back(ps2);
//
//vector<string> vec;
//vector<Action> setA;
//vec.push_back("occurs(put(b5,t),0)");
//Action a1(vec);
//setA.push_back(a1);
//vec.push_back("occurs(put(b7,b5),0)");
//Action a2(vec);
//setA.push_back(a2);
//
//ds.generateTransitions(setS0,setA,"#defined_f");

//	vector<Action> listA = ds.generateActions("#action");
//	for(unsigned int i = 0; i < listA.size(); i++)
//	{
//		listA[i].debug();
//	}

//	ds.debug();
//	PState s1("{val(on(b1,b0),true,7)}");
//	vector<PState> vec;
//	vec.push_back(s1);
//	ds.generateTransitions(vec,"#action");

//	vector<string> defs;
//	defs.push_back("val(#inertial_f(b0,X),X,X)");
//	defs.push_back("val(#inertial_f(b1,X),X,X)");
//	vector<vector<PState>> states = ds.generateStates("#inertial_f",defs);
//	for(unsigned int i = 0; i < states.size(); i++)
//	{
//		cout << "------------------------------------------------------------" << endl;
//		if(i < states.size()-1) cout << "Sub-answer-set #" << i << endl;
//		else cout << "Whole answer-set" << endl;
//		for(unsigned int j = 0; j < states[i].size(); j++) states[i][j].debug();
//	}

//Checking the substate function
//PState ps1("{val(on(b0,t),true,1),val(on(b1,t),true,1)}");
//vector<string> defs;
//defs.push_back("val(#inertial_f(b0,X),X,X)");
//PState ps2 = subPstate(ds,ps1,defs);
//ps2.debug();

//	string ps_str("{val(on(b0,t),true,0),val(on(b1,t),true,0),val(on(b2,t),true,0),val(above(b0,t),true,0),val(above(b2,t),true,0),val(above(b3,t),true,0)}");
//	PState ps(ps_str);
//
//	vector<string> defs;
//	if(argc == 2) defs.push_back(string(argv[1]));
//	//defs.push_back("val(#defined_f(X,X),X,X)");
//	//defs.push_back("val");
//
//	//Get a subset of the literals from 'ps'
//	PState sub_ps = subPstate(ds,ps,defs);
//	cout << "Sub-pstate: " + sub_ps.str() << endl;
	return 0;
}




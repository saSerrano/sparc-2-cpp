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


	string sparcFile("/home/zero/Dropbox/Tesis/SPARC-Learn/2pomdp/blocks-orig.sp");
	DomainSort ds;
	ds.loadFile(sparcFile);
	vector<Action> listA = ds.generateActions("#action");
	for(unsigned int i = 0; i < listA.size(); i++)
	{
		listA[i].debug();
	}
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




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
	string sparcFile("/home/zero/Dropbox/Tesis/SPARC-Learn/2pomdp/rob-navig-orig.sp");
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
	vector<Action> NKA = ds.generateActions("#nk_a",vector<PState>());
	vector<Action> KA = ds.generateActions("#k_a",S);

cout << "STATES" << endl;
for(unsigned int i = 0; i < S.size(); i++) cout << S[i].str() << endl;

cout << "OBSERVATIONS" << endl;
for(unsigned int i = 0; i < Z.size(); i++) cout << Z[i].str() << endl;

cout << "NK - ACTIONS" << endl;
for(unsigned int i = 0; i < NKA.size(); i++) cout << NKA[i].str() << endl;

cout << "K - ACTIONS" << endl;
for(unsigned int i = 0; i < KA.size(); i++) cout << KA[i].str() << endl;

	//Generate the transitions the result from the non-deterministic excution
	//of actions in i_actions over the states in i_states
//	vector<Transition> i_trans = ds.generateTransitions(S,i_actions,"#inertial_f");
//
//Debug
//cout << "#S:" << S.size() << endl;
//cout << "#A:" << i_actions.size() << endl;
//cout << "#T:" << i_trans.size() << endl;
//cout << "--------------------------------------" << endl;
//Pomdp p;
//vector<PState> Z;
//vector<Action> kA;
//vector<Action> termA;
//p.generatePomdp(i_states,i_actions,kA,termA,Z,i_trans,ds,"/home/zero/Desktop/pomdpfilename.txt");

////Temporal vector
//vector<string> a_vec;
//
////Debugging Pomdp class
//vector<PState> ss;
//vector<Action> nka;
//vector<Action> ka;
//vector<PState> zz;
//vector<Transition> tt;
//DomainSort ds;
//string pf("/home/zero/Desktop/pomdpfilename.txt");
//
////S
//ss.push_back(PState("{val(on(b0,b1),true,1)}"));
//ss.push_back(PState("{val(on(b1,table),true,2)}"));
//ss.push_back(PState("{val(on(b0,b1),true,3),val(on(b2,b0),true,3)}"));
//
////nkA
//a_vec.push_back("occurs(put(b0,table),0)");
//a_vec.push_back("occurs(put(b1,table),0)");
//nka.push_back(a_vec);
//a_vec.clear();
//a_vec.push_back("occurs(put(b2,b0),1)");
//nka.push_back(a_vec);
//
////kA
//a_vec.clear();
//a_vec.push_back("occurs(sense_loc(b1),0)");
//ka.push_back(a_vec);
//a_vec.clear();
//a_vec.push_back("occurs(sense_loc(b2),1)");
//ka.push_back(a_vec);
//
////Z
//zz.push_back(PState("{val(seen(b0,b1),true,1)}"));
//zz.push_back(PState("{val(seen(b1,table),true,2)}"));
//
//Pomdp p;
//p.generatePomdp(ss,nka,ka,zz,tt,ds,pf);

	return 0;
}




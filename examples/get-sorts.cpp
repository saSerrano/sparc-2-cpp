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
Pomdp pp;
vector<PState> ss;
vector<PState> zz;
vector<Action> nka;
vector<Action> ka;
vector<Action> terma;
vector<Transition> tt;
pp.loadTD(ss,zz,nka,ka,terma,tt,"td.txt");
pp.saveTD(ss,zz,nka,ka,terma,tt,"td2.txt");

return 0;
//---------------------------------------------------------------------------------

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
	vector<Action> NKA = ds.generateActions("#nk_a",S);
	vector<Action> KA = ds.generateActions("#k_a",S);

//cout << "STATES" << endl;
//for(unsigned int i = 0; i < S.size(); i++) cout << S[i].str() << endl;
//
//cout << "OBSERVATIONS" << endl;
//for(unsigned int i = 0; i < Z.size(); i++) cout << Z[i].str() << endl;
//
//cout << "NK - ACTIONS" << endl;
//for(unsigned int i = 0; i < NKA.size(); i++) cout << NKA[i].str() << endl;
//
//cout << "K - ACTIONS" << endl;
//for(unsigned int i = 0; i < KA.size(); i++) cout << KA[i].str() << endl;

	//Generate the transitions the result from the non-deterministic excution
	//of actions in i_actions over the states in i_states
	vector<Transition> T_trans = ds.generateTransitions(S,NKA,"#ink_f");
	vector<Transition> O_trans = ds.generateTransitions(S,KA,"#ik_f");

//cout << "############################################################" << endl;
//cout << "T - TRANSITIONS" << endl;
//for(unsigned int i = 0; i < T_trans.size(); i++)
//{
//	T_trans[i].debug();
//	cout << "------------------------------------------------" << endl;
//}
//cout << "############################################################" << endl;
//cout << "O - TRANSITIONS" << endl;
//for(unsigned int i = 0; i < O_trans.size(); i++)
//{
//	O_trans[i].debug();
//	cout << "------------------------------------------------" << endl;
//}

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
	p.generatePomdp(S,Z,NKA,KA,termA,TO_trans,ds,"model.pomdp","stats.txt");

	return 0;
}




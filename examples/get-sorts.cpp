#include <iostream>
#include <sparc2cpp.hpp>
#include <DomainSort.hpp>
#include <PState.hpp>

using namespace std;
using namespace sp;


int main(int argc, char** argv)
{


	string sparcFile("/home/zero/Dropbox/Tesis/SPARC-Learn/2pomdp/blocks-orig.sp");
	DomainSort ds;
	ds.loadFile(sparcFile);

vector<string> defs;
defs.push_back("val(#inertial_f(b0,X),X,X)");
defs.push_back("val(#inertial_f(b1,X),X,X)");
vector<vector<PState>> states = ds.generateStates("#inertial_f",defs);
for(unsigned int i = 0; i < states.size(); i++)
{
cout << "------------------------------------------------------------" << endl;
if(i < states.size()-1) cout << "Sub-answer-set #" << i << endl;
else cout << "Whole answer-set" << endl;
for(unsigned int j = 0; j < states[i].size(); j++) states[i][j].debug();
}

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

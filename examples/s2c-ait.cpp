#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <thread>

#include <AIToolbox/POMDP/Model.hpp>
#include <AIToolbox/MDP/Model.hpp>

#include <AIToolbox/POMDP/Algorithms/IncrementalPruning.hpp>
#include <AIToolbox/POMDP/Policies/Policy.hpp>

#include <sparc2cpp.hpp>
#include <DomainSort.hpp>
#include <PState.hpp>
#include <Transition.hpp>
#include <Action.hpp>

using namespace std;
using namespace sp;

int main(int argc, char **argv)
{
	// Actions are: 0-listen, 1-open-left, 2-open-right
	size_t S = 2, A = 3, O = 2;

	AIToolbox::POMDP::Model<AIToolbox::MDP::Model> model(O, S, A);

	AIToolbox::Table3D transitions(boost::extents[S][A][S]);
	AIToolbox::Table3D rewards(boost::extents[S][A][S]);
	AIToolbox::Table3D observations(boost::extents[S][A][O]);

	std::cout << "The AI-Toolbox POMDP is running :)" << std::endl;

	//Create a SPARC-2-CPP DomainSort object
	DomainSort ds;

	return 0;
}

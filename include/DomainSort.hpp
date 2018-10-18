/**
 * \class DomainSort
 * 
 * \brief This class is for extracting, representing and manipulating the sorts of a SPARC file.
 * 
 * \author $Author: Sergio A. Serrano$
 * 
 * \date $Date: 16/09/18$
 * 
 * Contact: sserrano@inaoep.mx
 */

#include <sparc2cpp.hpp>
#include <Pomdp.hpp>

#include <string>
#include <vector>

/**
 * This constant is a path to the location of the SPARC-to-ASP translator is. In order for this code to work properly, you should modify this constant according to the location of the SPARC translator in your computer.
*/
#define PATH_TO_SPARC "/home/zero/SPARC/sparc.jar"

#ifndef DOMAIN_SORT
#define DOMAIN_SORT

namespace sp
{
	class PState;
	class Transition;
	class Action;
	class Pomdp;
	class DomainSort
	{
		private:

			bool FileIsLoaded;
			std::string SourceFile;
			std::vector<std::string> Sorts;
			std::vector<std::vector<std::string>> SortElements;
			std::vector<std::vector<std::vector<std::string>>> SortArguments;
			std::vector<std::vector<int>> SubsortElemIndexes;
			std::vector<std::vector<std::vector<int>>> SubsortArgsIndexes;
			std::vector<std::string> Consts;
			std::vector<std::string> ConstVals;

			std::vector<std::string> CausalHeadFluents;
			std::vector<std::vector<std::string>> CausalHeadFluentsArgs;
			std::vector<std::vector<std::string>> CausalHeadOthers;
			std::vector<std::string> CausalBodyActions;
			std::vector<std::vector<std::string>> CausalBodyActionsArgs;
			std::vector<std::vector<std::string>> CausalBodyOthers;
			std::vector<int> CausalLawStartLines;
			std::vector<int> CausalLawLengths;

			/**
			 * \brief This method verifies if SourceFile is a valid SPARC file to be loaded. Once this verification has been performed, it extracts the sorts and the sub-sort based hierarchy. This method does not make a full logic and sintactic revision to the source file, it assumes that the source file has been executed. Also, the rules that were written based on a notation of val(fluent,boolean,step) and occurs(action(arg0,..,argn),step), that correspond to actions' causal laws are extracted.
			*/
			void verifyAndLoad();

			friend bool recursiveCheck(DomainSort const &ds, std::string const &input, int sortToCheck);

			friend bool checkDef(DomainSort const &ds, std::string const &def, std::string const &input);

			friend PState subPstate(DomainSort const &ds, PState const &ps, std::vector<std::string> const &defs);

			friend bool Pomdp::generatePomdp(std::vector<PState> &S, 
						      std::vector<Action> const &nkA, 
						      std::vector<Action> const &kA, 
						      std::vector<Action> const &termA, 
						      std::vector<PState> &Z, 
						      std::vector<Transition> const &T, 
						      DomainSort const &ds, 
						      std::string const &pomdpFile);

		public:

			/**
			 * \brief Default constructor.
			*/
			DomainSort();

			/**
			 * \brief Constructor that is initialized with a SPARC source file.
			 * \param SourceFile SPARC source file to be loaded.
			*/
			DomainSort(std::string SourceFile);

			/**
			 * \brief This method is for loading a SPARC source file.
			 * \param SourceFile SPARC source file to be loaded.
			 * \return Returns true if a source file has been succesfully loaded.
			*/
			bool loadFile(std::string SourceFile);

			/**
			 * \brief This method is for checking if a SPARC file has been succesfully loaded.
			 * \return Returns true if a source file has been succesfully loaded.
			*/
			bool fileIsLoaded() const;

			/**
			 * \brief This method is to get a string that corresponds to the path to a source file that has been succesfully loaded.
			 * \return Returns SourceFile.
			*/
			std::string getFile() const;

			/**
			 * \brief The method is for displaying the structure of sorts extracted from SourceFile.
			*/
			void sortDisplay() const;

			/**
			 * \brief The method is for displaying the structure of the sub-sort relation among the sorts extracted from SourceFile.
			*/
			void hierDisplay() const;

			/**
			 * \brief The method is for displaying the structure of the causal laws extracted from the source file.
			*/
			void causalDisplay() const;

			/**
			 * \brief This method is for generating a set of states from a collection of literals, that is, all the possible combinations of values for these literals, such that these combinations of values are consistent with the constraints that describe the domain encoded in the SPARC file.

			 * \param rootGenSort This string is a sort definition of the form #root_sort, its where the method will start to look for literals, that is, literals that are elements of #root_sort or elements of a sort that is sub-sort of #root_sort.

			 * \param subStateDefs contains a list of definitions. For each definition, this method will extract a sub-state (that satisfies the definition) from each state generated. That is why this method's output has the form of a vector of vectors of states.

			 * \return The returned value is a vector of vectors of states. The returned vector holds one vector of states for each definition in 'subStateDefs'. It also holds an extra vector of states that corresponds to the original vector of states.
			*/
			std::vector<std::vector<PState>> generateStates(std::string const &rootGenSort, std::vector<std::string> const &subStateDefs) const;

			/**
			 * \brief In a similar manner to the 'generateStates', hhis method is for generating a set of states from a collection of literals, with the difference that uses the 'rootZSort' root sort to determine which fluents (those considered to be observations within the SPARC file) will be restricted to a length of 1 literal when creating its choice-rule.

			 * \param rootGenSort This string is a sort definition of the form #root_sort, its where the method will start to look for literals, that is, literals that are elements of #root_sort or elements of a sort that is sub-sort of #root_sort.

			 * \param rootZSort This string is a sort definition of the form #root_sort, its where the method will start to look for the literals that will be considered as observation-literals.

			 * \param subStateDefs contains a list of definitions. For each definition, this method will extract a sub-state (that satisfies the definition) from each state generated. That is why this method's output has the form of a vector of vectors of states.

			 * \return The returned value is a vector of vectors of states. The returned vector holds one vector of states for each definition in 'subStateDefs'. It also holds an extra vector of states that corresponds to the original vector of states.
			*/
			std::vector<std::vector<PState>> generateSZ(std::string const &rootGenSort, std::string const &rootZSort, std::vector<std::string> const &subStateDefs) const;

			/**
			 * \brief This method generates a vector of instances of actions, from those defined and in the sort structure loaded from the SPARC file. This method will start looking for actions from the sort rootGenSort and will go downwards form there. Also, in order for an action to be instantiated, it must have extactly 1 causal law that describes its effects. As future improvements, we'd like to extend this method to support several causal laws for the same action.
			 * \param rootGenSort The root sort from which this method will start looking for actions downwards. This string must begin with '#', followed by the name of the root sort.
			 * \return Returns a vector of Action objects, one for each generated action. All these actions have for time-stamp 0.
			*/
			std::vector<Action> generateActions(std::string const &rootGenSort, std::vector<PState> const &S) const;

			/**
			 * \brief This method generates the set of possible transitions that: start from one of the states in setS0, execute one of the actions in setA, and reach one of the states in setS0, i.e., a transition diagram. This method assumes that the SPARC file loaded has, for each of its actions, a defined fluent of the form 'actionname_pos_outcome(#arg1,#arg2,...)', where the list of args of this def. fluent must match those of the inertial fluent the action modifies. Also, the first argument of the inertial and defined fluent, must be of the same sort than the action. Thus, this method does not verifies if the the correspondant defined fluent for each action actually exists, it is responsability of the SPARC coder to provide definition of these defined fluents.
			 * \param setS0 The set of states the transition diagram has.
			 * \param setA The set of actions the transition diagram has. This vector of actions could be modified by this method, by removing thos actions that do not reach a state with any of the starting states.
			 * \param rootDisplaySort This sort indicates which fluents should be considered as part of the reached states, that is, if non of the literals generated by executing an action is part of the rootDisplaySort sort or to any of its sub-sorts, we say the action did not reach a state. The sort should start with a '#'.
			 * \return Returns a vector of all the possible transitions given the sets of states and actions. The transitions' states and action are already indexed with respect to the vectors setS0 and the final version of setA (The one without the useless actions).
			*/
			std::vector<Transition> generateTransitions(std::vector<PState> const &setS0, std::vector<Action> &setA, std::string const &rootDisplaySort);
	};

}

#endif

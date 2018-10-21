/**
 * \class PState
 * 
 * \brief This class is for representing, manipulating and comparing states that are defined by a collection of literals extracted from a SPARC file.
 * 
 * \author $Author: Sergio A. Serrano$
 * 
 * \date $Date: 16/09/18$
 * 
 * Contact: sserrano@inaoep.mx
 */
#include <sparc2cpp.hpp>
#include <DomainSort.hpp>
#include <Pomdp.hpp>

#include <string>
#include <vector>

#ifndef P_STATE
#define P_STATE

namespace sp
{
	class DomainSort;
	class Transition;
	class Action;
	class Pomdp;
	class PState
	{
		private:

			int PStateLength;
			std::string PStateString;
			std::vector<std::string> LitNames;
			std::vector<std::vector<std::string>> LitArgs;
			std::vector<std::vector<std::vector<std::string>>> LitArgArgs;
			std::vector<std::vector<std::vector<std::vector<std::string>>>> LitArgArgArgs;

			/**
			 * \brief This method is for obtaining the structure of the literals that constitute a state, from a string. If the method succeeds in extracting the literals' structure, these will be stored in the class' members 'LitNames', 'LitArgs' and 'LitArgArgs'. A valid literal is of the form 'lit_name(arg1,arg2(subarg1,subarg2),arg3)'. It is important to know that the PState class supports only literals that have at most subargs, that is, a literal's structure of the form 'lit_name(arg1(subarg1(subsubarg1)))' will not be extracted properly.
			 * \param psString Input string that should contain a set (enclosed by brackets '{' and '}') of valid literals, separated by commas.
			 * \return Returns true if a set of valid literals was successfully extracted, otherwise, false.
			*/
			bool extractPStateFromString(std::string psString);

			/**
			 * \brief This method displays the structure of the set of literals that constitute the state. These structures are illustrated by using tab separations. 
			*/
			void pstateDisplay() const;

			friend PState subPstate(DomainSort const &ds, PState const &ps, std::vector<std::string> const &defs);

			friend std::vector<Transition> DomainSort::generateTransitions(std::vector<PState> const &setS0, std::vector<Action> &setA, std::string const &rootDisplaySort);

			friend bool Pomdp::generatePomdp(std::vector<PState> &S, 
						      std::vector<PState> &Z, 
						      std::vector<Action> const &nkA, 
						      std::vector<Action> const &kA, 
						      std::vector<Action> const &termA, 
						      std::vector<Transition> &T, 
						      DomainSort const &ds, 
						      std::string const &pomdpFile,
						      std::string const &statFile);

		public:

			/**
			 * \brief Default constructor.
			*/
			PState();

			/**
			 * \brief Constructor that is initialized by a string that should correspond to a set of valid literals, separated by commas and enclosed by brackets.
			 * \param psString Input string containing a set of valid literals. 
			*/
			PState(std::string psString);

			/**
			 * \brief This method assigns a new set of literals to define the state.
			 * \param psString Input string containing a set of valid literals. 
			 * \return Returns true if a set of valid literals was successfully extracted, otherwise, false.
			*/
			bool set(std::string psString);

			/**
			 * \brief This method is for getting the amount of literals a state has.
			 * \return Returns the value of PStateLength.
			*/
			int len() const;

			/**
			 * \brief This method is for getting the string that corresponds to the set of literals that constitute the state.
			 * \return Returns the value of PStateString.
			*/
			std::string str() const;

			/**
			 * \brief This method is mainly used to display information about the literals, which is useful for debugging.
			*/
			void debug() const;
			
			/**
			 * \brief This method verifies if the instance state from which is invoked contains a set of literals that the input state ps2 has, however, the literals must differ in their last argument to find their match. That is, in order for a pair of literals to match, they must have the same name and have the same arguments, except by the last one. This method was specially developed to match states that differ only in their time-stamp, which is usually encoded as the last argument. 
			 * \param ps2 The other state to which the instance will compare to.
			 * \return Returns true if both states have the same set of literals that differ only in their last argument, otherwise, false.
			*/
			bool equalsExcLastArg(PState const &ps2) const;
			
			/**
			 * \brief This method verifies if the instance state from which is invoked contains the exact same set of literals that the input state ps2 has, which in turn would mean that these states are the same. This method is robust against different ordering of the literals.
			 * \param ps2 The other state to which the instance will compare to.
			 * \return Returns true if both states have the same set of literals, otherwise, false.
			*/
			bool equals(PState const &ps2) const;
			
			/**
			 * \brief This method verifies if the instance state from which is invoked contains all of the literals ps2 has, that is, verifies if the set of literals in ps2 is a subset of the literals of this state instance.
			 * \param ps2 The other state to which the instance will compare to.
			 * \return Returns true if this intances has all the literals in ps2, otherwise, false.
			*/
			bool contains(PState const &ps2) const;
	};
}

#endif

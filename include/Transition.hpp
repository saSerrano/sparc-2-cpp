/**
 * \class Transition
 * 
 * \brief This class is for representing and manipulating transitions between states and actions extracted from a SPARC file. A Transition is defined by its starting state, ending state, a set of actions that occur concurrently, and the time stamp of when the actions were performed. The states are represented by an object of type PState. 
 * 
 * \author $Author: Sergio A. Serrano$
 * 
 * \date $Date: 16/09/18$
 * 
 * Contact: sserrano@inaoep.mx
 */

#include <sparc2cpp.hpp>
#include <PState.hpp>

#include <string>
#include <vector>

#ifndef TRANSITION
#define TRANSITION

namespace sp
{
	class PState;
	class Transition
	{
		private:
			PState ps0;
			PState ps1;
			int IndexPs0;
			int IndexPs1;
			std::vector<std::string> ActionNames;
			std::vector<std::vector<std::string>> ActionArgs;
			int TimeStamp;

			/**
			 * \brief This method takes for input a vector of strings and extracts from each string the structure of an action, where these actions are defined by the action's name, the action's arguments and its time stamp. In order for a string to be successfully converted into a structure, it must have the following form: occurs(action_name(arg1,arg2,...),time_stamp), where time stamp has to be a positive integer, and all the action strings in actionStrings must have the same time stamp.
			 * \param actionStrings Vector of strings, where each string corresponds to a action whose structure will be extracted.
			 * \return Returns true if all of the strings in actionStrings were successfully converted into a valid action, otherwise, false.
			*/
			bool exractActionsFromString(std::vector<std::string> actionStrings);

		public:

			/**
			 * \brief Default constructor.
			*/
			Transition();

			/**
			 * \brief Constructor that initializes the Transition object by a vector of strings that will be converted into actions, a starting state and an ending state.
			 * \param actionStrings Vector of strings, where each string corresponds to a action whose structure will be extracted.
			 * \param ps0 The transition's starting state.
			 * \param ps1 The transition's ending state.
			*/
			Transition(std::vector<std::string> actionStrings, PState ps0, PState ps1);

			/**
			 * \brief This method is mainly used to display information about the transition's actions and states, which is useful for debugging. 
			*/
			void debug() const;

			/**
			 * \brief This method assigns to the Transition object a new set of actions by a vector of strings that will be converted into actions, also a new starting state and an ending state.
			 * \param actionStrings Vector of strings, where each string corresponds to a action whose structure will be extracted.
			 * \param ps0 The transition's starting state.
			 * \param ps1 The transition's ending state.
			 * \return Returns true if all of the strings in actionStrings were successfully converted into a valid action, otherwise, false.
			*/
			bool set(std::vector<std::string> actionStrings, PState ps0, PState ps1);

			/**
			 * \brief This method assigns an ID number to its starting and ending states, with respect to the position these states have within the vector setOfStates. Only if both starting and ending states appear in setOfStates, the IDs will be assigned.
			 * \param setOfStates Vector of PState objects from which the starting and ending states IDs will be ectracted.
			 * \return Returns true if both starting and ending states were found within setOfStates, otherwise, false.
			*/
			bool setStatesID(std::vector<PState> const &setOfStates);

			/**
			 * \brief This method is for checking if the Transition object from which it is invoked has the same starting state as the input transition t.
			 * \param t Transition object which this instance is being compared to.
			 * \return Returns true if both Transition objects have the same starting state.
			*/
			bool shareS0(Transition const &t) const;

			/**
			 * \brief This method is for checking if the Transition object from which it is invoked has the same ending state as the input transition t.
			 * \param t Transition object which this instance is being compared to.
			 * \return Returns true if both Transition objects have the same ending state.
			*/
			bool shareS1(Transition const &t) const;

			/**
			 * \brief This method is for checking if the Transition object from which it is invoked has the same set of actions as the input transition t, regardless of their time stamps.
			 * \param t Transition object which this instance is being compared to.
			 * \return Returns true if both Transition objects have the same set of actions.
			*/
			bool shareAc(Transition const &t) const;

			/**
			 * \brief This method is for checking if the Transition object's set of actions, from which it is invoked, occured before than those in the input transition t.
			 * \param t Transition object which this instance is being compared to.
			 * \return Returns true if this instance's TimeStamp value is less than t's TimeStamp.
			*/
			bool beforeThan(Transition const &t) const;

			/**
			 * \brief This method is for checking if the Transition object's set of actions, from which it is invoked, occured after than those in the input transition t.
			 * \param t Transition object which this instance is being compared to.
			 * \return Returns true if this instance's TimeStamp value is greater than t's TimeStamp.
			*/
			bool afterThan(Transition const &t) const;

			/**
			 * \brief This method is for checking if the Transition object's set of actions, from which it is invoked, occured simultaneously than those in the input transition t.
			 * \param t Transition object which this instance is being compared to.
			 * \return Returns true if this instance's TimeStamp value equals t's TimeStamp.
			*/
			bool atSameTime(Transition const &t) const;
	};
}

#endif

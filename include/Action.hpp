/**
 * \class Action
 * 
 * \brief This class is for representing and manipulating sets of actions. The set of actions contained by an 'Action' objectrepresent actions that occur concurrently, they all begin and end synchronized, and induce a transition between two states within a transition diagram.
 * 
 * \author $Author: Sergio A. Serrano$
 * 
 * \date $Date: 27/09/18$
 * 
 * Contact: sserrano@inaoep.mx
 */

#include <sparc2cpp.hpp>
#include <string>
#include <vector>

#ifndef ACTION
#define ACTION

namespace sp
{
	class Action
	{
		private:
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
			Action();

			/**
			 * \brief Constructor that initializes the object with a set strings that correspond to actions.
			 * \param actionStrings Vector of strings that should correspond to actions.
			*/
			Action(std::vector<std::string> const &actionStrings);

			/**
			 * \brief This method assigns a new set of actions to an Action object. All the strings must be valid strings in order for the assigment of this new set of actions to take place.
			 * \param actionStrings Vector of strings that should correspond to actions.
			 * \return Return true if all the strings in actionStrings were successfully converted to actions, otherwise, false.
			*/
			bool set(std::vector<std::string> const &actionStrings);

			/**
			 * \brief This method is for checking the step time at which the set of actions occur.
			 * \return Returns the value of TimeStamp.
			*/
			int step() const;

			/**
			 * \brief This method is for checking if the Action object from which it is invoked has the same set of actions as the input Action a, regardless of their time stamps.
			 * \param a Action object which this instance is being compared to.
			 * \return Returns true if both Action objects have the same set of actions.
			*/
			bool shareA(Action const &a) const;

			/**
			 * \brief This method is mainly used to display information about the action elements, which is useful for debugging.
			*/
			void debug() const;
	};
}

#endif

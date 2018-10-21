#include <string>
#include <vector>
#include <tuple>

#ifndef SP_POMDP
#define SP_POMDP

namespace sp
{
	class PState;
	class Action;
	class Transition;
	class DomainSort;
	class Pomdp
	{
		private:

			double discount;
			std::string values;
			double default_prob_T;

			/**
			 * \brief This method is for extracting the statistics for the Transition & Observation functions, and the specification of the Reward function, from a text file. The Cassandra formats supported for the Transition and Observation function are: 'identity', 'uniform', a full-matrix, and the one-line specification (T:action:*:end-state 1, or O:action:*:observation 1). In the case of specifications for the Reward function, only the one-line format is supported (R:action:start-state:end-state:observation %f). This method verifies that the actions, states , and observations actually exist in this POMDP, otherwise, those statistic or reward specifications are ignored. What this method does not do is to verify if the elements of the matrices are valid values and add up to 1 for each row. Therefore, care must be taken when writing those matrices.
			 * \param file String holding the path to the text file that contains the T and O functions' statistics, also the reward function specification.
			 * \param actions Vector of strings that holds the list of actions this POMDP has.
			 * \param states Vector of strings that holds the list of states this POMDP has.
			 * \param obs Vector of strings that holds the list of observations this POMDP has.
			 * \return Returns a tuple holding two objects. The first one is a vector of indexes for the vector of actions 'actions', and the second object is a vector of strings that correspnd to the statistics of their corresponding element in the vector of indexes. Reward specification strings are not associated to an action, thus, their corresponding index is alwas -1.
			*/
			std::tuple<std::vector<int>,std::vector<std::string>> getStats(std::string file, std::vector<std::string> const &actions, std::vector<std::string> const &states, std::vector<std::string> const &obs);

		public:

			/**
			 * \brief Default constructor.
			*/
			Pomdp();

			/**
			 * \brief Constructor that initializes the object with the discount value.
			 * \param discount The discount value to use when solving the POMDP.
			*/
			Pomdp(double discount);

			/**
			 * \brief Constructor that initializes the object with the type of values that are used in the reward function, whether it is "reward" or "cost".
			 * \param values String that specifies the type of values used.
			*/
			Pomdp(std::string values);

			/**
			 * \brief Constructor that initializes the object with the discount value, and the type of values that are used in the reward function, whether it is "reward" or "cost".
			 * \param discount The discount value to use when solving the POMDP.
			 * \param values String that specifies the type of values used.
			*/
			Pomdp(double discount,std::string values);

			/**
			 * \brief This method assigns the discount value.
			 * \param discount The discount value to use when solving the POMDP.
			 * \return Returns true if 'discount' is within the range [0,1], otherwise, false.
			*/
			bool setDiscount(double discount);

			/**
			 * \brief This method assigns the type of values that are used in the reward function, whether it is "reward" or "cost".
			 * \param values String that specifies the type of values used.
			 * \return Returns true if 'values' is either "reward" or "cost", otherwise, false.
			*/
			bool setValues(std::string values);

			bool generatePomdp(std::vector<PState> &S,
					   std::vector<PState> &Z,
					   std::vector<Action> const &nkA,
					   std::vector<Action> const &kA,
					   std::vector<Action> const &termA,
					   std::vector<Transition> &T,
					   DomainSort const &ds,
					   std::string const &pomdpFile,
					   std::string const &statFile);
	};
}

#endif

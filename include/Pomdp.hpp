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
			double default_prob_O;
			int float_precision;

			/**
			 * \brief This method is for extracting the statistics for the Transition & Observation functions, and the specification of the Reward function, from a text file. The Cassandra formats supported for the Transition and Observation function are: 'identity', 'uniform', a full-matrix, and the one-line specification (T:action:*:end-state 1, or O:action:*:observation 1). In the case of specifications for the Reward function, only the one-line format is supported (R:action:start-state:end-state:observation %f). This method verifies that the actions, states , and observations actually exist in this POMDP, otherwise, those statistic or reward specifications are ignored. What this method does not do is to verify if the elements of the matrices are valid values and add up to 1 for each row. Therefore, care must be taken when writing those matrices.
			 * \param file String holding the path to the text file that contains the T and O functions' statistics, also the reward function specification.
			 * \param actions Vector of strings that holds the list of actions this POMDP has.
			 * \param states Vector of strings that holds the list of states this POMDP has.
			 * \param obs Vector of strings that holds the list of observations this POMDP has.
			 * \return Returns a tuple holding two objects. The first one is a vector of indexes for the vector of actions 'actions', and the second object is a vector of strings that correspnd to the statistics of their corresponding element in the vector of indexes. Reward specification strings are not associated to an action, thus, their corresponding index is alwas -1.
			*/
			std::tuple<std::vector<int>,std::vector<std::string>> getStats(std::string file,
										       std::vector<std::string> const &actions,
										       std::vector<std::string> const &states,
										       std::vector<std::string> const &obs);

			/**
			 * \brief This method converts literals that follow the val(F,B,I) format from SPARC into an acceptable token name for the Cassandra POMDP format. This method substitutes with a '_' the following characters: '(', ')', and ','.
			 * \param str The original literal string.
			 * \return Returns a valid token name fpr the Cassandra POMDP format.
			*/
			std::string fluent2Cass(std::string const &str);

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
			 * \brief This method is for checking the discount value that will be assigned to the resulting POMDP.
			 * \return Returns the value of 'discount'.
			*/
			double getDiscount() const;

			/**
			 * \brief This method is for checking the type of values that are used in the reward function, whether it is "reward" or "cost".
			 * \return Returns the value of 'values'.
			*/
			std::string getValues() const;

			/**
			 * \brief This method assigns the discount value.
			 * \param discount The discount value to use when solving the POMDP.
			 * \return Returns true if 'discount' is within the range [0,1], otherwise, false.
			*/
			bool setDiscount(double const &discount);

			/**
			 * \brief This method assigns the type of values that are used in the reward function, whether it is "reward" or "cost".
			 * \param values String that specifies the type of values used.
			 * \return Returns true if 'values' is either "reward" or "cost", otherwise, false.
			*/
			bool setValues(std::string const &values);

			/**
			 * \brief This method is for checking the default probability that will be used to construct the rows of the T-matrices whose definition is not provided.
			 * \return Returns the value of 'default_prob_T'.
			*/
			double getT() const;

			/**
			 * \brief This method is for checking the default probability that will be used to construct the rows of the O-matrices whose definition is not provided.
			 * \return Returns the value of '&default_prob_O'.
			*/
			double getO() const;

			/**
			 * \brief This method assigns the default probability for T-matrices.
			 * \param default_prob_T The default probability to be used when constructing T-matrices whose definition is not provided by a statistics file.
			 * \return Returns true if 'default_prob_T' is within the range [0,1], otherwise, false.
			*/
			bool setT(double const &default_prob_T);

			/**
			 * \brief This method assigns the default probability for O-matrices.
			 * \param default_prob_O The default probability to be used when constructing O-matrices whose definition is not provided by a statistics file.
			 * \return Returns true if 'default_prob_O' is within the range [0,1], otherwise, false.
			*/
			bool setO(double const &default_prob_O);

			/**
			 * \brief This method is for generating the Cassandra file of a POMDP. This file is built using the states (S), observations (O), actions (nkA, kA, termA), possible transitions (T). The method assumes that the elements of these vectors were extracted from the domain loaded in 'ds'. 'T' holds the transitions of the type state-action-state, and state-action-observation. The resulting Cassandra POMDP file is stored under the path given by 'pomdpFile'. In case that a file with statistics is not provided, the probabilities for the T and O functions are filled with the class' members 'default_prob_T' and 'default_prob_O'. In case a statistics file is provided, the T, O, and R matrices conatained in this file overwrite the default ones.
			 * \param S Vector of states of the POMDP.
			 * \param Z Vector of observations of the POMDP.
			 * \param nkA Vector of non-knowledge producing actions of the POMDP.
			 * \param kA Vector of knowledge producing actions of the POMDP.
			 * \param termA Vector of actions that terminate the POMDPs execution.
			 * \param T Vector of transitions that indexed over the vectors [S, Z] and [nkA, kA].
			 * \param ds DomainSort object that holds the sorts and causal laws of the domain from which S, Z, nkA, kA, and T were extracted.
			 * \param pomdpFile Name of the output file that will hold the Cassandra format description of the resulting POMDP.
			 * \param statFile File from which the method will try to extract matrices of probabilities for the O, and T functions. Also, from this file the reward function is extracted. The probability matrices and reward specifications must follow the Cassandra format. For the reward function, only the 1-line format is supported. In the case of O, and T, the following formats are supported: 1-line format, 'uniform', 'identity' (only for transition function matrices), and full-matrix.
			 * \return Returns true if the input vectors are non-empty, they were extracted from the domain loaded in 'ds', and 'pomdpFile' is a valid file name, otherwise, false. Passing a valid file name for 'statFile' is optional.
			*/
			bool generatePomdp(std::vector<PState> &S,
					   std::vector<PState> &Z,
					   std::vector<Action> const &nkA,
					   std::vector<Action> const &kA,
					   std::vector<Action> const &termA,
					   std::vector<Transition> &T,
					   DomainSort const &ds,
					   std::string const &pomdpFile,
					   std::string const &statFile);

			/**
			 * \brief This method is for generating a row of a probability matrix for a transition or an observation function. In the case that a row for a T-matrix is being built, then 'Z' must be an empty vector, whereas that for an O-matrix row, both 'S' and 'Z' are required. From all the transitions that have 'S[s0]' for starting-state and action 'a_str' (that is, those transitions from 'T' whose indexes are stored in 'relT'), this method selects the transition that matches the result of the deterministic causal law (which is stored in 'sparcF') of action 'a_str', and assigns to it a probability of 'default_prob_T' or 'default_prob_O', whereas that the other transitions are asigned a probability of (1-'default_prob_T')/(relT.size()-1) or (1-'default_prob_O')/(relT.size()-1). The remaining elements of the row, that correspond to transtitions with ending-states that cannot be reached from 'S[s0]' by executing action 'a_str', are assigned a probability of 0.
			 * \param sparcF SPARC file that describes the domain from which 'a_str' and the elements of 'S', 'Z', and 'T' were generated.
			 * \param s0 Index of the starting state, with respect to 'S'.
			 * \param a_str String version of the action for which a matrix of probabilities is being written.
			 * \param S Vector of states of the POMDP.
			 * \param Z Vector of observations of the POMDP.
			 * \param T Vector of transitions that indexed over the vector of PState obejcts [S, Z].
			 * \param relT Vector of indexes, with respect 'T', of those transitions that start from the state with index 's0' and occur with action 'a_str'.
			 * \param outf The ofstream object onto which the Cassandra file is being written.
			*/
			void writeDefMat(std::string const &sparcF,
				         int const &s0,
					 std::string const &a_str,
				         std::vector<PState> const &S,
				         std::vector<PState> const &Z,
					 std::vector<Transition> const &T,
					 std::vector<int> const &relT,
					 std::ofstream &outf);

			/**
			 * \brief This method saves in a text file a POMDP's transition diagram structure, that is, without the transitions' probabilities. Also, this method computes a check-sum for the resulting file, in order to identify if the file was modified, later on when the file is loaded by the 'loadTD' method.
			 * \param S Vector of PState objects that correspond to the states of a POMDP's transition diagram.
			 * \param Z Vector of PState objects that correspond to the observations of a POMDP's transition diagram.
			 * \param nkA Vector of Action objects that correspond to the non-knowledge actions, that cause a change in the state of a POMDP's transition diagram.
			 * \param kA Vector of Action objects that correspond to the knowledge actions, that enable a POMDP's transition diagram to reach an observation from a given state.
			 * \param termA Vector of Action objects that correspond to the termination actions, that cause the end of a POMDP's execution.
			 * \param T Vector of Transition objects that represent the transitions that can possibly occur within the states and observations in 'S' and 'Z', respectively, given the actions in 'nkA', 'kA', and 'termA'.
			 * \param tdFile Name of the output file that will contain the transition diagram.
			 * \return Returns true if all the input vectors have at least one element, and if 'tdFile' is a reachable (or valid) file name, otherwise, false.
			*/
			bool saveTD(std::vector<PState> const &S,
				    std::vector<PState> const &Z,
				    std::vector<Action> const &nkA,
				    std::vector<Action> const &kA,
				    std::vector<Action> const &termA,
				    std::vector<Transition> const &T,
				    std::string const &tdFile);

			/**
			 * \brief This method saves in a text file a POMDP's transition diagram structure, that is, without the transitions' probabilities. Also, this method computes a check-sum for the resulting file, in order to identify if the file was modified, later on when the file is loaded by the 'loadTD' method.
			 * \param S Vector that will hold the PState objects that correspond to the states of the POMDP's transition diagram to be loaded.
			 * \param Z Vector that will hold the PState objects that correspond to the observations of the POMDP's transition diagram to be loaded.
			 * \param nkA Vector that will hold the Action objects that correspond to the non-knowledge actions, that cause a change in the state of the POMDP's transition diagram to be loaded.
			 * \param kA Vector that will hold the Action objects that correspond to the knowledge actions, that enable the POMDP's transition diagram to be loaded to reach an observation from a given state.
			 * \param termA Vector that will hold the Action objects that correspond to the termination actions, that cause the end of the POMDP's execution.
			 * \param T Vector that will hold the Transition objects that represent the transitions that can possibly occur within the states and observations in 'S' and 'Z', respectively, given the actions in 'nkA', 'kA', and 'termA'.
			 * \param tdFile Name of the input file from which the transition diagram will be loaded.
			 * \return Returns true if 'tdFile' is a reachable (or valid) file written by the 'saveTD' method and whose check-sum correspond to the actual file, otherwise, false.
			*/
			bool loadTD(std::vector<PState> &S,
				    std::vector<PState> &Z,
				    std::vector<Action> &nkA,
				    std::vector<Action> &kA,
				    std::vector<Action> &termA,
				    std::vector<Transition> &T,
				    std::string const &tdFile);

			bool whatStats(std::vector<PState> const &S,
				       std::vector<PState> const &Z,
				       std::vector<Action> const &nkA,
				       std::vector<Action> const &kA,
				       std::vector<Transition> const &T,
				       std::string const &tdFile);
	};
}

#endif

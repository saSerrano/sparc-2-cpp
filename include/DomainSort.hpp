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

			/**
			 * \brief This method verifies if SourceFile is a valid SPARC file to be loaded. Once this verification has been performed, it extracts the sorts and the sub-sort based hierarchy. This method does not make a full logic and sintactic revision to the source file, it assumes that the source file has been executed.
			*/
			void verifyAndLoad();

			/**
			 * \brief The method is for displaying the structure of sorts extracted from SourceFile.
			*/
			void sortDisplay() const;

			/**
			 * \brief The method is for displaying the structure of the sub-sort relation among the sorts extracted from SourceFile.
			*/
			void hierDisplay() const;

			friend bool recursiveCheck(DomainSort const &ds, std::string const &input, int sortToCheck);

			friend bool checkDef(DomainSort const &ds, std::string const &def, std::string const &input);

			friend PState subPstate(DomainSort const &ds, PState const &ps, std::vector<std::string> const &defs);
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
			 * \brief This method is mainly used to display information about the sorts, which is useful for debugging.
			*/
			void debug() const;

			/**
			 * \brief This method is for generating a set of states from a collection of literals, that is, all the possible combinations of values for these literals, such that these combinations of values are consistent with the constraints that describe the domain encoded in the SPARC file.

			 * \param rootGenSort This string is a sort definition of the form #root_sort, its where the method will start to look for literals, that is, literals that are elements of #root_sort or elements of a sort that is sub-sort of #root_sort.

			 * \param subStateDefs contains a list of definitions. For each definition, this method will extract a sub-state (that satisfies the definition) from each state generated. That is why this method's output has the form of a vector of vectors of states.

			 * \return The returned value is a vector of vectors of states. The returned vector holds one vector of states for each definition in 'subStateDefs'. It also holds an extra vector of states that corresponds to the original vector of states.
			*/
			std::vector<std::vector<PState>> generateStates(std::string const &rootGenSort, std::vector<std::string> const &subStateDefs) const;

			std::vector<Transition> generateTransitions(std::vector<PState> const &setS0, std::string const &rootActionSort) const;
	};

}

#endif

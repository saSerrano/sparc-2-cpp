#include <sparc2cpp.hpp>

#include <string>
#include <vector>

#define PATH_TO_SPARC "/home/zero/SPARC/sparc.jar"

#ifndef DOMAIN_SORT
#define DOMAIN_SORT

namespace sp
{
	class PState;
	class DomainSort
	{
		private:

			bool FileIsLoaded;
			std::string StatusString;
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
			 * \return Returns true if a source file has been succesfully loaded.
			*/
			bool fileIsLoaded() const;

			/**
			 * \return Returns the status of the instance, i.e., whether a domain has been extracted, if a source file has  even been attempted to be loaded and so.
			*/
			std::string getStatus() const;

			/**
			 * \return Returns the path to a source file that has been succesfully loaded.
			*/
			std::string getFile() const;

			void debug() const;

			std::vector<std::vector<PState>> generateStates(std::string const &rootGenSort, std::vector<std::string> const &subStateDefs);
	};

}

#endif

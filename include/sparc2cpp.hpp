/**
 * \class DomainSort
 * 
 * \brief This class is for extracting, representing and manipulating the sort of a SPARC file.
 * 
 * \author $Author: Sergio A. Serrano$
 * 
 * \date $Date: 16/09/18$
 * 
 * Contact: sserrano@inaoep.mx
 */

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cstdlib>

namespace sp
{
	/**
	 * \brief This method is for checking if a string ends with a given sub-string.
	 * \param fullString The whole string.
	 * \param ending The sub-string to be checked on the whole string.
	 * \return The output of the verification.
	*/
	bool endsWith(std::string const &fullString,std::string const &ending);

	/**
	 * \brief This method is for segmenting an input string into a list of tokens separated by a delimiter sub-string.
	 * \param fullString The whole input string.
	 * \param delimiter The sub-string that will be considered the token delimiter.
	 * \return The output is the vector of tokens separated by the delimiter string.
	*/
	std::vector<std::string> splitStr(std::string const &fullString,std::string const &delimiter);

	/**
	 * \brief This method is for verifying if a string could be succesfully converted into an integer number.
	 * \param inputString The string to be verified.
	 * \return The output is the result of the verification.
	*/
	bool isInt(std::string const &inputString);

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
			void sortDisplay();
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
			bool fileIsLoaded();

			/**
			 * \return Returns the status of the instance, i.e., whether a domain has been extracted, if a source file has  even been attempted to be loaded and so.
			*/
			std::string getStatus();

			/**
			 * \return Returns the path to a source file that has been succesfully loaded.
			*/
			std::string getFile();
	};
}

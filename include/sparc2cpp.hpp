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

#ifndef SPARC_2_CPP
#define SPARC_2_CPP

namespace sp
{
	class PState;
	class DomainSort;

	std::string shellCmd(std::string cmd);

	/**
	 * \brief This method is for checking if a string ends with a given sub-string.
	 * \param fullString The whole string.
	 * \param ending The sub-string to be checked on the whole string.
	 * \return The output of the verification.
	*/
	bool endsWith(std::string const &fullString,std::string const &ending);

	bool startsWith(std::string const &fullString, std::string const &start);

	std::string strBtwn(std::string const &fullString,int leftLimit, int rightLimit);

	int countOcc(std::string const &fullString, std::string const &s);

	std::vector<std::string> correctCommaSplit(std::vector<std::string> commaSplit);
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

	bool equal(PState const &ps1,PState const &ps2);

	std::vector<PState> pstatesFromFile(std::string const &answerSetsFile);

	bool recursiveCheck(DomainSort const &ds, std::string const &input, int sortToCheck);

	bool checkDef(DomainSort const &ds, std::string const &def, std::string const &input);

	PState subPstate(DomainSort const &ds, PState const &ps, std::vector<std::string> const &defs);
}

#endif

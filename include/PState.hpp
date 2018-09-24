#include <sparc2cpp.hpp>

#include <string>
#include <vector>

#ifndef P_STATE
#define P_STATE

namespace sp
{
	class DomainSort;
	class PState
	{
		private:

			int PStateLength;
			std::string PStateString;
			std::vector<std::string> LitNames;
			std::vector<std::vector<std::string>> LitArgs;
			std::vector<std::vector<std::vector<std::string>>> LitArgArgs;

			bool extractPStateFromString(std::string psString);

			void pstateDisplay() const;

			friend bool equal(PState const &ps1,PState const &ps2);

			friend PState subPstate(DomainSort const &ds, PState const &ps, std::vector<std::string> const &defs);

		public:

			PState();

			PState(std::string psString);

			bool set(std::string psString);

			int len() const;

			std::string str() const;

			void debug() const;
	};
}

#endif

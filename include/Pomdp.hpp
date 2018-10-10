#include <string>
#include <vector>

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

		public:

			Pomdp();

			Pomdp(double discount);

			Pomdp(std::string values);

			Pomdp(double discount,std::string values);

			bool setDiscount(double discount);

			bool setValues(std::string values);

			bool generatePomdp(std::vector<PState> &S,
					   std::vector<Action> const &nkA,
					   std::vector<Action> const &kA,
					   std::vector<Action> const &termA,
					   std::vector<PState> &Z,
					   std::vector<Transition> const &T,
					   DomainSort const &ds,
					   std::string const &pomdpFile);
	};
}

#endif

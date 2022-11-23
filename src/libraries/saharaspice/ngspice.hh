/*
 * utils.hh
 *
 */

#ifndef NGSPICE_HH_
#define NGSPICE_HH_
#include <string>
#include <vector>
#include <map>
#include <tuple>

namespace ngspice{	
	class NgSpice{
		public:
			NgSpice();
			virtual ~NgSpice();
			std::string simulate(std::string netlist);
	};

	class NgSpiceParser{
		public:
			NgSpiceParser();
			virtual ~NgSpiceParser();
			void parse(std::string result, 
				std::vector<std::string>voltages, 
				std::vector<std::tuple<std::string, std::string>>& voltagesRes,
				std::vector<std::string>currents, 
				std::vector<std::tuple<std::string, std::string>>& currentsRes
			);
	};
}
using namespace ngspice;

#endif /* NGSPICE_HH_ */

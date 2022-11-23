/*
 * utils.hh
 *
 */

#ifndef CIRCUIT_HH_
#define CIRCUIT_HH_
#include <string>
#include <vector>
#include <map>

namespace ngspice{	
	class CircuitElement{
		public:
			CircuitElement();
			CircuitElement(std::string name, std::string edge0, std::string edge1, std::string value);
			CircuitElement(std::string name, std::string edge0, std::string edge1);
			CircuitElement(CircuitElement* src);
			virtual ~CircuitElement();
			std::string _name = "";
			std::string _edge0 = "";
			std::string _edge1 = "";
			std::string _value = "";
			std::string toString(); 
	};

	class Circuit {
		public:
			Circuit();
			Circuit(Circuit* src);
			virtual ~Circuit();
			void init();
			bool load(std::string netlist);
			bool load(std::vector<std::string> lines);
			std::string _title;
			std::vector<std::string> _options;
			std::vector<CircuitElement*> _elements;
			std::vector<std::string> _models;
			std::vector<std::string> _dcs;
			std::vector<std::string> _acs;
			std::vector<std::string> _tranes;
			std::vector<std::string> _controls;
			std::map<std::string, bool> _edge_names;

			float _timeStep = 0.02;
			float _timeStep1 =  0.20;
			void setParams(std::vector<std::string> params);
			std::string listing();
			std::string listing(std::vector<std::string> additional_options);
			std::string makeTransCommand(std::string strCmd);
			std::string makeTransCommand();
			std::string _error;
		public:
			bool is_exist_edge(std::string edge);
			bool is_exist_element(std::string element);
		private:
			bool is_valid_edge_name(std::string);
	};
}
using namespace ngspice;

#endif /* CIRCUIT_HH_ */

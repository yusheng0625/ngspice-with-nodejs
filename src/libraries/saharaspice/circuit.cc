/*
 * utils.cc
 *
 */

#include "circuit.hh"
#include "utils.hh"
#include <string.h>

CircuitElement::CircuitElement() {
}
CircuitElement::CircuitElement(CircuitElement* src)
{
	_name = src->_name;
	_edge0 = src->_edge0;
	_edge1 = src->_edge1;
	_value = src->_value;
}
CircuitElement::CircuitElement(std::string name, std::string edge0, std::string edge1, std::string value)
{
	_name= name;
	_edge0= edge0;
	_edge1= edge1;
	_value= value;
}
CircuitElement::CircuitElement(std::string name, std::string edge0, std::string edge1)
{
	_name= name;
	_edge0= edge0;
	_edge1= edge1;
	_value= "";
}

CircuitElement::~CircuitElement() {
}

std::string CircuitElement::toString(){
	return _name + " " + _edge0 + " " + _edge1 + " " + _value;
}



Circuit::Circuit() {
}

Circuit::Circuit(Circuit* src){
	init();

	_title = src->_title;
	for(auto e : src->_options)
		_options.push_back(e);

	for(auto ele : src->_elements)
		_elements.push_back(new CircuitElement(ele));

	for(auto model : src->_models)
		_models.push_back(model);

	for(auto e : src->_tranes)
		_tranes.push_back(e);

	for(auto e : src->_dcs)
		_dcs.push_back(e);

	for(auto e : src->_acs)
		_acs.push_back(e);

	for(auto cmd : src->_controls)
		_controls.push_back(cmd);

	for(auto edge : src->_edge_names)
		_edge_names[edge.first] = true;
}

void Circuit::init()
{
	for(auto ele : _elements)
	{
		delete ele;
	}
	_title = "";
	_elements.clear();
	_controls.clear();
	_models.clear();
	_options.clear();
	_tranes.clear();
	_dcs.clear();
	_acs.clear();
	_edge_names.clear();
}
Circuit::~Circuit() {
	init();
}

std::string Circuit::listing()
{
	std::string elements = "";
	for(auto ele : _options)
	{
		elements += ele + "\n";
	}

	for(auto ele : _elements)
	{
		elements += ele->toString() + "\n";
	}

	for(auto ele : _models)
	{
		elements += ele + "\n";
	}

	for(auto ele : _dcs)
	{
		elements += ele + "\n";
	}

	for(auto ele : _acs)
	{
		elements += ele + "\n";
	}

	// for(auto ele : _tranes)
	// {
	// 	elements += ele + "\n";
	// }

	bool existTransLine = false;
	std::string controls = "";
	std::vector<std::string> control_lines;
	for(auto ele : _controls)
	{
		if(ele.find("tran") ==0){
			existTransLine = true;
			control_lines.push_back(makeTransCommand(ele));
		}
		else{
			control_lines.push_back(ele);
		}			
	}

	if(existTransLine ==false)
	{
		//find index of fist print 
		int index = 0;
		for(auto line: control_lines)
		{
			if(line.find("print ") == 0)
				break;
			index ++;
		}
		control_lines.insert(control_lines.begin() + index, makeTransCommand());
	}

	for(auto ele: control_lines){
		if(controls.length() > 0){
			controls +="\n";
		}
		controls += ele;
	}
	return _title + "\n"  + elements + "\n.control\n" + controls + "\n.endc\n.end\n";
}

std::string Circuit::listing(std::vector<std::string> additional_options)
{
	std::string elements = "";
	for(auto ele : _options)
	{
		elements += ele + "\n";
	}

	for(auto ele : _elements)
	{
		elements += ele->toString() + "\n";
	}

	for(auto ele : _models)
	{
		elements += ele + "\n";
	}

	for(auto ele : _dcs)
	{
		elements += ele + "\n";
	}

	for(auto ele : _acs)
	{
		elements += ele + "\n";
	}

	// for(auto ele : _tranes)
	// {
	// 	elements += ele + "\n";
	// }


	//make controls lines
	bool existTransLine = false;
	std::string controls = "";

	std::vector<std::string> control_lines;
	for(auto ele : _controls)
	{
		if(ele.find("tran") ==0){
			existTransLine = true;
			control_lines.push_back(makeTransCommand(ele));
		}
		else{
			control_lines.push_back(ele);
		}			
	}

	for(auto ele : additional_options)
	{
		if(ele.find("tran") ==0){
			existTransLine = true;
			control_lines.push_back(makeTransCommand(ele));
		}			
		else{
			control_lines.push_back(ele);
		}
	}

	if(existTransLine ==false)
	{
		//find index of fist print 
		int index = 0;
		for(auto line: control_lines)
		{
			if(line.find("print ") == 0)
				break;
			index ++;
		}
		control_lines.insert(control_lines.begin() + index, makeTransCommand());
	}
	for(auto ele: control_lines){
		if(controls.length() > 0){
			controls +="\n";
		}
		controls += ele;
	}

	return _title + "\n"  + elements + "\n.control\n" + controls + "\n.endc\n.end\n";
}

bool Circuit::is_valid_edge_name(std::string name)
{
	if(name.length() ==0)
		return false;
	for(auto c: name){
		if(!isalnum(c))
			return false;
	}
	return true;
}

bool Circuit::is_exist_edge(std::string edge)
{
	return _edge_names.find(edge) != _edge_names.end();
}

bool Circuit::is_exist_element(std::string element)
{
	for(auto ele: _elements)
	{
		if(ele->_name == element)
			return true;
	}
	return false;
}

bool Circuit::load(std::vector<std::string> lines)
{
	
	if(lines.size() ==0)	
		return false;

	//process multi lines
	for(size_t i = lines.size() -1; i >=1; i--)
	{
		auto l = lines[i];
		if(l.find("+ ") == 0)
		{			
			lines[i-1] = lines[i-1] + l.substr(1);
			lines.erase(lines.begin() + i);
		}
	}
	

	//set title
	_title = lines[0];

	//analyze line
	std::vector<std::string> datas;
	std::vector<std::string> datas1;	
	bool bError = false;
	bool bStartedStatement = false;
	for(size_t i=1; i<lines.size(); i++)	{
		auto l = lines[i];
		if(l.length() ==0 || l.at(0) == '*')
			continue;

		if(!bStartedStatement)
		{
			if(l.find(".option") == 0){
				_options.push_back(l);
			}else if(l.find(".model") ==0){
				_models.push_back(l);
			}else if(l.find(".tran") ==0){
				_tranes.push_back(l);
			}else if(l.find(".dc")==0){
				_dcs.push_back(l);
			}else if(l.find(".ac")==0){
				_acs.push_back(l);
			}else if(l.find(".control") ==0){
				bStartedStatement = true;
				//_controls.push_back(l);
			}else{				
				datas.clear();
				Utils::split(l, ' ', datas);
				
				if(datas.size() < 4)
				{
					bError = true;
					_error = l;
					break;
				}

				_edge_names[datas[1]] = true;
				_edge_names[datas[2]] = true;
				//check edge name
				// if( !is_valid_edge_name(datas[1]) || !is_valid_edge_name(datas[2]))
				// {
				// 	bError = true;
				// 	_error = l;
				// 	break;
				// }

				//check if element name exist
				if(is_exist_element(datas[0]))
				{
					return false;
				}


				if(datas.size() == 4)
				{			
					CircuitElement* newEle = new CircuitElement(datas[0], datas[1], datas[2], datas[3]);
					_elements.push_back(newEle);
				}
				else{
					datas1.clear();
					for(size_t j=3; j<datas.size(); j++)
						datas1.push_back(datas[j]);
					CircuitElement* newEle = new CircuitElement(datas[0], datas[1], datas[2], Utils::join(datas1, ' '));
					_elements.push_back(newEle);
				}				
			}
		}else{
			if(l.find(".endc") == 0)
				bStartedStatement = false;
			else if(l.at(0) != '.')
				_controls.push_back(l);
		}
	}

	if(bError)
		init();

	return !bError;
}


bool Circuit::load(std::string netlist)
{
	std::vector<std::string> lines;

	//reading netlist
	Utils::split(netlist, '\n', lines);
	return load(lines);
}

void Circuit::setParams(std::vector<std::string> params)
{
	for(size_t i=0; i<_elements.size(); i++)
	{
		_elements[i]->_value = params[i];
	}
}

std::string Circuit::makeTransCommand(std::string strCmd)
{
	std::vector<std::string> data;
	Utils::split(strCmd, ' ', data);
	std::string res = data[0] + " " + std::to_string(_timeStep) + "ms " + std::to_string(_timeStep1) + "ms " + data[3];
	return res;
}

std::string Circuit::makeTransCommand()
{
	std::string res = "tran " + std::to_string(_timeStep) + "ms " + std::to_string(_timeStep1) + "ms uic";
	return res;
}
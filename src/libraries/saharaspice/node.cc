#include <napi.h>
#include <pthread.h>
#include "global.h"
#include "utils.hh"
#include <string.h>
#include <vector>
#include <unistd.h>
#include "circuit.hh"
#include "ngspice.hh"
#include "i_ngspice.hh"
//#include "foo.h"

#include <ngspice/sharedspice.h>

/*--------- *
 * General  *
 * -------- */
Napi::Object getInfo(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  return Napi::Object::New(env);
}

/*--------------- *
 * Configuration  *
 * -------------- */
Napi::Boolean setConfig(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  if (info.Length() != 1 || !info[0].IsString())
  {
    throw Napi::TypeError::New(env, "Json string expected");
  }
  // config.from_json(info[0].ToString().Utf8Value());
  return Napi::Boolean::New(env, true);
}

Napi::String getConfig(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  // std::string res = config.to_json().dump();
  // return Napi::String::New(env, res.c_str());
  return Napi::String::New(env, "");
}

/*---------- *
 * Commands  *
 * --------- */
Napi::Boolean start(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  if (Global::instance()->_started)
    return Napi::Boolean::New(env, false);
  Global::instance()->_started = true;

  bool res = INgSpice::instance()->init();
  usleep(100000);

  return Napi::Boolean::New(env, res);
}

Napi::Object getStats(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  // return JSON with mode & data array
  Napi::Object json_obj = Napi::Object::New(env);
  return json_obj;
}

Napi::Boolean stop(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  if (!Global::instance()->_started)
    return Napi::Boolean::New(env, false);
  Global::instance()->_started = false;
  return Napi::Boolean::New(env, true);
}

Napi::Object loadCircuit(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  if (info.Length() != 1 || !info[0].IsArray())
  {
    throw Napi::TypeError::New(env, "1 params expected netlist");
  }

  std::vector<std::string> netlist;

  Napi::Array dat = info[0].As<Napi::Array>();
  for (size_t i = 0; i < dat.Length(); i++)
  {
    std::string line = dat.Get(i).ToString().Utf8Value();
    netlist.push_back(line);
  }

  Global *global = Global::instance();
  ngspice::Circuit circuit;
  bool bLoad = circuit.load(netlist);
  if (bLoad)
  {
    if (global->_circuit != nullptr)
      delete global->_circuit;

    global->_circuit = new ngspice::Circuit(&circuit);
  }

  Napi::Object obj = Napi::Object::New(env);
  obj.Set("status", bLoad);
  if (bLoad)
  {
    obj.Set("message", "ok");
    obj.Set("listing", global->_circuit->listing().c_str());
  }
  else
  {
    obj.Set("message", ("error: " + circuit._error).c_str());
    obj.Set("listing", "");
  }
  return obj;
}

Napi::Boolean setCircuitParameters(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  if (info.Length() != 1 || !info[0].IsArray())
  {
    throw Napi::TypeError::New(env, "1 params expected paramlist");
  }

  // check if already loaded circuit
  Global *global = Global::instance();
  if (global->_circuit == nullptr)
  {
    throw Napi::TypeError::New(env, "you didn't load circuit");
  }

  Napi::Array dat = info[0].As<Napi::Array>();
  if (global->_circuit->_elements.size() != dat.Length())
  {
    throw Napi::TypeError::New(env, "mismatch parameter count");
  }

  std::vector<std::string> paramlist;
  for (size_t i = 0; i < dat.Length(); i++)
  {
    std::string line = dat.Get(i).ToString().Utf8Value();
    paramlist.push_back(line);
  }
  global->_circuit->setParams(paramlist);

  return Napi::Boolean::New(env, true);
}

Napi::Object runTransientWithError(Napi::Env &env, std::string error)
{
  Napi::Object obj = Napi::Object::New(env);
  obj.Set("status", false);
  obj.Set("message", error.c_str());
  obj.Set("execution_ms", 0);
  obj.Set("result", "");
  obj.Set("voltages", Napi::Array::New(env));
  obj.Set("currents", Napi::Array::New(env));
  return obj;
}

// Napi::Object runTransient(const Napi::CallbackInfo &info)
// {
//   Napi::Env env = info.Env();
//   if (info.Length() < 5)
//   {
//     throw Napi::TypeError::New(env, "expect parameters timestep, loop_count, additional_options[]");
//   }

//   //check if already loaded circuit
//   Global* global = Global::instance();
//   if(global->_circuit == nullptr)
//   {
//     throw Napi::TypeError::New(env, "you didn't load circuit");
//   }

//   global->_circuit->_timeStep = info[0].ToNumber().FloatValue();
//   global->_circuit->_timeStep1 = info[1].ToNumber().FloatValue();

//   //additional commands
//   std::vector<std::string> additional_options;
//   Napi::Array options = info[2].As<Napi::Array>();
//   for (size_t i = 0; i < options.Length(); i++)
//   {
//     std::string line = options.Get(i).ToString().Utf8Value();
//     additional_options.push_back(line);
//   }

//   //voltage list
//   std::vector<std::string> voltage_list;
//   Napi::Array voltages = info[3].As<Napi::Array>();
//   for (size_t i = 0; i < voltages.Length(); i++)
//   {
//     Napi::Array voltage_edges = voltages.Get(i).As<Napi::Array>();
//     std::vector<std::string> edges;
//     for(size_t j=0; j<voltage_edges.Length(); j++)
//     {
//       auto edge = voltage_edges.Get(j).ToString().Utf8Value();
//       //check if edge name exist
//       if(!global->_circuit->is_exist_edge(edge))
//       {
//         return runTransientWithError(env, "you passed wrong edge name '" + edge + "'");
//       }
//       edges.push_back(edge);
//     }
//     voltage_list.push_back(Utils::join(edges, ','));
//   }

//   if(voltage_list.size() > 0)
//   {
//     std::string line = "print";
//     for(auto s : voltage_list)
//     {
//       line += " v(" + s + ")[9999999]";
//     }
//     additional_options.push_back(line);
//   }

//   //current list
//   std::vector<std::string> current_list;
//   Napi::Array currents = info[4].As<Napi::Array>();
//   for (size_t i = 0; i < currents.Length(); i++)
//   {
//     std::string element = currents.Get(i).ToString().Utf8Value();
//     //check if edge name exist
//     if(!global->_circuit->is_exist_element(element))
//     {
//       return runTransientWithError(env, "you passed wrong element name '" + element + "'");
//     }
//     current_list.push_back(element);
//   }

//   if(current_list.size() > 0)
//   {
//     std::string line = "print";
//     for(auto s : current_list)
//     {
//       line += " " + s + "#branch[999999]";
//     }
//     additional_options.push_back(line);
//   }

//   uint64_t start_ts = Utils::get_timestamp_ms();

//   INgSpice::instance()->loadCircuit(global->_circuit->listing(additional_options));
//   INgSpice::instance()->run();
//   std::string strRes = "";

//   //std::string strRes = simulator.simulate(global->_circuit->listing(additional_options));

//   uint64_t excution_time = Utils::get_timestamp_ms() - start_ts;

//   std::vector<std::tuple<std::string, std::string>> votagesRes;
//   std::vector<std::tuple<std::string, std::string>> currentsRes;
//   ngspice::NgSpiceParser parser;
//   parser.parse(strRes, voltage_list, votagesRes, current_list, currentsRes);

//   if(true)
//   {
//     printf("simulate result=\n%s\n", strRes.c_str());

//     for(auto x: votagesRes)
//     {
//       printf("parse result %s=%s\n", std::get<0>(x).c_str(), std::get<1>(x).c_str());
//     }
//   }

//   Napi::Array voltagesResObj = Napi::Array::New(env, votagesRes.size());
//   int   iRes = 0;
//   for(auto x: votagesRes)
//   {
//     Napi::Object o = Napi::Object::New(env);
//     o.Set(std::get<0>(x), std::get<1>(x));
//     voltagesResObj[iRes] = o;
//     iRes ++;
//   }

//   Napi::Array currentsResObj = Napi::Array::New(env, currentsRes.size());
//   iRes = 0;
//   for(auto x: currentsRes)
//   {
//     Napi::Object o = Napi::Object::New(env);
//     o.Set(std::get<0>(x), std::get<1>(x));
//     currentsResObj[iRes] = o;
//     iRes ++;
//   }

//   bool bError = (voltage_list.size() != votagesRes.size()) || (current_list.size() != currentsRes.size());

//   Napi::Object obj = Napi::Object::New(env);
//   obj.Set("status", bError);
//   obj.Set("message", "ok");
//   obj.Set("execution_ms", excution_time);
//   obj.Set("result", strRes);
//   obj.Set("voltages", voltagesResObj);
//   obj.Set("currents", currentsResObj);
//   return obj;
// }

Napi::Object runTransient(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  if (info.Length() < 5)
  {
    throw Napi::TypeError::New(env, "expect parameters timestep, loop_count, additional_options[]");
  }

  // check if already loaded circuit
  Global *global = Global::instance();
  if (global->_circuit == nullptr)
  {
    throw Napi::TypeError::New(env, "you didn't load circuit");
  }

  global->_circuit->_timeStep = info[0].ToNumber().FloatValue();
  global->_circuit->_timeStep1 = info[1].ToNumber().FloatValue();

  // additional commands
  std::vector<std::string> additional_options;
  Napi::Array options = info[2].As<Napi::Array>();
  for (size_t i = 0; i < options.Length(); i++)
  {
    std::string line = options.Get(i).ToString().Utf8Value();
    additional_options.push_back(line);
  }

  // voltage list
  std::vector<std::vector<std::string>> voltage_list;
  Napi::Array voltages = info[3].As<Napi::Array>();
  for (size_t i = 0; i < voltages.Length(); i++)
  {
    Napi::Array voltage_edges = voltages.Get(i).As<Napi::Array>();
    std::vector<std::string> edges;
    for (size_t j = 0; j < voltage_edges.Length(); j++)
    {
      auto edge = voltage_edges.Get(j).ToString().Utf8Value();
      // check if edge name exist
      if (!global->_circuit->is_exist_edge(edge))
      {
        return runTransientWithError(env, "you passed wrong edge name '" + edge + "'");
      }
      edges.push_back(edge);
    }
    voltage_list.push_back(edges);
  }

  std::vector<std::string> current_list;
  Napi::Array currents = info[4].As<Napi::Array>();
  for (size_t i = 0; i < currents.Length(); i++)
  {
    std::string element = currents.Get(i).ToString().Utf8Value();
    // check if edge name exist
    if (!global->_circuit->is_exist_element(element))
    {
      return runTransientWithError(env, "you passed wrong element name '" + element + "'");
    }
    current_list.push_back(element);
  }

  uint64_t start_ts = Utils::get_timestamp_ms();
  INgSpice::instance()->loadCircuit(global->_circuit->listing(additional_options));

  uint64_t excution_time = Utils::get_timestamp_ms() - start_ts;
  std::vector<std::tuple<std::string, double>> votagesRes;
  std::vector<std::tuple<std::string, double>> currentsRes;
  INgSpice::instance()->getResult(voltage_list, current_list, votagesRes, currentsRes);

  Napi::Array voltagesResObj = Napi::Array::New(env, votagesRes.size());
  int iRes = 0;
  for (auto x : votagesRes)
  {
    Napi::Object o = Napi::Object::New(env);
    o.Set(std::get<0>(x), std::get<1>(x));
    voltagesResObj[iRes] = o;
    iRes++;
  }

  Napi::Array currentsResObj = Napi::Array::New(env, currentsRes.size());
  iRes = 0;
  for (auto x : currentsRes)
  {
    Napi::Object o = Napi::Object::New(env);
    o.Set(std::get<0>(x), std::get<1>(x));
    currentsResObj[iRes] = o;
    iRes++;
  }

  bool bError = (voltage_list.size() != votagesRes.size()) || (current_list.size() != currentsRes.size());

  Napi::Object obj = Napi::Object::New(env);
  obj.Set("status", bError);
  obj.Set("message", "ok");
  obj.Set("execution_ms", excution_time);
  obj.Set("result", INgSpice::instance()->logs());
  obj.Set("voltages", voltagesResObj);
  obj.Set("currents", currentsResObj);
  return obj;
}

// export all functions
Napi::Object Init(Napi::Env env, Napi::Object exports)
{

  exports.Set(Napi::String::New(env, "getInfo"),
              Napi::Function::New(env, getInfo));
  exports.Set(Napi::String::New(env, "getStats"),
              Napi::Function::New(env, getStats));

  exports.Set(Napi::String::New(env, "setConfig"),
              Napi::Function::New(env, setConfig));
  exports.Set(Napi::String::New(env, "getConfig"),
              Napi::Function::New(env, getConfig));

  exports.Set(Napi::String::New(env, "start"),
              Napi::Function::New(env, start));
  exports.Set(Napi::String::New(env, "stop"),
              Napi::Function::New(env, stop));

  exports.Set(Napi::String::New(env, "loadCircuit"),
              Napi::Function::New(env, loadCircuit));
  exports.Set(Napi::String::New(env, "setCircuitParameters"),
              Napi::Function::New(env, setCircuitParameters));
  exports.Set(Napi::String::New(env, "runTransient"),
              Napi::Function::New(env, runTransient));

  return exports;
}

NODE_API_MODULE(SaharaSpice, Init)

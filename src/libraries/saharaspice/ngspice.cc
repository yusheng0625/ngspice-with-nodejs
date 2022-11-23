/*
 * utils.cc
 *
 */

#include "ngspice.hh"
#include "utils.hh"
#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>

NgSpice::NgSpice()
{
}

NgSpice::~NgSpice()
{
}

std::string NgSpice::simulate(std::string netlist)
{
    char buffer[2048];
    std::string result = "";
    std::string cmd = "echo '";
    cmd += netlist + "' | ngspice";

    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe)
    {
        return "error";
    }

    try
    {
        while (fgets(buffer, 2048, pipe) != nullptr)
        {
            result += buffer;
        }
    }
    catch (...)
    {
        pclose(pipe);
        return "error";
    }
    pclose(pipe);
    return result;
}

NgSpiceParser::NgSpiceParser()
{
}
NgSpiceParser::~NgSpiceParser()
{
}
void NgSpiceParser::parse(std::string result,
                          std::vector<std::string> voltages,
                          std::vector<std::tuple<std::string, std::string>> &voltagesRes,
                          std::vector<std::string> currents,
                          std::vector<std::tuple<std::string, std::string>> &currentsRes)
{

    std::vector<std::string> lines;
    Utils::split(result, '\n', lines);

    std::vector<std::string> voltageKeys;
    for (auto x : voltages)
        voltageKeys.push_back("v(" + x + ")[9999999]");

    std::vector<std::string> currentKeys;
    for (auto x : currents)
        voltageKeys.push_back(x + "#branch[999999]");

    for (auto line : lines)
    {
        bool bFind = false;

        // check voltage keys
        for (auto key : voltageKeys)
        {
            if (line.find(key) == 0)
            {
                std::vector<std::string> datas;
                Utils::split(line, '=', datas);
                if (datas.size() == 2)
                    voltagesRes.push_back(std::make_tuple(key, datas[1]));
                bFind = true;
                break;
            }
        }
        if (bFind)
            continue;

        // check currents keys
        for (auto key : currentKeys)
        {
            if (line.find(key) == 0)
            {
                std::vector<std::string> datas;
                Utils::split(line, '=', datas);
                if (datas.size() == 2)
                    currentsRes.push_back(std::make_tuple(key, datas[1]));
                break;
            }
        }
    }
}

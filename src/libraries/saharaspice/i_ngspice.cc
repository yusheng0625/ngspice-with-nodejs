/*
 * utils.cc
 *
 */

#include "i_ngspice.hh"
#include "utils.hh"
#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <string.h>

INgSpice *INgSpice::m_s = nullptr;

INgSpice::INgSpice()
{
}

INgSpice::~INgSpice()
{
}

bool INgSpice::init()
{
    int res = ngSpice_Init(_SendChar, _SendStat, _ControlledExit, _SendData, _SendInitData, _BGThreadRunning, this);
    // int res = ngSpice_Init(_SendChar, _SendStat, _ControlledExit, nullptr, nullptr, _BGThreadRunning, this);
    // command("unset ngbehavior");
    // command("set ngbehavior=lt");

    command("set interactive");
    command("set noaskquit");
    command("set nomoremode");
    command("set ngbehavior=ltps");
    return res == 0;
}

bool INgSpice::loadCircuit(std::string listings)
{
    // printf("listings = %s\n", listings.c_str());

    if (_bLoadedCircuit)
        command("remcirc");

    // command( "source /home/keny/work/eclipse-wrokspaces/sahara-spice/__test__/rc.cir");
    // //run();
    // _bLoadedCircuit = true;

    _bLoadedCircuit = false;

    std::vector<std::string> lines;
    Utils::split(listings, '\n', lines);

    std::vector<char *> circuit;
    for (auto a : lines)
    {
        circuit.push_back(strdup((char *)a.c_str()));
    }
    circuit.push_back(nullptr);
    int res = ngSpice_Circ(circuit.data());
    _bLoadedCircuit = res == 0;

    for (auto a : circuit)
        free(a);

    // print result
    //  for(auto a: _lastVectors)
    //  {
    //      printf("%s=%f, %f\n", a.first.c_str(), a.second.value, a.second.time);
    //  }
    return _bLoadedCircuit;
}

double INgSpice::getVoltage(std::string edge)
{
    // for voltage sometimes it's V(1) or `a` format
    std::string edgeName = "V(" + edge + ")";
    auto it = _lastVectors.find(edgeName);
    if (it != _lastVectors.end())
    {
        return it->second.value;
    }

    // if didn't find voltage
    it = _lastVectors.find(edge);
    if (it != _lastVectors.end())
    {
        return it->second.value;
    }
    return 0.0;
}

double INgSpice::getCurrent(std::string element)
{
    std::string elementName = element + "#branch";
    auto it = _lastVectors.find(elementName);
    if (it != _lastVectors.end())
    {
        return it->second.value;
    }
    return 0.0;
}

void INgSpice::getResult(
    std::vector<std::vector<std::string>> &votagesList,
    std::vector<std::string> &currentsList,
    std::vector<std::tuple<std::string, double>> &voltagesRes,
    std::vector<std::tuple<std::string, double>> &currentsRes)
{
    // get votage list
    for (auto edges : votagesList)
    {
        double voltage = 0.0;
        if (edges.size() == 1)
        {
            voltage = getVoltage(edges[0]);
        }
        else if (edges.size() == 2)
        {
            voltage = getVoltage(edges[0]) - getVoltage(edges[1]);
        }
        std::string name = "v(" + Utils::join(edges, ',') + ")";
        voltagesRes.push_back(std::make_tuple(name, voltage));
    }

    // get current list
    for (auto element : currentsList)
    {
        double current = getCurrent(element);
        std::string name = element + "#branch";
        currentsRes.push_back(std::make_tuple(name, current));
    }
}

bool INgSpice::run()
{
    _logs = "";
    bool success = command("bg_run");
    if (success)
    {
        // wait for end of simulation.
        do
        {
            usleep(10000);
        } while (isRunning());
    }
    // printf("INgSpice::Run result = %s\n", _logs.c_str());

    // char*     currentPlot = ngSpice_CurPlot();
    // char**    allPlots    = ngSpice_AllVecs( currentPlot );
    // int       noOfPlots   = 0;

    // std::vector<std::string> retVal;
    // if( allPlots != nullptr )
    // {
    //     for( char** plot = allPlots; *plot != nullptr; plot++ )
    //         noOfPlots++;

    //     retVal.reserve( noOfPlots );

    //     for( int i = 0; i < noOfPlots; i++, allPlots++ )
    //     {
    //         std::string vec = *allPlots;
    //         retVal.push_back( vec );
    //         printf("vec=$s\n", vec.c_str());
    //     }
    // }

    // pvector_info res = ngGet_Vec_Info("v1");
    // if(res)
    // {
    //     printf("run:: = %s\n", res->v_name);
    // }

    return success;
}

bool INgSpice::stop()
{
    return command("bg_halt"); // bg_* commands execute in a separate thread
}

bool INgSpice::isRunning()
{
    // No need to use C locale here
    return ngSpice_running();
}

bool INgSpice::command(std::string command)
{
    return ngSpice_Command((char *)command.c_str()) == 0;
}

int INgSpice::_SendChar(char *output, int id, void *data)
{
    // strip stdout/stderr from the line
    if ((strncasecmp(output, "stdout ", 7) == 0) || (strncasecmp(output, "stderr ", 7) == 0))
        output += 7;

    if (!m_s->_logs.empty())
        m_s->_logs += "\n";
    m_s->_logs += output;
    return 0;
}
int INgSpice::_SendStat(char *status, int id, void *data)
{
    return 0;
}
int INgSpice::_ControlledExit(int status, NG_BOOL bUnload, NG_BOOL bQuit, int id, void *data)
{
    // printf("INgSpice::_ControlledExit = %d, %d, %d\n", status, bUnload, bQuit);
    return 0;
}
int INgSpice::_SendData(pvecvaluesall all, int count, int id, void *data)
{
    // find time
    double time = 0.0;
    int iTime = -1;
    for (int i = 0; i < all->veccount; i++)
    {
        if (strstr(all->vecsa[i]->name, "time"))
        {
            time = all->vecsa[i]->creal;
            iTime = i;
            break;
        }
    }

    SpiceResult res;
    for (int i = 0; i < all->veccount; i++)
    {
        if (i == iTime)
            continue;
        res.time = time;
        res.value = all->vecsa[i]->creal;
        std::string name(all->vecsa[i]->name);
        m_s->_lastVectors[name] = res;
        //  printf("INgSpice::SendData %s, %f\n", name.c_str(), res.value);
    }
    return 0;
}
int INgSpice::_SendInitData(pvecinfoall all, int id, void *data)
{
    // printf("INgSpice::_SendInitData count = %d\n", all->veccount);
    return 0;
}
int INgSpice::_BGThreadRunning(NG_BOOL bRunning, int id, void *data)
{
    // printf("INgSpice::_BGThreadRunning = %d\n", bRunning);
    return 0;
}

// callbacks
int INgSpice::_GetVSRCData(double *voltages, double actualTime, char *nodeName, int id, void *data)
{
    // printf("INgSpice::_GetVSRCData node = %s\n", nodeName);
}
int INgSpice::_GetISRCData(double *currents, double actualTime, char *nodeName, int id, void *data)
{
    // printf("INgSpice::_GetISRCData node = %s\n", nodeName);
}
int INgSpice::_GetSyncData(double actualTime, double *deltaTime, double oldDeltaTime, int redoStep, int id, int location, void *data)
{
    // printf("INgSpice::_GetSyncData\n");
}

// callbacks
int INgSpice::_SendEvtData(int nodeIdx, double step, double value, char *sVal, void *bVal, int size, int mode, int id, void *data)
{
    // printf("INgSpice::_SendEvtData\n");
}

int INgSpice::_SendInitEvtData(int nodeIdx, int maxNodeIdx, char *nodeName, char *nodeType, int id, void *data)
{
    // printf("INgSpice::_SendInitEvtData\n");
}

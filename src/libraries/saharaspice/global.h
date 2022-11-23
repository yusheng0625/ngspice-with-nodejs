/*
 * global.h
 */

#ifndef SRC_GLOBAL_H_
#define SRC_GLOBAL_H_

#include "circuit.hh"
#include <map>
#include <math.h>


class Global
{
public:
    Global();
    virtual ~Global();
    bool _started;

    ngspice::Circuit* _circuit = nullptr;

protected:
    static Global *m_inst;

public:
    static Global *instance();
};

#endif /* SRC_GLOBAL_H_ */

/*****************************************************************************
 * This module defines methods to create pertinent packets to send across network
 ******************************************************************************/
'use strict'
const _ = require('lodash')
const ds0 = require('../../build/Release/saharaspice.node')
const spice_utils = require('./saharaspice_utils')

var ds_status = [
  {
    state: 'reset',
    error: '',
    circuit: null,
    memory: {},
  },
  {
    state: 'reset',
    error: '',
    circuit: null,
    memory: {},
  },
  {
    state: 'reset',
    error: '',
    circuit: null,
    memory: {},
  },
  {
    state: 'reset',
    error: '',
    circuit: null,
    memory: {},
  },
]

function getStatus({ i, saharaspice }) {
  return ds_status[i]
}

function getInfo({ i, saharaspice }) {
  return JSON.parse(saharaspice.getInfo())
}

function getStats({ i, saharaspice }) {
  return saharaspice.getStats()
}

/*
JSON Format:

...

*/
function setConfig({ i, saharaspice }, j) {
  let res = saharaspice.setConfig(JSON.stringify(j))

  if (!res) {
    ds_status[i].state = 'error'
    ds_status[i].error = 'Error setting config'
  }

  return res
}

function getConfig({ i, saharaspice }) {
  return JSON.parse(saharaspice.getConfig())
}

// bind to sockets and start devsrv
function start({ i, saharaspice }) {
  let res = saharaspice.start()

  if (!res) {
    ds_status[i].state = 'error'
    ds_status[i].error = 'Error starting devsrv'
  } else {
    ds_status[i].state = 'running'
  }

  return res
}

// stop() then start()
function restart({ i, saharaspice }) {
  let res = false

  res &= stop({ i, saharaspice })
  res &= start({ i, saharaspice })

  if (!res) {
    ds_status[i].state = 'error'
    ds_status[i].error = 'Error restarting devsrv'
  } else {
    ds_status[i].state = 'running'
  }

  return res
}

// stop devsrv and release socket
function stop({ i, saharaspice }) {
  let res = saharaspice.stop()

  // clear internal circuit parameters
  ds_status[i].circuit = null
  ds_status[i].memory = {}
  //

  if (!res) {
    ds_status[i].state = 'error'
    ds_status[i].error = 'Error stopping devsrv'
  } else {
    ds_status[i].state = 'stopped'
  }

  return res
}

// reset devsrv and release socket
function reset({ i, saharaspice }) {
  let res = saharaspice.stop()

  if (!res) {
    ds_status[i].state = 'error'
    ds_status[i].error = 'Error stopping devsrv'
  } else {
    ds_status[i].state = 'reset'
  }

  return res
}

function clearStatistics({ i, saharaspice }) {
  let res = saharaspice.clearStatistics()
  return res
}

function loadCircuit({ i, saharaspice }, netlist, initial_values = {}) {
  let res = saharaspice.loadCircuit(netlist)

  ds_status[i].circuit = netlist

  let memory = {}
  // get all capacitors & inductors, and set initial values
  for (const elm of netlist) {
    if (elm[0] == 'c' || elm[0] == 'l') {
      const name = elm.split(' ')[0]

      // check if initial value is being passed in
      let value

      if (name in initial_values) {
        value = initial_values[name]
      } else {
        // check if initial value already exists by default
        if (name in ds_status[i].memory) {
          value = ds_status[i].memory[name].value
        } else {
          // in all other cases, set to 0
          value = 0
        }
      }

      memory[name] = { element: elm, value: value }
    }
  }

  ds_status[i].memory = memory // set initial values

  return res
}

function setCircuitParameters({ i, saharaspice }, paramlist) {
  let res = saharaspice.setCircuitParameters(paramlist)
  return res
}

function runTransient(
  { i, saharaspice },
  timeStep,
  loopCount,
  ret_voltages = [],
  ret_currents = []
) {
  let alter = []
  let probe_vol = []
  let probe_vol_mapping = {}
  let probe_cur = []
  let probe_cur_mapping = {}

  // create alter and probe commands
  for (const name of Object.keys(ds_status[i].memory)) {
    // alter
    const value = ds_status[i].memory[name].value
    alter.push(`alter @${name}[ic]=${value}`)

    // probe - capacitor
    if (name[0] == 'c') {
      // netlist line
      const element = ds_status[i].memory[name].element

      // if capacitor, probe voltage
      let n0 = parseInt(element.split(' ')[1])
      let n1 = parseInt(element.split(' ')[2])

      if (n1 != 0) {
        probe_vol.push([n0, n1])
      } else {
        probe_vol.push([n0])
      }

      probe_vol_mapping[name] = probe_vol.length - 1
    }

    // probe - inductor
    if (name[0] == 'l') {
      probe_cur.push(name)
      probe_cur_mapping[name] = probe_cur.length - 1
    }
  }

  // add on what the user is querying
  for (const v of ret_voltages) {
    if (v.length == 2 && v[1] == 0) {
      probe_vol.push([v[0]])
    } else {
      probe_vol.push(v)
    }
  }

  for (const c of ret_currents) {
    probe_cur.push(c)
  }

  let res = saharaspice.runTransient(
    timeStep,
    loopCount,
    alter,
    probe_vol,
    probe_cur
  )

  // store resulting voltages and currents
  const voltages = spice_utils.parseVoltages(res.voltages)
  const currents = spice_utils.parseCurrents(res.currents)
  for (const name of Object.keys(ds_status[i].memory)) {
    // capacitor
    if (name[0] == 'c') {
      const index = probe_vol_mapping[name]
      const value = voltages[index]
      ds_status[i].memory[name].value = value
    }

    // inductor
    if (name[0] == 'l') {
      const index = probe_cur_mapping[name]
      const value = currents[index]
      ds_status[i].memory[name].value = value
    }
  }

  return {
    status: res.status,
    message: res.message,
    memory: ds_status[i].memory,
    voltages:
      ret_voltages.length > 0 ? voltages.slice(-ret_voltages.length) : [],
    currents:
      ret_currents.length > 0 ? currents.slice(-ret_currents.length) : [],
  }
}

function updateMemory({ i, saharaspice }, memory) {
  for (const name of Object.keys(ds_status[i].memory)) {
    if (name in memory) {
      ds_status[i].memory[name].value = memory[name]
    }
  }
}

function getStatistics({ i, saharaspice }) {
  let res = saharaspice.getStatistics()
  return res
}

let exp_funcs = [
  getInfo,
  getStats,
  setConfig,
  getConfig,
  start,
  restart,
  stop,
  reset,
  getStatus,
  getStatistics,
  clearStatistics,
  loadCircuit,
  setCircuitParameters,
  runTransient,
  updateMemory,
]

module.exports = (instance_num = 0) => {
  let inst

  switch (instance_num) {
    case 0:
      inst = ds0
      break
    case 1:
      inst = ds1
      break
    case 2:
      inst = ds2
      break
    case 3:
      inst = ds3
      break
    default:
      inst = ds0
  }

  let to_exp = {}
  _.each(exp_funcs, (f) => {
    to_exp[f.name] = _.partial(f, { i: instance_num, saharaspice: inst })
  })

  to_exp.instance_num = instance_num

  // additional
  to_exp.parseSpice = spice_utils.parseSpice
  to_exp.drawChart = spice_utils.drawChart

  return to_exp
}

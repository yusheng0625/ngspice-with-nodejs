// mocha/chai cheatsheet - https://gist.github.com/yoavniran/1e3b0162e1545055429e

let { expect, done } = require('chai')

const SaharaSpiceLibrary = require('../index.js')
const {
  getVoltage,
  getResistance,
  getCapacitance,
} = require('../src/libraries/saharaspice_utils.js')
const SaharaSpiceFactory = SaharaSpiceLibrary.SaharaSpiceFactory
const parseSpiceFile = SaharaSpiceFactory().parseSpice
const drawChart = SaharaSpiceFactory().drawChart

function sleep(ms) {
  return new Promise((resolve) => {
    setTimeout(resolve, ms)
  })
}

describe('ngspice', () => {
  // basic tests
  describe('simple test circuit', () => {
    let configJson_mac0 = {
      options: {
        verbose: 0,
      },
    }

    // grab an instance
    let SaharaSpice_mac0 = SaharaSpiceFactory(0)

    it('set configs', async function () {
      SaharaSpice_mac0.setConfig(configJson_mac0)
      SaharaSpice_mac0.start()
    })

    //
    it('load circuit', async function () {
      this.timeout(10000) // no more than 10 seconds
      let res

      const circuit = parseSpiceFile('./__test__/lowpass.cir')
      const input = [circuit.title, ...circuit.elements]

      // send the packet
      SaharaSpice_mac0.loadCircuit(input)
    })

    it('runTransient circuit', async function () {
      this.timeout(10000) // no more than 10 seconds

      let i = 0
      let cycles = 300
      let chartdata = { x: [], y: [] }

      chartdata.x.push(i)
      chartdata.y.push(0)

      while (i < cycles) {
        const res = SaharaSpice_mac0.runTransient(0.02, 0.1, [[4, 0]])

        //
        //console.log(res)

        //
        chartdata.x.push(i)
        chartdata.y.push(res.memory.c1.value)

        //
        i++
        //console.log('cycle', i)
      }

      // writing to file
      //drawChart(chartdata)
    })

    // cleanup and stop devsrv instances
    it('stop devsrv', async function () {
      let res0 = SaharaSpice_mac0.stop()
      expect(res0).to.be.true
    })
  })

  describe('currents test circuit', () => {
    let configJson_mac0 = {
      options: {
        verbose: 0,
      },
    }

    // grab an instance
    let SaharaSpice_mac0 = SaharaSpiceFactory(0)

    it('set configs', async function () {
      SaharaSpice_mac0.setConfig(configJson_mac0)
      SaharaSpice_mac0.start()
    })

    //
    it('load circuit', async function () {
      this.timeout(10000) // no more than 10 seconds
      let res

      const circuit = parseSpiceFile('./__test__/ac.cir')

      const input = [circuit.title, ...circuit.elements]

      // send the packet
      res = SaharaSpice_mac0.loadCircuit(input)
      expect(res.status).to.be.true
    })

    it('runTransient circuit', async function () {
      this.timeout(10000) // no more than 10 seconds

      const res = SaharaSpice_mac0.runTransient(0.02, 0.2, [[1], [2]], ['v1'])
      //console.log(res)
    })

    // cleanup and stop devsrv instances
    it('stop devsrv', async function () {
      let res0 = SaharaSpice_mac0.stop()
      expect(res0).to.be.true
    })
  })

  describe('error testing', () => {
    let configJson_mac0 = {
      options: {
        verbose: 0,
      },
    }

    // grab an instance
    let SaharaSpice_mac0 = SaharaSpiceFactory(0)

    it('set configs', async function () {
      SaharaSpice_mac0.setConfig(configJson_mac0)
      SaharaSpice_mac0.start()
    })
    it('load right circuit', async function () {
      const circuit = parseSpiceFile('./__test__/rc.cir')
      const input = [circuit.title, ...circuit.elements]

      // send the packet
      let res = SaharaSpice_mac0.loadCircuit(input)
      expect(res.status).to.be.true
    })

    it('load duplicated elements in circuit', async function () {
      const circuit = parseSpiceFile('./__test__/rc.cir')
      const input = [circuit.title, ...circuit.elements, 'r1 1 2 10']

      // send the packet
      let res = SaharaSpice_mac0.loadCircuit(input)
      expect(res.status).to.be.false
    })

    it('run transient with invalid edge name', async function () {
      const circuit = parseSpiceFile('./__test__/rc.cir')
      const input = [circuit.title, ...circuit.elements]

      // send the packet
      let res = SaharaSpice_mac0.loadCircuit(input)
      expect(res.status).to.be.true

      res = SaharaSpice_mac0.runTransient(0.02, 0.02, [[4], [2, 1]], [])
      //console.log(res)

      expect(res.status).to.be.false
      //console.log(res.message)
    })

    it('run transient with invalid element name', async function () {
      const circuit = parseSpiceFile('./__test__/rc.cir')
      const input = [circuit.title, ...circuit.elements]

      // send the packet
      let res = SaharaSpice_mac0.loadCircuit(input)
      expect(res.status).to.be.true

      res = SaharaSpice_mac0.runTransient(0.02, 0.02, [[2], [2, 1]], ['c3'])

      expect(res.status).to.be.false
      //console.log(res.message)
    })
  })

  describe('circuit for including .model', () => {
    let configJson_mac0 = {
      options: {
        verbose: 0,
      },
    }

    // grab an instance
    let SaharaSpice_mac0 = SaharaSpiceFactory(0)

    it('set configs', async function () {
      SaharaSpice_mac0.setConfig(configJson_mac0)
      SaharaSpice_mac0.start()
    })

    // drive NAPI SDP packet through mac0, destined for mac1
    it('load circuit', async function () {
      this.timeout(10000) // no more than 10 seconds
      let res

      const circuit = parseSpiceFile('./__test__/rectifier.cir')
      const input = [circuit.title, ...circuit.elements]
      //console.log('circuit', input)

      // send the packet
      res = SaharaSpice_mac0.loadCircuit(input)
      expect(res.status).to.be.true
    })

    it('runTransient circuit', async function () {
      this.timeout(10000) // no more than 10 seconds

      const res = SaharaSpice_mac0.runTransient(0.02, 0.2, [[1], [2], [3]])
      //console.log(res)
    })

    // cleanup and stop devsrv instances
    it('stop devsrv', async function () {
      let res0 = SaharaSpice_mac0.stop()
      expect(res0).to.be.true
    })
  })

  describe('circuit for including .model + multiline', () => {
    let configJson_mac0 = {
      options: {
        verbose: 1,
      },
    }

    // grab an instance
    let SaharaSpice_mac0 = SaharaSpiceFactory(0)

    it('set configs', async function () {
      SaharaSpice_mac0.setConfig(configJson_mac0)
      SaharaSpice_mac0.start()
    })

    // drive NAPI SDP packet through mac0, destined for mac1
    it('load circuit', async function () {
      this.timeout(10000) // no more than 10 seconds
      let res

      const circuit = parseSpiceFile('./__test__/multiline-model.cir')
      const input = [circuit.title, ...circuit.elements]
      //console.log('circuit', input)

      // send the packet
      res = SaharaSpice_mac0.loadCircuit(input)
      expect(res.status).to.be.true
    })

    it('load circuit several models', async function () {
      this.timeout(10000) // no more than 10 seconds
      let res

      const circuit = parseSpiceFile('./__test__/cmos-inverter.cir')
      const input = [circuit.title, ...circuit.elements]

      // send the packet
      res = SaharaSpice_mac0.loadCircuit(input)
      expect(res.status).to.be.true
      //console.log(res.listing)
    })

    it('runTransient circuit', async function () {
      this.timeout(10000) // no more than 10 seconds
      // send the packet
      // const res = SaharaSpice_mac0.runTransient(0.02, 0.5, [
      //   'alter @c1[ic]=2',
      //   'print v(1)[999999] v(2)[999999]',
      // ])
      const res = SaharaSpice_mac0.runTransient(0.02, 0.2, [], [])
      //console.log(res)
    })

    // cleanup and stop devsrv instances
    it('stop devsrv', async function () {
      let res0 = SaharaSpice_mac0.stop()
      expect(res0).to.be.true
    })
  })

  describe('circuit for including .dc', () => {
    let configJson_mac0 = {
      options: {
        verbose: 0,
      },
    }

    // grab an instance
    let SaharaSpice_mac0 = SaharaSpiceFactory(0)

    it('set configs', async function () {
      SaharaSpice_mac0.setConfig(configJson_mac0)
      SaharaSpice_mac0.start()
    })

    // drive NAPI SDP packet through mac0, destined for mac1
    it('load circuit', async function () {
      this.timeout(10000) // no more than 10 seconds
      let res

      const circuit = parseSpiceFile('./__test__/dc.cir')
      const input = [circuit.title, ...circuit.elements]
      //console.log('circuit', input)

      // send the packet
      res = SaharaSpice_mac0.loadCircuit(input)
      expect(res.status).to.be.true
    })

    // cleanup and stop devsrv instances
    it('stop devsrv', async function () {
      let res0 = SaharaSpice_mac0.stop()
      expect(res0).to.be.true
    })
  })

  describe('circuit for including .ac', () => {
    let configJson_mac0 = {
      options: {
        verbose: 0,
      },
    }

    // grab an instance
    let SaharaSpice_mac0 = SaharaSpiceFactory(0)

    it('set configs', async function () {
      SaharaSpice_mac0.setConfig(configJson_mac0)
      SaharaSpice_mac0.start()
    })

    // drive NAPI SDP packet through mac0, destined for mac1
    it('load circuit', async function () {
      this.timeout(10000) // no more than 10 seconds
      let res

      const circuit = parseSpiceFile('./__test__/.ac.cir')
      const input = [circuit.title, ...circuit.elements]
      //console.log('circuit', input)

      // send the packet
      res = SaharaSpice_mac0.loadCircuit(input)
      expect(res.status).to.be.true
    })

    // cleanup and stop devsrv instances
    it('stop devsrv', async function () {
      let res0 = SaharaSpice_mac0.stop()
      expect(res0).to.be.true
    })
  })

  describe('inspect RC Charging circuit', () => {
    let configJson_mac0 = {
      options: {
        verbose: 0,
      },
    }

    // grab an instance
    let SaharaSpice_mac0 = SaharaSpiceFactory(0)

    it('set configs', async function () {
      SaharaSpice_mac0.setConfig(configJson_mac0)
      SaharaSpice_mac0.start()
    })

    // drive NAPI SDP packet through mac0, destined for mac1
    it('load circuit', async function () {
      this.timeout(10000) // no more than 10 seconds
      let res

      const circuit = parseSpiceFile('./__test__/rc.cir')
      const input = [circuit.title, ...circuit.elements]
      //console.log('circuit', input)

      // send the packet
      SaharaSpice_mac0.loadCircuit(input)
    })

    it('check output through runTransient', async function () {
      this.timeout(10000) // no more than 10 seconds

      let status = SaharaSpice_mac0.getStatus()

      //find voltage_supply and capacitance and resistance
      let voltageSupply = 0
      let resistance = 0
      let capacitance = 0
      for (let i = 1; i < status.circuit.length; i++) {
        let line = status.circuit[i]
        if (line.startsWith('vin'))
          voltageSupply = getVoltage(line.split(' ')[3])
        else if (line.startsWith('r'))
          resistance = getResistance(line.split(' ')[3])
        else if (line.startsWith('c'))
          capacitance = getCapacitance(line.split(' ')[3])
      }
      //console.log(voltageSupply, resistance, capacitance)

      expect(voltageSupply).to.be.greaterThan(0)
      expect(resistance).to.be.greaterThan(0)
      expect(capacitance).to.be.greaterThan(0)

      let i = 0
      let cycles = 300
      // let chartdata = { x: [], y: [] }
      // chartdata.x.push(i)
      // chartdata.y.push(0)

      let time = 0
      let period = 0.5 / 1000 //0.5ms
      while (i < cycles) {
        time = time + period
        let y = (-1 * time) / (resistance * capacitance)
        let expectedVoltage = voltageSupply * (1 - Math.pow(Math.E, y))
        const res = SaharaSpice_mac0.runTransient(0.02, 0.5, [[2, 0]])

        //console.log(expectedVoltage, res.memory.c1.value)
        expect(expectedVoltage.toFixed(3)).to.be.equal(
          res.memory.c1.value.toFixed(3)
        )
        //
        // chartdata.x.push(i)
        // chartdata.y.push(expectedVoltage)
        //chartdata.y.push(res.memory.c1.value)
        //
        i++
        //console.log('cycle', i)
      }
      //drawChart(chartdata)
    })

    // cleanup and stop devsrv instances
    it('stop devsrv', async function () {
      let res0 = SaharaSpice_mac0.stop()
      expect(res0).to.be.true
    })
  })

  describe('inspect Voltage Divider circuit', () => {
    let configJson_mac0 = {
      options: {
        verbose: 0,
      },
    }

    // grab an instance
    let SaharaSpice_mac0 = SaharaSpiceFactory(0)

    it('set configs', async function () {
      SaharaSpice_mac0.setConfig(configJson_mac0)
      SaharaSpice_mac0.start()
    })

    // drive NAPI SDP packet through mac0, destined for mac1
    it('load circuit', async function () {
      this.timeout(10000) // no more than 10 seconds
      let res

      const circuit = parseSpiceFile('./__test__/voltage-divider.cir')
      const input = [circuit.title, ...circuit.elements]
      //console.log('circuit', input)

      // send the packet
      SaharaSpice_mac0.loadCircuit(input)
    })

    it('check output through runTransient', async function () {
      this.timeout(10000) // no more than 10 seconds

      let status = SaharaSpice_mac0.getStatus()

      //find voltage_supply and capacitance and resistance
      let voltageSupply = 0
      let resistance1 = 0
      let resistance2 = 0
      for (let i = 1; i < status.circuit.length; i++) {
        let line = status.circuit[i]
        if (line.startsWith('v')) voltageSupply = getVoltage(line.split(' ')[3])
        else if (line.startsWith('r1'))
          resistance1 = getResistance(line.split(' ')[3])
        else if (line.startsWith('r2'))
          resistance2 = getResistance(line.split(' ')[3])
      }
      //console.log(voltageSupply, resistance, capacitance)

      expect(voltageSupply).to.be.greaterThan(0)
      expect(resistance1).to.be.greaterThan(0)
      expect(resistance2).to.be.greaterThan(0)

      let expectedVoltage =
        (voltageSupply * resistance2) / (resistance1 + resistance2)
      const res = SaharaSpice_mac0.runTransient(0.02, 0.2, [['a'], ['b']])
      //console.log(expectedVoltage, res.voltages[1])
      expect(expectedVoltage).to.be.equal(res.voltages[1])
    })

    // cleanup and stop devsrv instances
    it('stop devsrv', async function () {
      let res0 = SaharaSpice_mac0.stop()
      expect(res0).to.be.true
    })
  })

  describe('inspect Fullwave bridge rectifier', () => {
    let configJson_mac0 = {
      options: {
        verbose: 0,
      },
    }

    // grab an instance
    let SaharaSpice_mac0 = SaharaSpiceFactory(0)

    it('set configs', async function () {
      SaharaSpice_mac0.setConfig(configJson_mac0)
      SaharaSpice_mac0.start()
    })

    // drive NAPI SDP packet through mac0, destined for mac1
    it('load circuit', async function () {
      this.timeout(10000) // no more than 10 seconds
      let res

      const circuit = parseSpiceFile('./__test__/rectifier.cir')
      const input = [circuit.title, ...circuit.elements]
      //console.log('circuit', input)

      // send the packet
      SaharaSpice_mac0.loadCircuit(input)
    })

    it('check output through runTransient', async function () {
      this.timeout(10000) // no more than 10 seconds

      let status = SaharaSpice_mac0.getStatus()

      let chartdata = { x: [], y: [], y1: [] }
      chartdata.x.push(0)
      chartdata.y.push(0)

      let hz = 60
      let voltage = 15
      let sinCycles = 4
      let oneSinCycleTime = 1000 / hz //ms
      let totalTime = oneSinCycleTime * sinCycles
      let period = 1.0
      let loopCount = totalTime / period

      let i = 1
      while (i < loopCount) {
        const res = SaharaSpice_mac0.runTransient(period / 10, i * period, [
          [1],
          [2, 3],
        ])
        //console.log(res.voltages[0], res.voltages[1])

        let timeInSinCycle = (i * period) % oneSinCycleTime
        let expectedVoltage =
          voltage * Math.sin(2 * Math.PI * (timeInSinCycle / oneSinCycleTime))

        /*
        console.log(
          ((timeInSinCycle * 360) / oneSinCycleTime).toFixed(2),
          expectedVoltage,
          res.voltages[0],
          res.voltages[1]
        )*/

        expect(Math.round(expectedVoltage * 1000) / 1000).to.be.equal(
          Math.round(res.voltages[0] * 1000) / 1000
        )
        expect(
          Math.abs(Math.abs(expectedVoltage) - res.voltages[1])
        ).to.be.lessThan(2.0)

        chartdata.x.push(i)
        chartdata.y.push(res.voltages[1])
        i = i + 1
      }
      //drawChart(chartdata)
    })

    // cleanup and stop devsrv instances
    it('stop devsrv', async function () {
      let res0 = SaharaSpice_mac0.stop()
      expect(res0).to.be.true
    })
  })
})

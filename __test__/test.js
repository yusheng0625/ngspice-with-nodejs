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

describe('test', () => {
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

      const input = [
        'spice circuit',
        'r0 1 2 10000.000000000',
        'c1 2 0 0.000100000',
        'v0 4 0 dc 10',
        'r2 4 1 100.000000000',
      ]

      // send the packet
      res = SaharaSpice_mac0.loadCircuit(input)
      console.log(res)
    })

    it('runTransient circuit', async function () {
      this.timeout(10000) // no more than 10 seconds

      let i = 0
      let cycles = 10
      let chartdata = { x: [], y: [] }

      chartdata.x.push(i)
      chartdata.y.push(0)

      while (i < cycles) {
        const res = SaharaSpice_mac0.runTransient(100, 1000, [
          [1, 0],
          [2, 0],
        ])

        //
        console.log(res)

        //
        i++
        console.log('cycle', i)
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
})

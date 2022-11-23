const fs = require('fs')
const { ChartJSNodeCanvas } = require('chartjs-node-canvas')

function parseSpice(fname_or_spice) {
  try {
    const circuit_text = fs.readFileSync(fname_or_spice, 'utf8')
    return parseSpiceText(circuit_text)
  } catch (e) {}

  try {
    return parseSpiceText(fname_or_spice)
  } catch (e) {
    console.log('Error parsing spice', e)
  }
}

// parse a spice file into its components
// see https://web.stanford.edu/class/ee133/handouts/general/spice_ref.pdf
function parseSpiceText(circuit_text) {
  title = ''
  elements = []
  control = []

  index = 0
  state = 'title'

  const circuit = circuit_text.split(/\r?\n/)

  while (index < circuit.length) {
    // state control
    if (index == 1) {
      state = 'elements'
    }

    if (circuit[index] == '.control') {
      state = 'control'
    }

    // special conditions
    if (index != 0) {
      // 0 is a special case for the title
      if (circuit[index][0] == '*' || circuit[index] == '') {
        index++
        continue
      }

      if (circuit[index] == '.control') {
        index++
        continue
      }

      if (circuit[index] == '.endc') {
        index++
        continue
      }

      if (circuit[index] == '.end') {
        break
      }
    }

    // data operations
    if (state == 'title') {
      title = circuit[index].trim()
    }

    if (state == 'elements') {
      elements.push(circuit[index].trim())
    }

    if (state == 'control') {
      control.push(circuit[index].trim())
    }

    // next line
    index++
  }

  return { title, elements, control }
}

async function drawChart(chartdata) {
  const width = 400 //px
  const height = 400 //px
  const backgroundColour = 'white' // Uses https://www.w3schools.com/tags/canvas_fillstyle.asp
  const chartJSNodeCanvas = new ChartJSNodeCanvas({
    width,
    height,
    backgroundColour,
  })

  const configuration = {
    type: 'line', // for line chart
    data: {
      labels: chartdata.x,
      datasets: [
        {
          label: 'Data',
          data: chartdata.y,
          fill: false,
          borderColor: ['rgb(51, 204, 204)'],
          borderWidth: 1,
          xAxisID: 'xAxis1', //define top or bottom axis ,modifies on scale
        },
      ],
    },
    options: {
      scales: {
        y: {
          suggestedMin: 0,
        },
      },
    },
  }

  async function run() {
    const dataUrl = await chartJSNodeCanvas.renderToDataURL(configuration)
    const base64Image = dataUrl

    var base64Data = base64Image.replace(/^data:image\/png;base64,/, '')

    fs.writeFile('out.png', base64Data, 'base64', function (err) {
      if (err) {
        console.log(err)
      }
    })
    return dataUrl
  }
  await run()
}

function parseVoltages(voltages) {
  v_obj = []

  for (const reading of voltages) {
    const res = Number(Object.values(reading)[0])
    v_obj.push(res)
  }

  return v_obj
}

function parseCurrents(currents) {
  c_obj = []

  for (const reading of currents) {
    const res = Number(Object.values(reading)[0])
    c_obj.push(res)
  }

  return c_obj
}

function getVoltage(value) {
  let res = 0
  let units = ['mv', 'kv', 'v']
  let multis = [1000000, 1000, 1]
  let strVal = value.toLowerCase()

  for (let i = 0; i < units.length; i++) {
    if (strVal.endsWith(units[i])) {
      let valStr = strVal.substr(0, strVal.length - units[i].length)
      res = Number(valStr) * multis[i]
      return res
    }
  }
  return Number(strVal)
}

function getResistance(value) {
  let res = 0
  let units = ['m', 'k']
  let multis = [1000000, 1000]
  let strVal = value.toLowerCase()

  for (let i = 0; i < units.length; i++) {
    if (strVal.endsWith(units[i])) {
      let valStr = strVal.replace(units[i], '')
      res = Number(valStr) * multis[i]
      return res
    }
  }
  return Number(strVal)
}

function getCapacitance(value) {
  let res = 0
  let units = ['u', 'n', 'p']
  let multis = [1000000, 1000000000, 1000000000000]
  let strVal = value.toLowerCase()

  for (let i = 0; i < units.length; i++) {
    if (strVal.endsWith(units[i])) {
      let valStr = strVal.replace(units[i], '')
      res = Number(valStr) / multis[i]
      return res
    }
  }
  return Number(strVal)
}

module.exports = {
  parseSpice,
  drawChart,
  parseVoltages,
  parseCurrents,
  getVoltage,
  getResistance,
  getCapacitance,
}

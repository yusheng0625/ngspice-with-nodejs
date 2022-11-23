/*****************************************************************************
 * Project: Sahara DevSrv Library v1.0.0
 * Description: This is the library that enables devsrv access through NodeJS
 ******************************************************************************/

'use strict'

const InitializeRouter = require('./src/routes/router')
const SaharaSpiceFactory = require('./src/libraries/saharaspice')

module.exports = { SaharaSpiceFactory, InitializeRouter }

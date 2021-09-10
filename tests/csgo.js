'use strict'

// this is an instance
const basil = require("../lib/basil")

(function main() {
  // capture contents of process named "csgo.exe" in the process list on the instance
  // returns js struct
  const instrument = basil.set("csgo.exe")

  // print process name & pid
  console.log(instrument.name)
  console.log(instrument.pid)

  // read value in memory
  console.log(instrument.read_uint8(0x268f0000))
})();

'use strict'

// this is an instance
const basil = require("../lib/basil")

function main() {
  // capture contents of process named "csgo.exe" in the process list on the instance
  // returns js struct
  const instrument = basil.set("csgo.exe")

  // print name
  console.log(instrument.name)
  console.log(instrument.pid)
  console.log(instrument.read_uint8(0x268f0000))
}
main()
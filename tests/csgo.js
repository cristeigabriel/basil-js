'use strict'

// this is an instance
const process_instance = require("../lib/basil")

// capture contents of process named "csgo.exe" in the process list on the instance
// returns js struct
const instrument = process_instance.set("csgo.exe")

// print process name & pid
console.log(instrument.name)
console.log(instrument.pid)

// print client module base and size
const client = instrument.get_module("client.dll")
console.log(client.start)
console.log(client.size)

// read first 2 bytes of engine.dll (should be 'MZ'), module captured in context.
console.log(instrument.read_uint16_t(0x0, "engine.dll"))


// read first 2 bytes of client.dll (should be 'MZ'), module captured in context already but also
// available with our already stored js data
console.log(instrument.read_uint16_t(client.start))

// find pointer to local player position in entity list
console.log([0x83, 0x3D, -1, -1, -1, -1, -1, 0x75, 0x68, 0x8B, 0x0D, -1, -1, -1, -1, 0x8B, 0x01])

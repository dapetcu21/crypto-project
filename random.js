#!/usr/bin/env node

function nextState(rngstate) {
  var r0 = (Math.imul(18273, rngstate[0] & 0xFFFF) + (rngstate[0] >>> 16)) | 0
  var r1 = (Math.imul(36969, rngstate[1] & 0xFFFF) + (rngstate[1] >>> 16)) | 0
  return [r0, r1]
}

function randomInt32(rngstate) {
  var r0 = rngstate[0]
  var r1 = rngstate[1]
  var x = ((r0 << 16) + (r1 & 0xFFFF)) | 0
  return x
}

function int32ToDouble(x) {
  return (x < 0 ? (x + 0x100000000) : x) * 2.3283064365386962890625e-10
}

function toHex(x) {
  var s = (x < 0 ? (x + 0x100000000) : x).toString(16)
  return new Array(8 - s.length + 1).join('0') + s
}

function test(rngstate) {
  rngstate = nextState(rngstate)
  console.log("Next state: " + toHex(rngstate[0]) + toHex(rngstate[1]))
  var x = randomInt32(rngstate)
  console.log("Random u32: " + toHex(x))
  console.log("Random double: " + int32ToDouble(x))
}

function usage() {
  console.log("Usage: ./random.js <\"generate\"|\"test\"> <seed_MSB> <seed_LSB>")
  process.exit(1)
}

if (process.argv.length < 5) {
  usage()
}

var msb = parseInt(process.argv[3], 16)
var lsb = parseInt(process.argv[4], 16)
var rngstate = [msb, lsb]

switch (process.argv[2]) {
  case 'test':
    test(rngstate)
    break
  case 'generate':
    for (var i = 0; i < 50; i++) {
      rngstate = nextState(rngstate)
      console.log(int32ToDouble(randomInt32(rngstate)))
    }
    break
  default:
    usage()
}

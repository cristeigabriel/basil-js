'use strict'

const basil = require('../build/Release/basil')

module.exports = {
  // takes string, instance holds it's data
  set(str) {
    return basil.set(str)
  }
};

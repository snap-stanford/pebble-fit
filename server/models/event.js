var mongoose = require('mongoose')

var eventSchema = new mongoose.Schema({
  time: {type: Date, required: true},
  type: {type: String, required: true},
  watch: {type: String, required: true},
  created: {type: Date, default: Date.now},
  // additional data
  data: {type: Number}
})

module.exports = mongoose.model('Event', eventSchema)

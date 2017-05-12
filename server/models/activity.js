var mongoose = require('mongoose')

var activitySchema = new mongoose.Schema({
  watch: {type: String, required: true},
  time: {type: Date, required: true},
  steps: {type: Number, required: true},
  created: {type: Date, default: Date.now}
})

module.exports = mongoose.model('Activity', activitySchema)

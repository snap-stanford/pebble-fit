var mongoose = require('mongoose')

var activitySchema = new mongoose.Schema({
  steps: {type: Number, required: true},
  time: {type: Date, required: true},
  watch: {type: String, required: true},
  created: {type: Date, default: Date.now}
})

module.exports = mongoose.model('Activity', activitySchema)

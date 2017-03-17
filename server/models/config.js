var mongoose = require('mongoose');

var configSchema = new mongoose.Schema({
  watch: {type: String, required: true},
  name: {type: String, required: false},
  email: {type: String, required: false},
  timeZone: {type: String, required: true},
  startTime: {type: String, required: true},
  endTime: {type: String, required: true},
  created: {type: Date, default: Date.now},
});

module.exports = mongoose.model('Config', configSchema);

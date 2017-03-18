var mongoose = require('mongoose');

var configSchema = new mongoose.Schema({
  watch: {type: String, required: true},
  timeZone: {type: Number, required: true},
  startTime: {type: String, required: true},
  endTime: {type: String, required: true},
  breakFreq: {type: Number, required: true},
  breakLen: {type: Number, required: true},
  threshold: {type: Number, required: true},
  group: {type: String, required: true},
  name: {type: String, required: false},
  email: {type: String, required: false},
  created: {type: Date, default: Date.now},
});

module.exports = mongoose.model('Config', configSchema);

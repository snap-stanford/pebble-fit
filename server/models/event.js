var mongoose = require('mongoose');

var eventSchema = new mongoose.Schema({
  type: {type: String, required: true},
  time: {type: Date, required: true},
  watch: {type: String, required: true},
  msgid: {type: String, required: false},
  score: {type: Number, required: false},
  created: {type: Date, default: Date.now},
  // additional data
  data: {type: Number}
});

module.exports = mongoose.model('Event', eventSchema);

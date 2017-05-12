var mongoose = require('mongoose');

var eventSchema = new mongoose.Schema({
  watch: {type: String, required: true},
  type: {type: String, required: true},
  time: {type: Date, required: true},
  msgid: {type: String, required: false},
  score: {type: Number, required: false},
  scorediff: {type: Number, required: false},
  created: {type: Date, default: Date.now},
  // additional data
  data: {type: Number}
});

module.exports = mongoose.model('Event', eventSchema);

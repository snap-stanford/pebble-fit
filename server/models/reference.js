var mongoose = require('mongoose');

var referenceSchema = new mongoose.Schema({
  watch: {type: String, required: true},
  scores: {type: Array, required: true},
  created: {type: Date, default: Date.now},
});

module.exports = mongoose.model('Reference', referenceSchema);

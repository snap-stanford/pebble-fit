var mongoose = require('mongoose');

var referenceSchema = new mongoose.Schema({
  watch: {type: String, required: true},
  average: {type: Array, required: true},
  count: {type: Number, required: true},
  best: {type: Number, required: true},
  lastModified: {type: Date, default: Date.now},
});

module.exports = mongoose.model('Reference', referenceSchema);

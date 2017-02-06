var mongoose = require('mongoose');

var groupSchema = new mongoose.Schema({
  name: {type: String, required: true},
  file: {type: String, required: true},
  configUpdatedAt: {type: Date, default: Date.now},
});

module.exports = mongoose.model('Group', groupSchema);

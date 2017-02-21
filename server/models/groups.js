var mongoose = require('mongoose');

var groupSchema = new mongoose.Schema({
  name: {type: String, required: true},
  file: {type: String, required: true},
  configUpdatedAt: {type: Date, required: true},
});

module.exports = mongoose.model('Group', groupSchema);

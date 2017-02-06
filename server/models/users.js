var mongoose = require('mongoose');

var userSchema = new mongoose.Schema({
  watch: {type: String, required: true},
  group: {type: String, required: true},
  createdAt: {type: Date, default: Date.now},
  configUpdatedAt: {type: Date, default: Date.now},
  // additional data
  data: {type: Number}
});

module.exports = mongoose.model('User', userSchema);

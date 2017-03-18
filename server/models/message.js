var mongoose = require('mongoose');

var messageSchema = new mongoose.Schema({
  watch: {type: String, required: true},
  messages: {type: Object, required: true},
  created: {type: Date, default: Date.now},
});

module.exports = mongoose.model('Message', messageSchema);

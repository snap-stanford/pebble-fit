var Message = require('../models/message');

var actionMessages = require('../config/action_messages.json');
var healthMessages = require('../config/health_messages.json');
var outcomeMessages = require('../config/outcome_messages.json');

var input = [actionMessages, healthMessages, outcomeMessages];
//var input = [outcomeMessages];
var messages = [];
//for (var m in [actionMessages, outcomeMessages, healthMessages]) {
input.forEach(function (m) {
  messages = messages.concat(m); // For the new message JSON files that converted from text file.
  //for (var key in m) {
  //  if (m.hasOwnProperty(key)) {
  //      messages.push({"id": key, "content": m[key]});
  //  }
  //}
});
//console.log(messages);

exports.getRandomMessages = function (count) {
  var res = []
  for (var i = 0; i < count; i++) {
    messageGroup = input[Math.floor(Math.random()*input.length)];
    res.push(messageGroup[Math.floor(Math.random()*messageGroup.length)]);
  }
  return res;
};

/**
 * Save the messages that are sent to the user.
 */
exports.save = function (watch, messages, next) {
  //console.log(JSON.stringify(messages));
  var obj = {watch: watch, messages: messages};

  var message = new Message(obj);
  message.save(next);
};

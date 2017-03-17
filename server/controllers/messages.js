var actionMessages = require('../config/action_messages.json');
var healthMessages = require('../config/health_messages.json');
//var outcomeMessages = require('../config/outcome_messages.json'); // TODO: taken out for now.
var outcomeMessages = [];

var input = [actionMessages, outcomeMessages, healthMessages];
var messages = [];
//for (var m in [actionMessages, outcomeMessages, healthMessages]) {
input.forEach(function (m) {
  //messages = messages.concat(m); // For the new message JSON files that converted from text file.
  for (var key in m) {
    if (m.hasOwnProperty(key)) {
        messages.push({"id": key, "content": m[key]});
    }
  }
});
//console.log(messages);

exports.getRandomMessages = function (count) {
  var res = []
  for (var i = 0; i < count; i++) {
    res.push(messages[Math.floor(Math.random()*messages.length)]);
  }
  return res;
};



var actionMessages = require('../config/action_messages.json');
var outcomeMessages = require('../config/outcome_messages.json');
var healthMessages = require('../config/health_messages.json');

var input = [actionMessages, outcomeMessages, healthMessages];
var messages = []
//for (var m in [actionMessages, outcomeMessages, healthMessages]) {
input.forEach(function (m) {
  for (var key in m) {
    if (m.hasOwnProperty(key)) {
        messages.push({"id": key, "content": m[key]});
    }
  }
});

exports.getRandomMessages = function (count) {
  var res = []
  for (var i = 0; i < count; i++) {
    res.push(messages[Math.floor(Math.random()*messages.length)]);
  }
  return res;
};


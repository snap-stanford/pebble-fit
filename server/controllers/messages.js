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

/**
 * Get a random message from the message pool of the messageGroup. Do not include
 * outcome messages that comparing to self if no reference scores exist.
 */
var getARandomMessage = function(messageGroup, hasReference) {
  var res;

  if (hasReference) {
    res = messageGroup[Math.floor(Math.random()*messageGroup.length)];
  } else {
    do {
      res = messageGroup[Math.floor(Math.random()*messageGroup.length)];
      resSub = res.id.substring(0,2);
    } while (resSub === 'ou' || resSub === 'ov' || resSub === 'ow');
  }

  return res;
}

/**
 * Get some random messages.
 */
exports.getRandomMessages = function (count, groupName, hasReference) {
  var res = []
  var messageGroup;
  if (groupName === 'real_time_random') {
    for (var i = 0; i < count; i++) {
      messageGroup = input[Math.floor(Math.random()*input.length)];
      res.push(getARandomMessage(messageGroup, hasReference));
    }
  } else {
    switch (groupName) {
      case 'real_time_action':
        messageGroup = actionMessages;
        break;
      case 'real_time_health':
        messageGroup = healthMessages;
        break;
      case 'real_time_outcome':
        messageGroup = outcomeMessages;
        break;
      default:
        console.log("Error: unrecognized group name " + groupName);
        messageGroup = messages;
    }
    for (var i = 0; i < count; i++) {
      res.push(getARandomMessage(messageGroup, hasReference));
    }
  }
  //for (var i = 0; i < res.length; i++) {
  //  console.log(res[i].id);
  //}

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

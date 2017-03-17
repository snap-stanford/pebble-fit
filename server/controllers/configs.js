var Config = require('../models/config');
var _ = require('lodash');
var moment = require('moment');

exports.save = function (watch, timeZone, startTime, endTime, breakFreq, breakLen, threshold,
                         name, email, next) {
  var obj = {watch: watch, timeZone: timeZone, startTime: startTime, endTime: endTime,
             breakFreq: breakFreq, breakLen: breakLen, threshold: threshold};
  if (name) obj.name = name;
  if (email) obj.data = email;

  var config = new Config(obj);
  config.save(next);
};

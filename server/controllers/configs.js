var Config = require('../models/config');
var _ = require('lodash');
var moment = require('moment');

exports.save = function (watch, name, email, timeZone, startTime, endTime, next) {
  var obj = {watch: watch, timeZone: timeZone, startTime: startTime, endTime: endTime};
  if (name) obj.name = name;
  if (email) obj.data = email;

  var config = new Config(obj);
  config.save(next);
};

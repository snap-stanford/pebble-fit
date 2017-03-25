var Config = require('../models/config');
var _ = require('lodash');

exports.save = function (watch, timeZone, startTime, endTime, breakFreq, breakLen, threshold,
                         group, next) {
  var obj = { watch: watch, 
    timeZone: timeZone, 
    startTime: startTime, 
    endTime: endTime,
    breakFreq: breakFreq, 
    breakLen: breakLen, 
    threshold: threshold,
    group: group };

  var config = new Config(obj);
  config.save(next);
};

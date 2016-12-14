var Event = require('../models/event')
var _ = require('lodash')
var moment = require('moment')

exports.save = function (type, data, time, watch_token, next) {
  var obj = {time: moment.unix(time).toDate(), watch: watch_token, type: type}
  if (data) obj.data = data
  var event = new Event(obj)
  event.save(next)
}

exports.get_between = function (watch_token, start_time, end_time, next) {
  Event.find({watch: watch_token, time: {$gte: start_time, $lte: end_time}})
    .sort('time')
    .select('time type data')
    .lean()
    .exec(next)
}

exports.get_last_recorded_time = function (watch_token, next) {
  Event.findOne({watch: watch_token})
    .sort('-time')
    .lean()
    .exec(function (err, latest_event) {
      if (err) return next(err)
      if (!latest_event) return next(new Error('No last event'))
      return next(null, new Date(latest_event.time))
    })
}

var _ = require('lodash')
var Activity = require('../models/activity')
var moment = require('moment')
var async = require('async')

exports.saveActivities = function (data, start_time, watch_token, next) { 
  var end = moment.unix(start_time).add(data.length - 1, 'minutes').toDate()
  var start = moment.unix(start_time)
  //remove data we already have for this
  Activity.remove({time: {$lte: end, $gte: start}, watch: watch_token}, function (err) {
    if (err) return next(err)
    async.forEachOf(data, function (steps, index, cb) {
      var data_point_time = moment.unix(start_time).add(index, 'minutes').toDate()
      var data_point = new Activity({time: data_point_time, steps: parseInt(steps, 10), watch: watch_token})
      data_point.save(cb)
    }, next)
  })
}

exports.getActivitiesBetween = function (start_time, end_time, next) {
  Activity.find({time: {$lt: moment(end_time).toDate(), $gt: moment(start_time).toDate()}})
  .lean()
  .exec(next)
}

exports.getActivitiesFor = function (watch_token, next) {
  Activity.find({watch: watch_token})
  .lean()
  .exec(next)
}

exports.latest_hour = function (watch_token, next) {
  Activity.find({watch: watch_token})
  .sort('-time')
  .limit(60)
  .lean()
  .exec(next)
}

exports.get_last_activity_time = function (watch_token, next) {
  Activity.findOne({watch: watch_token})
  .sort('-time')
  .lean()
  .exec(function (err, latest_activity) {
    if(err) return next(err)
    if(!latest_activity) return next(new Error('No last activity'))
    return next(null, new Date(latest_activity.time))
  })
}

exports.get_n_days_before_now = function (n, watch_token, next) {
  exports.get_last_activity_time(watch_token, function (err, latest_time) {
    if (err) return next(err)
    var end_time = new Date();
    end_time.setDate(latest_time.getDate() - n);
    var start_time = new Date();
    start_time.setDate(latest_time.getDate() - n - 1);
    Activity.find({watch: watch_token, time: {$gte: start_time, $lte: end_time}})
    .sort('time')
    .lean()
    .exec(next)
  })
}

exports.latest_day = function (watch_token, next) {
  return exports.get_n_days_before_now(0, watch_token, next)
}

exports.last_n_days = function (n, watch_token, next) {
  async.times(n, function (k, cb) {
    exports.get_n_days_before_now(k, watch_token, cb)
  }, next) 
}

exports.last_3_days = function (watch_token, next) {
  return exports.last_n_days(3, watch_token, next)
}


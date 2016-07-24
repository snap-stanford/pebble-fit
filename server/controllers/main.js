var _ = require('lodash')
var async = require('async')
var moment = require('moment')

// models
var Activity = require('../models/activity')
var Event = require('../models/event')

// controllers
var activities = require('./activities')
var events = require('./events')

var get_activities_and_events_between =
function (watch_token, start_time, end_time, next) {
  async.parallel({
    activities: function (cb) {
      activities.get_between(watch_token, start_time, end_time, cb)
    },
    events: function (cb) {
      events.get_between(watch_token, start_time, end_time, cb)
    }
  }, next)
}

exports.latest_hour = function (watch_token, next) {
  activities.get_last_recorded_time(watch_token, function (err, end_time) {
    var start_time = moment(end_time).subtract(1, 'hours').toDate()
    get_activities_and_events_between(watch_token, start_time, end_time, next)
  })
}

var get_n_days_before_now = function (n, watch_token, next) {
  activities.get_last_recorded_time(watch_token, function (err, latest_time) {
    if (err) return next(err)
    var end_time = new Date()
    end_time.setDate(latest_time.getDate() - n)
    var start_time = new Date()
    start_time.setDate(latest_time.getDate() - n - 1)
    get_activities_and_events_between(watch_token, start_time, end_time, next)
  })
}

exports.latest_day = function (watch_token, next) {
  return get_n_days_before_now(0, watch_token, next)
}

var last_n_days = function (n, watch_token, next) {
  async.times(n, function (k, cb) {
    get_n_days_before_now(k, watch_token, cb)
  }, next)
}

exports.compare_yesterday = function (watch_token, next) {
  return last_n_days(2, watch_token, next)
}

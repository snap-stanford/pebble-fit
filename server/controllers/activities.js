var Activity = require('../models/activity')
var moment = require('moment')
var async = require('async')

exports.save = function (data, start_time, watch_token, next) {
  var end = moment.unix(start_time).add(data.length - 1, 'minutes').toDate()
  var start = moment.unix(start_time)
  // remove data we already have for this
  Activity.remove({time: {$lte: end, $gte: start}, watch: watch_token}, function (err) {
    if (err) return next(err)
    async.forEachOf(data, function (steps, index, cb) {
      var data_point_time = moment.unix(start_time).add(index, 'minutes').toDate()
      var data_point = new Activity({time: data_point_time, steps: parseInt(steps, 10), watch: watch_token})
      data_point.save(cb)
    }, next)
  })
}

exports.get_last_recorded_time = function (watch_token, next) {
  Activity.findOne({watch: watch_token})
    .sort('-time')
    .lean()
    .exec(function (err, latest_activity) {
      if (err) return next(err)
      if (!latest_activity) return next(new Error('No last activity'))
      return next(null, new Date(latest_activity.time))
    })
}

exports.get_between = function (watch_token, start_time, end_time, next) {
  Activity.find({watch: watch_token, time: {$gte: start_time, $lte: end_time}})
    .sort('time')
    .lean()
    //.exec(next)
    .exec(function (err, activities) {
        // FIXME: this is a workaround for displaying a whole day range of onto browser,
        // by adding a artificial data point at the current time and another at the 
        // start of the day. Note that this might affect the real data if there is 
        // actually some data collected at these 2 timestamp points.
        var sod = new Date();
        sod.setHours(6,0,0,0);
        if (start_time < sod) {
            activities.push({time: sod, steps: 0});
        }
        activities.push({time: new Date(), steps: 0});

        next(null, activities);
    })
}

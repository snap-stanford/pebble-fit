var Activity = require('../models/activity')

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
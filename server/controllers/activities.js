var Activity = require('../models/activity');
var users = require('./users');
var moment = require('moment');
var async = require('async');

exports.save = function (data, start_time, watch_token, next) {
  var stepUploadTime;

  var end = moment.unix(start_time).add(data.length - 1, 'minutes');
  var start = moment.unix(start_time);

  if (data.length == 1 && isNaN(parseInt(data[0]))) {
    // Re-request one minute piror to start.
    stepUploadTime = end.add(-1, 'minutes').unix();
  } else if (!data[data.length-1] || isNaN(parseInt(data[data.length-1]))) {
    // Re-request the same start time.
    stepUploadTime = end.unix();
  } else {
    stepUploadTime = end.add(1, 'minutes').unix();
  }

  end = end.toDate();

  // Update user's step_upload_time to the end timestampe.
  users.setStepUploadedAt(
    watch_token,
    stepUploadTime,
    function(err) {
      if (err) return next(err);

      // Remove data we already have for this
      Activity.remove({time: {$lte: end, $gte: start}, watch: watch_token}, function (err) {
        if (err) return next(err);

        async.forEachOf(data, function (steps, index, cb) {
          var data_point_time = moment.unix(start_time).add(index, 'minutes').toDate();

          steps = parseInt(steps, 10);
          if (isNaN(steps)) { steps = 0; }

          var data_point = new Activity({time: data_point_time, steps: steps, watch: watch_token});

          data_point.save(cb);
        }, function (err) {
          next(err, stepUploadTime);
        });
      });
    }
  );
}

exports.get_last_recorded_time = function (watch_token, next) {
  Activity.findOne({watch: watch_token})
    .sort('-time')
    .lean()
    .exec(function (err, latest_activity) {
      if (err) return next(err);

      if (!latest_activity) return next(new Error('No last activity'));

      return next(null, new Date(latest_activity.time));
    });
}

exports.get_between = function (watch_token, start_time, end_time, next) {
  Activity.find({watch: watch_token, time: {$gte: start_time, $lte: end_time}})
    .sort('time')
    .lean()
    .exec(next);
}

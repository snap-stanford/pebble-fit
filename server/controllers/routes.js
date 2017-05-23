var express = require('express');
var router = express.Router();
var fs = require('fs');

// Controllers
var query = require('./query');
var main = require('./main');
var users = require('./users');
var groups = require('./groups');
var configs = require('./configs');
var activities = require('./activities');
var events = require('./events');

const configDir = './config/';

/** 
 * Insert a new entry to Events and send a response containing the new configuration back.
 * If config exists, fetch step upload time in additional to any existing config setting.
 */
function saveEvent(req, res, config, next) {
  users.getStepUploadedAt(
    req.query.watch,
    function (err, user) {
      events.save(
        req.path.substr(1),
        req.query.reason,
        req.query.date,
        req.query.watch,
        req.query.msgid,
        req.query.scorediff,
        req.query.score,
        function (err) {
          if (err) return next(err);

          if (config) {
            config.step_upload_time = user.stepUploadedAt;

            res.status(200).json(config).end();
          } else {
            res.status(200).end();
          }
        }
      );
    }
  );
}



router.get(['/config'],
  query.requireParam('query', ['watch', 'timezone', 'startTime', 'endTime',
    'breakFreq', 'breakLen', 'group', 'threshold']),
  function (req, res, next) {

    // This callback function is called after the user entry is updated.
    var callback = function (err) {
      if (err) return next(err);

      // Save configs and then send new configuration settings to user.
      configs.save(
        req.query.watch,
        req.query.timezone,
        req.query.startTime,
        req.query.endTime,
        req.query.breakFreq,
        req.query.breakLen,
        req.query.threshold,
        req.query.group,
        function (err) {
          if (err) return next(err);

          users.getConfig(req.query.watch, true, function (err, config) {
            if (err) return next(err);

            console.log("1.Before save&send: config == " + JSON.stringify(config));
            saveEvent(req, res, config, next);
          });
        });
    };

    if (req.query.first) {
      // If user also upload survey contents, save them first.
      users.update(req.query, callback);
    } else {
      callback(null);
    }
  }
);

router.get(['/launch'],
  query.requireParam('query', ['watch', 'reason', 'msgid', 'date', 'score']),
  function (req, res, next) {
    if (req.query.configrequest && req.query.configrequest === '1') {
      users.getConfig(
        req.query.watch,
        false,
        function (err, config) {
          if (err) return next(err);

          console.log("2.Before save&send: config == " + JSON.stringify(config));
          saveEvent(req, res, config, next);
        });
    } else { // If user not requesting new update.
      var config = {};
      config.step_upload_time = 0;

      saveEvent(req, res, config, next);
    }
  }
);

router.get(['/exit'],
  query.requireParam('query', ['watch', 'reason', 'date']),
  function (req, res, next) {
    saveEvent(req, res, null, next);
  }
);

router.get('/steps',
  query.requireParam('query', ['data', 'watch', 'date']),
  function (req, res, next) {
    activities.save(
      req.query.data.split(','),
      req.query.date,
      req.query.watch,
      function (err, newStepUploadTime) {
        if (err) return next(err);

        var responseObject = {}
        responseObject.step_upload_time = newStepUploadTime;

        res.status(200).json(responseObject).end();
      }
    );
  }
);

/**
 * Params: launchTime, exitTime, launchReason, exitReason, messageID, watchToken
 */
router.get(['/launchexit'],
  query.requireParam('query', 
    ['watch', 'launchtime', 'exittime', 'score', 'launchreason', 'exitreason']),
  function (req, res, next) {
    events.save('launch', req.query.launchreason, req.query.launchtime, 
      req.query.watch, req.query.msgid, req.query.scorediff, req.query.score,
      function (err) {
        if (err) return next(err);

        events.save('exit', req.query.exitreason, req.query.exittime, req.query.watch, 
                    null, null, null,
          function (err) {
            if (err) return next(err);
            res.status(200).end();
          });
      });
  }
);

router.get(['/latest_hour', '/latest_day'],
  query.requireParam('query', ['watch']),
  function (req, res, next) {
    main[req.path.substr(1)](req.query.watch, function (err, data) {
      if (err) return next(err);
      res.json({data: data, type: 'single'});
    });
  }
);

router.get(['/compare_yesterday'],
  query.requireParam('query', ['watch']),
  function (req, res, next) {
    main[req.path.substr(1)](req.query.watch, function (err, data) {
      if (err) return next(err);
      res.json({data: data, type: 'multiple'});
    });
  }
);

router.get('/analytics',
  query.requireParam('query', ['watch']),
  function (req, res, next) {
    activities.get_last_recorded_time(req.query.watch, function (err, last_time) {
      if (err) return next(err);
      res.render('index', {watch: req.query.watch, last_time: last_time});
    });
  }
);

router.get('/',
  function (req, res, next) {
    res.redirect('/analytics?watch=6b33a170f75a2fae307b0cbd139429fc');
  }
);

module.exports = router;

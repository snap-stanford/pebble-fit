var express = require('express')
var router = express.Router()
var fs = require('fs')

// Controllers
var query = require('./query')
var main = require('./main')
var users = require('./users')
var groups = require('./groups')
var configs = require('./configs')
var activities = require('./activities')
var events = require('./events')

const configDir = './config/';

/** 
 * Insert a new entry to Events and send a response containing the new configuration back.
 */
function saveEvent(req, res, config, next) {
  events.save(
    req.path.substr(1),
    req.query.reason,
    req.query.date,
    req.query.watch,
    req.query.msgid,
    req.query.score,
    function (err) {
      if (err) return next(err);
      if (config) res.status(200).json(config).end();
      res.status(200).end();
    });
}



router.get(['/config'],
  query.requireParam('query', ['watch', 'timezone', 'startTime', 'endTime',
    'breakFreq', 'breakLen', 'group', 'threshold']),
  function (req, res, next) {
    var callback = function (err) {
      if (err) return next(err);

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
          res.status(200).end();
        });
    }

    if (req.query.first) {
      // If user also upload survey contents, save them first.
      /*
      users.save(
        req.query.watch, 
        req.query.name, 
        req.query.email, 
        req.query.age, 
        req.query.gender, 
        req.query.height, 
        req.query.heightU, 
        req.query.weight, 
        req.query.weightU, 
        req.query.race, 
        req.query.school, 
        req.query.occupation, 
        req.query.deskwork, 
        req.query.income, 
        req.query.country, 
        req.query.zipcode, 
        callback); */
      users.save(req.query, callback);
    } else {
      callback(null);
    }
  });

router.get(['/launch'],
  //query.requireParam('query', ['watch', 'reason', 'msgid', 'date', 'score']),
  query.requireParam('query', ['watch', 'reason', 'msgid', 'date']),
  function (req, res, next) {
    if (req.query.configrequest && req.query.configrequest === '1') {
      //console.log("DEBUG: user is requesting for new update.");
      /*
      users.getConfigFile(
        req.query.watch,
        function (err, file) {
          if (err) return next(err);

          if (file) { 
            // New update available. Construct the new configuration and then send.
            fetchConfig(req, res, file, next);
          } else { 
            // No new update available. 
            saveEvent(req, res, null, next);
          } 
        });
      */
      users.getConfig(
        req.query.watch,
        function (err, config) {
          if (err) return next(err);

          console.log("Before save&send: config == " + JSON.stringify(config));
          saveEvent(req, res, config, next);
        });
    } else { // If user not requesting new update.
      //console.log("DEBUG: user is NOT requesting for new update.");
      saveEvent(req, res, null, next);
    }
  });

// TODO: Keep delaunch for backward compatibility for a while.
router.get(['/delaunch', '/exit'],
  query.requireParam('query', ['watch', 'reason', 'date']),
  function (req, res, next) {
    saveEvent(req, res, null, next);
  });

router.get('/steps',
  query.requireParam('query', ['data', 'watch', 'date']),
  function (req, res, next) {
    activities.save(
      req.query.data.split(','),
      req.query.date,
      req.query.watch,
      function (err) {
        if (err) return next(err)
        res.status(200).end()
      })
  })

/**
 * Params: launchTime, exitTime, launchReason, exitReason, messageID, watchToken
 */
router.get(['/launchexit'],
  //query.requireParam('query', ['watch', 'launchtime', 'exittime', 'score', 
  query.requireParam('query', ['watch', 'launchtime', 'exittime', 
    'launchreason', 'exitreason']),
  function (req, res, next) {
    events.save('launch', req.query.launchreason, req.query.launchtime, 
      req.query.watch, req.query.msgid, req.query.score,
      function (err) {
        if (err) return next(err);

        events.save('exit', req.query.exitreason, req.query.exittime, req.query.watch, 
                    null, null,
          function (err) {
            if (err) return next(err);
            res.status(200).end();
          });
      });
  });

router.get(['/latest_hour', '/latest_day'],
  query.requireParam('query', ['watch']),
  function (req, res, next) {
    main[req.path.substr(1)](req.query.watch, function (err, data) {
      if (err) return next(err)
      res.json({data: data, type: 'single'})
    })
  })

router.get(['/compare_yesterday'],
  query.requireParam('query', ['watch']),
  function (req, res, next) {
    main[req.path.substr(1)](req.query.watch, function (err, data) {
      if (err) return next(err)
      res.json({data: data, type: 'multiple'})
    })
  })

router.get('/analytics',
  query.requireParam('query', ['watch']),
  function (req, res, next) {
    activities.get_last_recorded_time(req.query.watch, function (err, last_time) {
      if (err) return next(err)
      res.render('index', {watch: req.query.watch, last_time: last_time})
    })
  })

router.get('/',
  function (req, res, next) {
    res.redirect('/analytics?watch=6b33a170f75a2fae307b0cbd139429fc')
  })

module.exports = router

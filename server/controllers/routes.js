var express = require('express')
var router = express.Router()

// Configurations
var messages = require('../config/messages');

// Controllers
var query = require('./query')
var main = require('./main')
var users = require('./users')
var groups = require('./groups')
var activities = require('./activities')
var events = require('./events')
var messages = require('./messages')

var configDir = '../config/';

// Insert a new entry to Events and send a response back.
function saveEvent(req, res, config, next) {
  events.save(
    req.path.substr(1),
    req.query.reason,
    req.query.date,
    req.query.watch,
    function (err) {
      if (err) return next(err);
      if (config) res.status(200).json(config).end();
      res.status(200).end();
    });
}

router.get(['/launch'],
  query.requireParam('query', ['watch', 'date', 'configrequest']),
  function (req, res, next) {
    if (req.query.configrequest === '1') {
      console.log("DEBUG: user is requesting for new update.");
      users.getConfigFile(
        req.query.watch,
        function (err, file) {
          if (err) return next(err);
          if (file) { // New update available.
            console.log("file: "+file);

            // Read the corresponding configuration file.
            delete require.cache[configDir + file]
            var config = require(configDir + file);
            console.log(config);

            // Read the messages.
            for (var m in config.messages) {
              config.messages[m] = messages[config.messages[m]];
            } 
            if (config.hasOwnProperty('random_messages')) {
              var messagesCount = config['random_messages'];
              config['random_messages'] = messages.getRandomMessages(messagesCount);
            }
            console.log(config);
            saveEvent(req, res, config, next);
          } else { // No update available.
            saveEvent(req, res, null, next);
          }
        });
    } else { // If user not requesting new update.
      console.log("DEBUG: user is NOT requesting for new update.");
      saveEvent(req, res, null, next);
    }
  });

// TODO: Keep delaunch for backward compatibility for a while.
router.get(['/delaunch', '/exit'],
  query.requireParam('query', ['watch', 'date']),
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

router.get(['/launchexit'],
  query.requireParam('query', ['launchtime', 'exittime', 'launchreason', 'exitreason']),
  function (req, res, next) {
    events.save('launch', req.query.launchreason, 
      req.query.launchtime, req.query.watch,
      function (err) {
        if (err) return next(err)
        res.status(200).end()
      });
    events.save('exit', req.query.exitreason, 
      req.query.exittime, req.query.watch,
      function (err) {
        if (err) return next(err)
        res.status(200).end()
      });
  }
)

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

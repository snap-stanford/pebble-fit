var express = require('express')
var router = express.Router()

// controllers
var query = require('./query')
var main = require('./main')
var activities = require('./activities')
var events = require('./events')

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

// Keep delaunch for backward compatibility for a while.
router.get(
  ['/launch', '/delaunch', 'exit'],
  query.requireParam('query', ['watch', 'date']),
  function (req, res, next) {
    events.save(
      req.path.substr(1),
      req.query.reason,
      req.query.date,
      req.query.watch,
      function (err) {
        if (err) return next(err)
        res.status(200).end()
      })
  })

router.get(
  '/launchexit',
  query.requireParam('query', ['launchtime', 'exittime', 'launchreason', 'exitreason']),
  function (req, res, next) {
    events.save('launch', req.path.substr(1), req.query.launchreason, 
      req.query.launchtime, req.query.watch,
      function (err) {
        if (err) return next(err)
        res.status(200).end()
      });
    events.save('exit', req.path.substr(1), req.query.exitreason, 
      req.query.exittime, req.query.watch,
      function (err) {
        if (err) return next(err)
        res.status(200).end()
      });
  }
);

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
    //res.redirect('/analytics?watch=6147d09748dd323ff6d0a3cb50b593db')
    res.redirect('/analytics?watch=6b33a170f75a2fae307b0cbd139429fc')
  })

module.exports = router

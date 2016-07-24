var express = require('express')
var router = express.Router()
var query = require('./query')
var main = require('./main')
var activities = require('./activities')
var events = require('./events')
var _ = require('lodash')

router.get('/steps', query.requireParam('query', ['data', 'watch', 'date']),
  function (req, res, next) {
  activity.saveActivities(req.query.data.split(','), req.query.date, req.query.watch,
    function (err) {
    if (err) return next(err)
    res.status(200).end()
  })
})

router.get('/launch', query.requireParam('query', ['reason', 'watch', 'date']),
  function (req, res, next) {
  event.saveEvent('launch', req.query.reason, req.query.date, req.query.watch,
    function (err) {
    if (err) return next(err)
    res.status(200).end()
  })
})

router.get('/delaunch', query.requireParam('query', ['watch', 'date']),
  function (req, res, next) {
  event.saveEvent('delaunch', null, req.query.date, req.query.watch,
    function (err) {
    if (err) return next(err)
    res.status(200).end()
  })
})

router.get(['/latest_hour', '/latest_day', '/last_3_days'], query.requireParam('query', ['watch']),
  function (req, res, next) {
    main[req.path.substr(1)](req.query.watch, function (err, data) {
      if (err) return next(err)
      res.json(data)
    })
  })

router.get('/analytics', query.requireParam('query', ['watch']), function (req, res, next) {
  activities.get_last_activity_time(req.query.watch, function (err, last_time) {
    if (err) return next(err)
    res.render('index', {watch: req.query.watch, last_time: last_time})  
  })
})

router.get('/', function (req, res, next) {
  res.redirect('/analytics?watch=6147d09748dd323ff6d0a3cb50b593db')
})


module.exports = router

var express = require('express')
var router = express.Router()
var query = require('./query')
var main = require('./main')
var _ = require('lodash')

router.get('/collect', query.requireParam('query', ['data', 'watch', 'date']),
  function (req, res, next) {
  main.saveActivities(req.query.data.split(','), req.query.date, req.query.watch, function (err) {
    if (err) return next(err)
    res.json({"message": "ok"})
  })
})

router.get(['/latest_hour', '/today', '/yesterday', '/last_3_days'], query.requireParam('query', ['watch']),
  function (req, res, next) {
    main[req.path.substr(1)](req.query.watch, function (err, activities) {
      if (err) return next(err)
      res.json(activities)
    })
  })

router.get('/', function (req, res, next) {
  res.render('index')
})


module.exports = router

var express = require('express')
var router = express.Router()
var query = require('./query')
var main = require('./main')
var _ = require('lodash')

router.get('/', function (req, res, next) {
  res.render('index')
})

router.get('/collect', function (req, res, next) {
  console.log(req.query.data)
  return res.json({'message': 'ok'})
})

module.exports = router

var express = require('express')
var compress = require('compression')
var path = require('path')
var favicon = require('serve-favicon')
var bodyParser = require('body-parser')
var config = require('config')
var mongoose = require('mongoose')
var stylus = require('stylus')
var moment = require('moment')
var session = require('express-session')
var MongoStore = require('connect-mongo')(session)
var db = require('./controllers/db')
var routes = require('./controllers/routes')
var logging = require('morgan')('dev')
var app = express()

app.set('views', path.join(__dirname, 'views'))
app.set('view engine', 'jade')
app.use(compress())
app.use(favicon(path.join(__dirname, '/public/favicon.ico')))
app.use(bodyParser.json())
app.use(bodyParser.urlencoded({ extended: true }))
app.use('/stylesheets', stylus.middleware({
  src: path.join(__dirname, '/views/styles'),
  dest: path.join(__dirname, '/public/stylesheets'),
  debug: app.get('env') === 'development',
  force: app.get('env') === 'development',
  compile: function (str, path) {
    return stylus(str).set('filename', path).set('compress', true)
  }
}))
app.use(express.static(path.join(__dirname, 'public'), { maxAge: '1h' }))
app.use('/bower_components', express.static(path.join(__dirname, 'bower_components'), { maxAge: '1h' }))
app.use(session({
  store: new MongoStore({ mongooseConnection: mongoose.connection }),
  secret: process.env.app_secret || config.get('app_secret'),
  saveUninitialized: false,
  resave: false,
  cookie: {httpOnly: true}
}))
app.use(logging)
app.use('/', routes)
app.locals.moment = moment
if (app.get('env') === 'development') app.locals.pretty = true
app.use(function (req, res, next) {
  var err = new Error('Page Not Found')
  err.status = 404
  next(err)
})
app.use(logErrors)
app.use(clientErrorHandler)
app.use(errorHandler)
db.initdb() // setup connection to the database

function logErrors (err, req, res, next) {
  req.log = req.log || console
  req.log.error(err)
  next(err)
}

function clientErrorHandler (err, req, res, next) {
  if (req.xhr) {
    res.status(500).send({ error: 'Something blew up!' })
  } else {
    next(err)
  }
}

function errorHandler (err, req, res, next) {
  var message = err.message
  var status = err.status || 500
  err.stack = err.stack || ''
  // if we are in production, let's not give the stack away
  if (app.get('env') === 'production') {
    err.stack = ''
    if (!(err.status === 404 || err.status === 400)) {
      message = 'Something Went Wrong'
    }
  }
  res.status(status)
  res.render('error', {
    message: message,
    error: err,
    status: status
  })
}

module.exports = app

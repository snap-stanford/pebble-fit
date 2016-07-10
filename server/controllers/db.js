var mongoose = require('mongoose')
var config = require('config')

exports.initdb = function (done) {
  mongoose.connect(process.env.MONGODB_URI || config.get('database.' + config.get('database.use')), done)
}

exports.closedb = function (cb) {
  mongoose.disconnect(cb)
}

exports.initproductiondb = function (cb) {
  mongoose.connect(config.get('database.production'), cb)
}

exports.initdbtype = function (str, cb) {
  if (str === 'local') {
    exports.initdb(cb)
  } else if (str === 'production') {
    exports.initproductiondb(cb)
  } else {
    return cb(new Error('db type not defined'))
  }
}

mongoose.connection.on('open', function () {
  console.log('MongoDB connection successful')
})

mongoose.connection.on('error', function (err) {
  console.error({msg: 'MongoDB connection failed', error: err})
})

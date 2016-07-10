/* helper if a request must require a particular parameter */
exports.requireParam = function (current, paramArray) {
  return function (req, res, next) {
    for (var i = 0; i < paramArray.length; i++) {
      var field = paramArray[i]
      if (!req.hasOwnProperty(current) || req[current][field] === undefined) {
        var error = new Error('Undefined Parameter ' + field)
        error.status = 400
        return next(error)
      }
    }
    return next()
  }
}
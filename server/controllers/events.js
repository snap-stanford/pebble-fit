var Event = require('../models/event')

exports.saveEvent = function (type, data, time, watch_token, next) {
  var obj = {time: moment.unix(time).toDate(), watch: watch_token, type: type}
  if (data) obj.data = data
  var event = new Event(obj)
  event.save(next)
}

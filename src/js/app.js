var DEBUG = true
var server = 'http://192.168.1.2:3000'

function info(content) {
  console.log(content)
}

function debug(content) {
  if(DEBUG) info(content)
}

function ajax(url, type, callback) {
  var xhr = new XMLHttpRequest()
  xhr.onload = function () {
    callback(null, this.status, this.response, this.responseText)
  }
  xhr.onerror = function() {
    callback('Network error.')
  }
  xhr.open(type, url)
  xhr.send()
}

Pebble.addEventListener('ready', function() {
  info('PebbleKit JS ready! Version ')

  Pebble.sendAppMessage({'AppKeyJSReady': 1})
})

function send_steps_data(data, date) {
  debug('Uploading steps data...')
    // Convert to string
  var str = '' + data[0];
  for(var i = 1; i < data.length; i++) {
    str += ',' + data[i]
  }
  var url = server + '/collect'
  + '?date=' + date
  + '&data=' + str
  + '&watch=' + Pebble.getWatchToken()

  ajax(url, 'GET', function(err, status) {
    if (err || status !== 200) {
      info(err || status)
    } else {
      Pebble.sendAppMessage({'ServerReceived': 1})
    }
  })
}

Pebble.addEventListener('appmessage', function(dict) {
  debug('Got appmessage: ' + JSON.stringify(dict.payload))

  var data, date

  if(dict.payload['AppKeyStepsEndDate'] != undefined) {
    date = dict.payload['AppKeyStepsEndDate']
    debug('Date: ' + date)
  }

  if(dict.payload['AppKeyStepsData'] != undefined) {
    data = dict.payload['AppKeyStepsData']
    debug('Data: ' + data)
    send_steps_data(data, date)
  }
})
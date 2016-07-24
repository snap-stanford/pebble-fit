var DEBUG = true
var server = 'http://192.168.1.2:3000'

function info(content) {
  console.log(content)
}

function debug(content) {
  if(DEBUG) info(content)
}

Pebble.addEventListener('ready', function() {
  info('PebbleKit JS ready!')
  Pebble.sendAppMessage({'AppKeyJSReady': 1})
})

function send_data_to_url(url) {
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

  ajax(url, 'GET', function(err, status) {
    if (err || status !== 200) {
      info(err || status)
    } else {
      Pebble.sendAppMessage({'AppKeyServerReceived': 1})
    }
  })
}

function send_steps_data(data, date) {
  debug('Uploading steps data...')
    // Convert to string
  var str = '' + data[0];
  for(var i = 1; i < data.length; i++) {
    str += ',' + data[i]
  }
  var url = server + '/steps'
  + '?date=' + date
  + '&data=' + str
  + '&watch=' + Pebble.getWatchToken()

  send_data_to_url(url)
}

function send_launch_data(reason, date) {
  debug('Uploading launch data...')
  var url = server + '/launch'
  + '?date=' + date
  + '&reason=' + reason
  + '&watch=' + Pebble.getWatchToken()

  send_data_to_url(url)
}

function send_delaunch_data(date) {
  debug('Uploading delaunch data...')
  var url = server + '/delaunch'
  + '?date=' + date
  + '&watch=' + Pebble.getWatchToken()

  send_data_to_url(url)
}

Pebble.addEventListener('appmessage', function(dict) {
  debug('Got appmessage: ' + JSON.stringify(dict.payload))

  var date

  if(dict.payload['AppKeyDate'] != undefined) {
    date = dict.payload['AppKeyDate']
    debug('Date: ' + date);
  }

  if(dict.payload['AppKeyStepsData'] != undefined) {
    var data = dict.payload['AppKeyStepsData']
    debug('Data: ' + data)
    send_steps_data(data, date)
  }

  if(dict.payload['AppKeyLaunchReason'] != undefined) {
    var reason = dict.payload['AppKeyLaunchReason']
    debug('Reason: ' + reason)
    send_launch_data(reason, date)
  }

  if(dict.payload['AppKeyDelaunchReason'] != undefined) {
    debug('Delaunched')
    send_delaunch_data(date)
  }
})
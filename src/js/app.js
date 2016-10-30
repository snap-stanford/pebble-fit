var packageinfo = require('../../package.json');

var Clay = require('pebble-clay');
var clayConfig = require('./config.json');
var customClay = require('./custom-clay');
var clay = new Clay(clayConfig, customClay);

var log = require('./logging');
log.set_level(3);

// URL at which to send data
var SERVER = 'http://192.168.1.2:3000';

// Flag to switch off server communication
var USE_OFFLINE = true;

// Start by sending message to watch that ready.
Pebble.addEventListener('ready', function () {
  log.info(packageinfo.pebble.displayName + " " + packageinfo.version + " ready !"    );

  Pebble.sendAppMessage({ 'AppKeyJSReady': 1 });
})

function send_data_to_route (route) {
  function send_request (url, type, callback) {
    var xhr = new XMLHttpRequest()
    xhr.onload = function () {
      callback(null, this.status, this.response, this.responseText)
    }
    xhr.onerror = function () {
      callback('Network error.')
    }
    xhr.open(type, url)
    xhr.send()
  }

  function send_received_message () {
    Pebble.sendAppMessage({
      'AppKeyServerReceived': 1
    })
  }

  if (USE_OFFLINE) {
    send_request(SERVER + route, 'GET', function (err, status) {
      if (err || status !== 200) {
        log.info(err || status)
      } else {
        send_received_message()
      }
    })
  } else {
    send_received_message()
  }
}

function send_steps_data (data, date) {
  log.debug('Uploading steps data...')
  // Convert to string
  var str = '' + data[0]
  for (var i = 1; i < data.length; i++) {
    str += ',' + data[i]
  }
  var url = '/steps' +
  '?date=' + date +
  '&data=' + str +
  '&watch=' + Pebble.getWatchToken()

  send_data_to_route(url)
}

function send_launch_data (reason, date) {
  log.debug('Uploading launch data...')
  var url = '/launch' +
  '?date=' + date +
  '&reason=' + reason +
  '&watch=' + Pebble.getWatchToken()

  send_data_to_route(url)
}

function send_delaunch_data (date) {
  log.debug('Uploading delaunch data...')
  var url = '/delaunch' +
  '?date=' + date +
  '&watch=' + Pebble.getWatchToken()

  send_data_to_route(url)
}

Pebble.addEventListener('appmessage', function (dict) {
  function load_data_array () {
    var length = dict.payload['AppKeyArrayLength']
    var start = dict.payload['AppKeyArrayStart']
    var data = []
    for (var i = start; i < start + length; i++) {
      data.push(dict.payload[String(i)])
    }
    return data
  }

  log.debug('Got appmessage: ' + JSON.stringify(dict.payload))

  var date

  if (dict.payload['AppKeyDate'] !== undefined) {
    date = dict.payload['AppKeyDate']
    log.debug('Date: ' + date)
  }

  if (dict.payload['AppKeyStepsData'] !== undefined) {
    var data = load_data_array()
    log.debug('Data: ' + data)
    send_steps_data(data, date)
  }

  if (dict.payload['AppKeyLaunchReason'] !== undefined) {
    var reason = dict.payload['AppKeyLaunchReason']
    log.debug('Reason: ' + reason)
    send_launch_data(reason, date)
  }

  if (dict.payload['AppKeyDelaunchReason'] !== undefined) {
    log.debug('Delaunched')
    send_delaunch_data(date)
  }
})

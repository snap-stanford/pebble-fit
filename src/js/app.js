var packageinfo = require('../../package.json');

var Clay = require('pebble-clay');
var clayConfig = require('./config.json');
var customClay = require('./custom-clay');
var clay = new Clay(clayConfig, customClay);

// Register custom Clay components.
clay.registerComponent(require('./pfbutton'));

var log = require('./logging');
log.set_level(3);

// URL at which to send data
var SERVER = 'http://pebble-fit.herokuapp.com';

// Local servers (ifconfig).
//var SERVER = 'http://10.30.202.74:3000';
//var SERVER = 'http://10.34.147.77:3000';

// Flag to switch off server communication
var USE_OFFLINE = true;

// Start by sending message to watch that ready.
Pebble.addEventListener('ready', function () {
  log.info(packageinfo.pebble.displayName + " " + packageinfo.version + " ready !");

  Pebble.sendAppMessage({ 'AppKeyJSReady': 1 });
})

function send_data_to_route (route) {
  function send_request (url, type, callback) {
    var xhr = new XMLHttpRequest()
    xhr.onload = function () {
      callback(null, this.status, this.response, this.responseText)
    }
    xhr.onerror = function () {
      callback('Network error. Cannot send to ' + url)
    }
    xhr.open(type, url)
    xhr.send()
  }

  function send_received_message () {
    log.info('Recevied ACK from the server.');
    Pebble.sendAppMessage({
      'AppKeyServerReceived': 1
    })
  }

  if (USE_OFFLINE) {
    log.info(route);
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
  //log.debug('Uploading steps data...')
  log.info('Uploading steps data...')
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

//function send_launch_data (reason, date) {
//  log.debug('Uploading launch data...')
//  var url = '/launch' +
//  '?date=' + date +
//  '&reason=' + reason +
//  '&watch=' + Pebble.getWatchToken()
//
//  send_data_to_route(url)
//}
//
//function send_exit_data (reason, date) {
//  log.debug('Uploading exit data...')
//  var url = '/exit' +
//  '?date=' + date +
//  '&reason=' + reason +
//  '&watch=' + Pebble.getWatchToken()
//
//  send_data_to_route(url)
//}

function send_launch_exit_data (launchTime, exitTime, launchReason, exitReason, date) {
  log.debug('Uploading launch/exit data...')
  if (exitReason === undefined) {
    log.debug('Uploading launch data only...')
    var url = '/launch' + '?date=' + date + '&reason=' + launchReason +
      '&watch=' + Pebble.getWatchToken();
  } else if (launchReason == undefined) {
    log.debug('Uploading exit data only...')
    var url = '/exit' + '?date=' + date + '&reason=' + exitReason +
      '&watch=' + Pebble.getWatchToken();
  } else {
    log.debug('Uploading launch & exit data ...')
    var url = '/launchexit' + '?date=' + date +
      '&launchtime=' + launchTime + '&launchreason=' + launchReason +
      '&exittime=' + exitTime + '&exitreason=' + exitReason +
      '&watch=' + Pebble.getWatchToken();
  }

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

  if (dict.payload['AppKeyLaunchReason'] !== undefined || 
      dict.payload['AppKeyExitReason'] !== undefined) {
    var launchTime = dict.payload['AppKeyLaunchTime'];
    var launchReason = dict.payload['AppKeyLaunchReason'];
    var exitTime = dict.payload['AppKeyExitTime'];
    var exitReason = dict.payload['AppKeyExitReason'];
    send_launch_exit_data(launchTime, exitTime, launchReason, exitReason, date);
  }
        /*
  if (dict.payload['AppKeyLaunchReason'] !== undefined) {
    var reason = dict.payload['AppKeyLaunchReason']
    log.debug('Launched. Reason: ' + reason)
    send_launch_data(reason, date)
  }

  if (dict.payload['AppKeyDelaunchReason'] !== undefined) {
    var reason = dict.payload['AppKeyDelaunchReason']
    log.debug('Delaunched. Reason: ' + reason)
    send_delaunch_data(reason, date)
  }
        */
});

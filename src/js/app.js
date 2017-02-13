var packageinfo = require('../../package.json');

var Clay = require('pebble-clay');
var messageKeys = require('message_keys');
var clayConfig = require('./config.json');
var customClay = require('./custom-clay');
var clay = new Clay(clayConfig, customClay);

// Register custom Clay components.
clay.registerComponent(require('./pfbutton'));

var log = require('./logging');
log.set_level(3);

// URL at which to send data
var SERVER = 'http://pebble-fit.herokuapp.com';

// Local servers (use ifconfig to find out).
var SERVER = 'http://10.30.202.74:3000';
//var SERVER = 'http://10.34.164.91:3000';

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

  // Server might response with new configuration settings. If so, we would apply 
  // them to both the phone app and the watch app.
  function send_received_message(response) {
    if (response === undefined) {
      Pebble.sendAppMessage({ 'AppKeyServerReceived': 1 })
    } else {
      var settings = JSON.parse(response);
      for (var s in settings) {
        log.info(s + ":" + settings[s]);
				if (s == 'messages') {
					log.info(JSON.stringify(settings[s]));
					for (var m in settings[s]) {
        		log.info(m + ":" + settings[s][m]);
					}
				}
						
        clay.setSettings(s, settings[s]);
      }
      Pebble.sendAppMessage({ 'AppKeyServerReceived': 1 })
      //settings['config_update'] = 0; // The first setting in config.json is dummy.
      //Pebble.sendAppMessage(settings, function() {
      //  console.log('Sent config data to Pebble: ' + JSON.stringify(settings));
      //}, function(error) {
      //  console.log('Failed to send config data!');
      //  console.log(JSON.stringify(error));
      //});
    }
  }

  if (USE_OFFLINE) {
    log.info(route);
    send_request(SERVER + route, 'GET', function(err, status, response, responseText) {
      if (err || status !== 200) {
        log.info(err || status)
      } else {
        send_received_message(response);
      }
    })
  } else {
    send_received_message()
  }
}

function send_steps_data(data, date) {
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

function send_launch_exit_data(configRequest, launchTime, exitTime, launchReason, exitReason, date) {
  if (exitReason === undefined) {
    //log.debug('Uploading launch data only...')
    var url = '/launch' + '?date=' + date + '&reason=' + launchReason +
			'&configrequest=' + configRequest + 
      '&watch=' + Pebble.getWatchToken();
  } else if (launchReason === undefined) {
    //log.debug('Uploading exit data only...')
    var url = '/exit' + '?date=' + date + '&reason=' + exitReason +
      '&watch=' + Pebble.getWatchToken();
  } else {
    //log.debug('Uploading launch & exit data ...')
    var url = '/launchexit' + '?date=' + date +
      '&launchtime=' + launchTime + '&launchreason=' + launchReason +
      '&exittime=' + exitTime + '&exitreason=' + exitReason +
      '&watch=' + Pebble.getWatchToken();
  }

  send_data_to_route(url);
}

Pebble.addEventListener('appmessage', function (dict) {
  log.debug('Got appmessage: ' + JSON.stringify(dict.payload));
  var date;

  function load_data_array () {
    var length = dict.payload['AppKeyArrayLength'];
    var start = dict.payload['AppKeyArrayStart'];
    var data = [];
    for (var i = start; i < start + length; i++) {
      data.push(dict.payload[String(i)]);
    }
    return data;
  }

  // Data related to date/timestamp.
  if (dict.payload['AppKeyDate'] !== undefined) {
    date = dict.payload['AppKeyDate']
    log.debug('Date: ' + date);
  }

  // Data related to step count.
  if (dict.payload['AppKeyStepsData'] !== undefined) {
    var data = load_data_array()
    log.debug('Data: ' + data);
    send_steps_data(data, date);
  }

  // Data related to launch and/or exit events.
  if (dict.payload['AppKeyLaunchReason'] !== undefined || 
      dict.payload['AppKeyExitReason'] !== undefined) {
		var configRequest = dict.payload['AppKeyConfigRequest'];
    var launchTime = dict.payload['AppKeyLaunchTime'];
    var launchReason = dict.payload['AppKeyLaunchReason'];
    var exitTime = dict.payload['AppKeyExitTime'];
    var exitReason = dict.payload['AppKeyExitReason'];
    send_launch_exit_data(configRequest, launchTime, exitTime, launchReason, exitReason, date);
  }

  /*
  if (dict.payload['AppKeyTest0'] !== undefined) {
    console.log("Received AppKeyTest0=" + dict.payload['AppKeyTest0']);
    testmode = dict.payload['AppKeyTest0'];
    if (testmode === 0) {
      clay.setSettings('watch_alert_text', 'Alert');
      clay.setSettings('watch_pass_text', 'Pass');
      var temp = {'watch_alert_text': 'Alert', 'watch_pass_text': 'Pass'};
    } else if (testmode === 1) {
      clay.setSettings('watch_alert_text', 'Let\'s Move');
      clay.setSettings('watch_pass_text', 'Keep Up');
      var temp = {'watch_alert_text': 'Let\'s Move', 'watch_pass_text': 'Keep Up'};
    }
    // Send settings to Pebble watchapp
    console.log("AAAAAAAAAAAAAAAAAAAAAAA");
    settings = { 'enamel_key': 0 };
    for (var s in temp) {
      settings[messageKeys[s]] = temp[s];
    }
    console.log(JSON.stringify(settings));
    Pebble.sendAppMessage(settings, function() {
      console.log('Sent config data to Pebble');
    }, function(error) {
      console.log('Failed to send config data!');
      console.log(JSON.stringify(error));
    });
  }
  */
});

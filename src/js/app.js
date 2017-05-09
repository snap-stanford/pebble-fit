var packageinfo = require('../../package.json');

var Clay = require('pebble-clay');
var messageKeys = require('message_keys');
var clayConfig = require('./config.json');
var customClay = require('./custom-clay');
//var clay = new Clay(clayConfig, customClay);
var clay = new Clay(clayConfig, customClay, { autoHandleEvents: false });

// Register custom Clay components.
clay.registerComponent(require('./pfbutton'));

var log = require('./logging');
log.set_level(3);

// URL at which to send data
var SERVER = 'http://pebble-fit.herokuapp.com';

// Local servers (use ifconfig to find out).
//var SERVER = 'http://10.30.202.74:3000';
//var SERVER = 'http://10.34.188.76:3000';
//var SERVER = 'http://10.34.178.45:3000';

// Flag to switch off server communication
var USE_OFFLINE = true;

// Start by sending message to watch that ready.
Pebble.addEventListener('ready', function () {
  log.info(packageinfo.pebble.displayName + " " + packageinfo.version + " ready !");

  Pebble.sendAppMessage({ 'AppKeyJSReady': 1 });
});

/**
 * Receive App message from the watch.
 */
Pebble.addEventListener('appmessage', function (dict) {
  log.debug('Got appmessage: ' + JSON.stringify(dict.payload));
  var date;

  function load_data_array () {
    var length = dict.payload.AppKeyArrayLength;
    var start = dict.payload.AppKeyArrayStart;
    var data = [];
    for (var i = start; i < start + length; i++) {
      data.push(dict.payload[String(i)]);
    }
    return data;
  }

  // Data related to date/timestamp.
  if (dict.payload.AppKeyDate !== undefined) {
    date = dict.payload.AppKeyDate;
    log.debug('Date: ' + date + "; " + new Date(date*1000));
  }

  // Data related to step count.
  if (dict.payload.AppKeyStepsData !== undefined) {
    if (dict.payload.AppKeyStringData !== undefined) {
      var string = dict.payload.AppKeyStringData;
      //if (string !== undefined) {
      //  var data = string.split(',').map(function(num) {return parseInt(num)});
      //}

      //log.debug('Data: ' + data);
      //log.debug('Data length: ' + data.length);
      var url = '/steps' +
      '?date=' + date +
      '&data=' + string +
      '&watch=' + Pebble.getWatchToken();

      sendToServer(url, receiveServerConfigACK);
    } else {
      console.log('Warning: should not reach here since the array data format is deprecated!');
      var data = load_data_array();
      sendStepData(data, date);
    }
  }

  // Data related to launch and/or exit events.
  if (dict.payload.AppKeyLaunchReason !== undefined ||
      dict.payload.AppKeyExitReason !== undefined) {
    var configRequest = dict.payload.AppKeyConfigRequest;
    var msgID = dict.payload.AppKeyMessageID;
    var launchTime = dict.payload.AppKeyLaunchTime;
    var exitTime = dict.payload.AppKeyExitTime;
    var scoreDiff = dict.payload.AppKeyScoreDiff;
    var score = dict.payload.AppKeyBreakCount;
    var launchReason = dict.payload.AppKeyLaunchReason;
    var exitReason = dict.payload.AppKeyExitReason;
    sendLaunchExitData(configRequest, msgID, launchTime, exitTime, scoreDiff, score, 
                       launchReason, exitReason, date);
  }
});

/**
 * Pass through function to handle Clay 'showConfiguration' event.
 */
Pebble.addEventListener('showConfiguration', function(e) {
  Pebble.openURL(clay.generateUrl());
});

/**
 * Override Clay settings Save event. Send information to both the server and the watch.
 */
Pebble.addEventListener('webviewclosed', function(e) {
  log.info("in webviewclosed!!!!!!!!!!!!!!!!!!!!!!");
  if (!e) {
    log.debug("Do not obtain Clay settings properly.");
  }
  if (!e.response) {
    log.info("User exit settings page without save.");
    return;
  }

  // Get the keys and values from each config item.
  var dict = clay.getSettings(e.response);

  // Append user-defined settings and then send to the watch.
  function saveConfigToWatch(settings) {
    settings.first_config       = dict[messageKeys.first_config];
    settings.is_consent         = dict[messageKeys.is_consent];
    settings.activate           = dict[messageKeys.activate];
    settings.vibrate            = dict[messageKeys.vibrate];
    settings.daily_start_time   = dict[messageKeys.daily_start_time];
    settings.daily_end_time     = dict[messageKeys.daily_end_time];
    settings.total_break        = dict[messageKeys.total_break];
    //settings.break_freq         = dict[messageKeys.break_freq];
    //settings.break_len          = dict[messageKeys.break_len];
    
    Pebble.sendAppMessage(settings, function(e) {
      console.log('Sent config data to Pebble (' + JSON.stringify(settings) + ').');
    }, function(e) {
      console.log('Failed to send config data!');
      console.log(JSON.stringify(e));
    }); 
  }

  if (!dict[messageKeys.is_consent]) {
    log.info("User did not provide the consent to continue the study.");
    saveConfigToWatch({});

    return;
  }

  // Prepare URL containing info to be sent to the server.
  var date = Math.floor(new Date().getTime() / 1000);
  var url = '/config' + '?date=' + date + 
    '&watch='       + Pebble.getWatchToken()                                  + 
    '&timezone='    + dict[messageKeys.time_zone]                             + 
    '&startTime='   + dict[messageKeys.daily_start_time]                      + 
    '&endTime='     + dict[messageKeys.daily_end_time]                        + 
    '&breakFreq='   + dict[messageKeys.break_freq]                            + 
    '&breakLen='    + dict[messageKeys.break_len]                             + 
    '&threshold='   + dict[messageKeys.step_threshold]                        + 
    '&group='       + dict[messageKeys.group]; 

  if (dict[messageKeys.first_config]) {
    url = url +
    '&first='       + dict[messageKeys.first_config]                          + 
    '&name='        + encodeURIComponent(dict[messageKeys.consent_name])      + 
    '&email='       + encodeURIComponent(dict[messageKeys.consent_email])     +
    
    '&age='         + dict[messageKeys.survey_age]                            +
    '&gender='      + dict[messageKeys.survey_gender]                         +
    '&height='      + encodeURIComponent(dict[messageKeys.survey_height])     +
    '&heightU='     + dict[messageKeys.survey_height_unit]                    +
    '&weight='      + encodeURIComponent(dict[messageKeys.survey_weight])     +
    '&weightU='     + dict[messageKeys.survey_weight_unit]                    +
    '&race='        + dict[messageKeys.survey_race]                           +
    '&school='      + dict[messageKeys.survey_school]                         +
    '&occupation='  + dict[messageKeys.survey_occupation]                     +
    '&deskwork='    + dict[messageKeys.survey_deskwork]                       +
    '&income='      + dict[messageKeys.survey_income]                         +
    '&country='     + encodeURIComponent(dict[messageKeys.survey_country])    +
    '&zipcode='     + encodeURIComponent(dict[messageKeys.survey_zipcode])    +

    '&sit1='        + encodeURIComponent(dict[messageKeys.survey_sit_1])      +
    '&sit2='        + encodeURIComponent(dict[messageKeys.survey_sit_2])      +
    '&sit3='        + encodeURIComponent(dict[messageKeys.survey_sit_3])      +
    '&sit4='        + encodeURIComponent(dict[messageKeys.survey_sit_4])      +
    '&sit5='        + encodeURIComponent(dict[messageKeys.survey_sit_5])      +
    '&sit6='        + encodeURIComponent(dict[messageKeys.survey_sit_6])      +
    '&sit7='        + encodeURIComponent(dict[messageKeys.survey_sit_7])      +
    '&sit8='        + encodeURIComponent(dict[messageKeys.survey_sit_8])      +
    '&sit8T='       + encodeURIComponent(dict[messageKeys.survey_sit_8_text])
    ;
  }

  // Send to the server first and then to the watch.
  sendToServer(url, function receiveServerACK (err, status, response, responseText) {
    if (err || status !== 200) {
      log.info(err || status);

      // Send settings values to watch side even if cannot logging to the server.
      //saveConfigToWatch({});
  	  Pebble.sendAppMessage(dict, function(e) {
  	    console.log('Sent config data to Pebble');
  	  }, function(e) {
  	    console.log('Failed to send config data!');
  	    console.log(JSON.stringify(e));
  	  });
    } else {
      // Do not expect a response from the server?
      // TODO: server might always send a response? Or omit that for non-new user?
      if (response) {
        saveConfigToWatch(parseServerConfig(JSON.parse(response)));
      } else {
        saveConfigToWatch({});
      }
    }
  });
});

/** 
 * Server might response with new configuration settings. If so, we would apply 
 * them to both the phone app and the watch app.
 * @param {Objest} response
 */
function receiveServerConfigACK (err, status, response, responseText) {
  if (err || status !== 200) {
    log.info(err || status);
  } else if (!response) {
    // No new configuration from the server, simply send ACK to the watch.
    console.log('Server response with a null.');
    Pebble.sendAppMessage({ 'AppKeyServerReceived': 1 });
  } else {
    console.log('Server response.');
    // Parse the new configuration from the server, and then send to the watch.
    var settings = JSON.parse(response);

    settings = parseServerConfig(settings);

    // Essential to update settings on the watch side.
    settings.first_config = 0;

    Pebble.sendAppMessage(settings, function(e) {
      console.log('Sent config data to Pebble (' + JSON.stringify(settings) + ').');
    }, function(e) {
      console.log('Failed to send config data!');
      console.log(JSON.stringify(e));
    }); 
  }
}

/**
 * Parse the settings associated with Clay config.json and then send them to the watch.
 * @param {Object} settings The object containing the Clay configuration settings.
 */
function parseServerConfig (settings) {
  for (var key in settings) {
    if (key === 'messages') {
      log.info("SHOULD NOT HAVE KEY messages!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
      log.info(JSON.stringify(settings[key]));
      for (var m in settings[key]) {
        log.info(m + ":" + settings[key][m]);
      }
    } else if (key === 'random_messages') {
      log.info(JSON.stringify(settings[key]));
      //for (var id in settings[key]) {
      var randomMessages = settings[key];
      for (var i = 0; i < randomMessages.length; i++) {
        var messageID = 'random_message_' + i;
        var messageContent = randomMessages[i].id + ":" + randomMessages[i].content;
        log.info(messageID);
        log.info(messageContent);
        settings[messageID] = messageContent;
        clay.setSettings(messageID, messageContent);
      }
      delete settings[key];
    } else {
      log.info(key + ":" + settings[key]);
      clay.setSettings(key, settings[key]);
    }
  }
  
  settings.AppKeyServerReceived = 1;

  return settings;
}

/**
 * Send data to the server.
 * @param {String} route
 * @param {Function} callback Callback function will take 4 parameters: 
 *        err, status, response, responseText. It should handle err propoerly.
 */
//function sendToServer (route, responseHandler) {
function sendToServer (route, callback) {
  function send_request (url, type, callback) {
    var xhr = new XMLHttpRequest();
    xhr.onload = function () {
      callback(null, this.status, this.response, this.responseText);
    };
    xhr.onerror = function () {
      callback('Network error. Cannot send to ' + url);
    };
    xhr.open(type, url);
    xhr.send();
  }

  // TODO: The meaning of USE_OFFLINE seems to be opposite. And we might not need it.
  if (USE_OFFLINE) {
    log.info("sendToServer: " + route);
    send_request(SERVER + route, 'GET', callback);
    //send_request(SERVER + route, 'GET', function(err, status, response, responseText) {
    //  if (err || status !== 200) {
    //    log.info(err || status)
    //  } else {
    //    responseHandler(response);
    //  }
    //})
  } else {
    callback();
  }
}

function sendStepData(data, date) {
  log.debug('Uploading steps data...');
  // Convert to string
  var str = '' + data[0];
  for (var i = 1; i < data.length; i++) {
    str += ',' + data[i];
  }
  var url = '/steps' +
  '?date=' + date +
  '&data=' + str +
  '&watch=' + Pebble.getWatchToken();

  sendToServer(url, receiveServerConfigACK);
}

function sendLaunchExitData(configRequest, msgID, launchTime, exitTime, scoreDiff, score,
                            launchReason, exitReason, date) {
  var url;

  if (exitReason === undefined) {
    //log.debug('Uploading launch data only...')
    url = '/launch' + '?date=' + date + '&reason=' + launchReason +
      '&configrequest=' + configRequest + 
      '&msgid=' + msgID + 
      '&scorediff=' + scoreDiff +
      '&score=' + score + 
      '&watch=' + Pebble.getWatchToken();
  } else if (launchReason === undefined) {
    //log.debug('Uploading exit data only...')
    url = '/exit' + '?date=' + date + '&reason=' + exitReason +
      '&watch=' + Pebble.getWatchToken();
  } else {
    //log.debug('Uploading launch & exit data ...')
    url = '/launchexit' + '?date=' + date +
      '&launchtime=' + launchTime +
      '&exittime=' + exitTime +
      '&scorediff=' + scoreDiff +
      '&score=' + score + 
      '&launchreason=' + launchReason +
      '&exitreason=' + exitReason +
      '&msgid=' + msgID + 
      '&watch=' + Pebble.getWatchToken();
  }

  sendToServer(url, receiveServerConfigACK);
}

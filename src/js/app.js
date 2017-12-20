var packageinfo = require('../../package.json');

var Clay = require('pebble-clay');
var messageKeys = require('message_keys');
var clayConfig = require('./config.json');
var customClay = require('./custom-clay');
var clay = new Clay(clayConfig, customClay, { autoHandleEvents: false });

// Register custom Clay components.
clay.registerComponent(require('./pfbutton'));

var log = require('./logging');
log.set_level(3);

// URL at which to send data
var SERVER = 'http://pebble-fit.herokuapp.com';

// Local servers (use ifconfig to find out).
// var SERVER = 'http://10.30.202.74:3000';


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
  if (dict.payload.AppKeyDate !== undefined) {
    date = dict.payload.AppKeyDate;
    //log.debug('Date: ' + date + "; " + new Date(date*1000));
  }

  // Data related to step count.
  if (dict.payload.AppKeyStepsData !== undefined) {
    if (dict.payload.AppKeyStringData !== undefined) {
      var string = dict.payload.AppKeyStringData;

      // Do not send the data to the server if it is empty?
      if (!string) {
        // TODO: right now server handle this case by re-request data one minute earlier.
        // Should we just not sending the data to server in this case and wait for next time?
        return;
      }

      var url = '/steps' +
      '?date=' + date +
      '&data=' + string +
      '&watch=' + Pebble.getWatchToken();

      // Send step data to the server and expect server response with next timestamp
      // it expects.
      //sendToServer(url, receiveServerConfigACK); // Deprecated.
      sendToServer(url, function (err, status, response, responseText) {
        if (err || status !== 200) {
          log.info(err || status);
        } else {
          var settings = JSON.parse(response);

          if (settings.step_upload_time === undefined) {
            settings.step_upload_time = 0;
          } else {
            settings.step_upload_time = parseInt(settings.step_upload_time, 10);
          }

          // Flag for Enamel to save the settings and flag for data uploading to proceed.
          //settings.first_config = 0;
          settings.AppKeyServerReceived = 1;

          Pebble.sendAppMessage(settings, function(e) {
            console.log('Update upload time to Pebble (' + JSON.stringify(settings) + ').');
          }, function(e) {
            console.log('Failed to send upload time to Pebble! data = ' + JSON.stringify(e));
          });
        }
      });
    } else {
      console.log('Error: should not reach here since the array data format is deprecated!');
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
                       launchReason, exitReason, date, generateWeightData(dict),
                       generateAppConfig(dict, configRequest));
  }
});

var secondsToHHMM = function(seconds) {
  d = Number(seconds);
  var h = Math.floor(d / 3600) + "";
  var m = Math.floor(d % 3600 / 60) + "";

  if (h.length < 2) h = "0" + h;
  if (m.length < 2) m = "0" + m;

  return h + ":" + m;
}

var generateWeightData = function(dict) {
  var weightData = [];  
  var weightMsgID = dict.payload.AppKeyWeightMessageID;
  if (weightMsgID !== undefined) {
    weightData.push(weightMsgID);
    weightData.push(dict.payload.random_message_weights);
  }
  return weightData
}

var generateAppConfig = function(dict, configRequest) {
    var appConfig = {};
    if (configRequest == 1) {
      appConfig[messageKeys.time_zone] = new Date().getTimezoneOffset();
      appConfig[messageKeys.daily_start_time] = secondsToHHMM(dict.payload.daily_start_time);
      appConfig[messageKeys.daily_end_time] = secondsToHHMM(dict.payload.daily_end_time);
      appConfig[messageKeys.break_freq] = dict.payload.break_freq;
      appConfig[messageKeys.break_len] = dict.payload.break_len;
      appConfig[messageKeys.step_threshold] = dict.payload.step_threshold;
      appConfig[messageKeys.group] = dict.payload.group;
      appConfig[messageKeys.vibrate] = dict.payload.vibrate;
      appConfig[messageKeys.display_duration] = dict.payload.display_duration;
    }
    return appConfig;
}

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
  //log.info("webviewclosed.");
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
  // Do not want ot modify those settings that can only be set by the server.
  function saveConfigToWatch(settings) {
    settings.first_config       = dict[messageKeys.first_config];
    settings.is_consent         = dict[messageKeys.is_consent];
    settings.activate           = dict[messageKeys.activate];
    settings.vibrate            = dict[messageKeys.vibrate];
    settings.daily_start_time   = dict[messageKeys.daily_start_time];
    settings.daily_end_time     = dict[messageKeys.daily_end_time];
    settings.total_break        = dict[messageKeys.total_break];
    settings.display_duration   = dict[messageKeys.display_duration];

    // Uncomment these lines if want to change break_freq/break_len on watch side for
    // debugging purpose.
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
  var url = generateBaseConfigUrl(dict)

  if (dict[messageKeys.first_config]) {
    url = url +
    '&first='       + dict[messageKeys.first_config]                            +
    '&name='        + encodeURIComponent(dict[messageKeys.consent_name])        +
    '&email='       + encodeURIComponent(dict[messageKeys.consent_email])       +

    '&age='         + encodeURIComponent(dict[messageKeys.survey_age])          +
    '&gender='      + encodeURIComponent(dict[messageKeys.survey_gender])       +
    '&heightCM='    + encodeURIComponent(dict[messageKeys.survey_height_cm])    +
    '&heightFT='    + encodeURIComponent(dict[messageKeys.survey_height_ft])    +
    '&heightIN='    + encodeURIComponent(dict[messageKeys.survey_height_in])    +
    '&heightU='     + encodeURIComponent(dict[messageKeys.survey_height_unit])  +
    '&weight='      + encodeURIComponent(dict[messageKeys.survey_weight])       +
    '&weightU='     + encodeURIComponent(dict[messageKeys.survey_weight_unit])  +
    '&race='        + encodeURIComponent(dict[messageKeys.survey_race])         +
    '&school='      + encodeURIComponent(dict[messageKeys.survey_school])       +
    '&occupation='  + encodeURIComponent(dict[messageKeys.survey_occupation])   +
    '&deskwork='    + encodeURIComponent(dict[messageKeys.survey_deskwork])     +
    '&income='      + encodeURIComponent(dict[messageKeys.survey_income])       +
    '&country='     + encodeURIComponent(dict[messageKeys.survey_country])      +
    '&zipcode='     + encodeURIComponent(dict[messageKeys.survey_zipcode])      +

    '&sit1='        + encodeURIComponent(dict[messageKeys.survey_sit_1])        +
    '&sit2='        + encodeURIComponent(dict[messageKeys.survey_sit_2])        +
    '&sit3='        + encodeURIComponent(dict[messageKeys.survey_sit_3])        +
    '&sit4='        + encodeURIComponent(dict[messageKeys.survey_sit_4])        +
    '&sit5='        + encodeURIComponent(dict[messageKeys.survey_sit_5])        +
    '&sit6='        + encodeURIComponent(dict[messageKeys.survey_sit_6])        +
    '&sit7='        + encodeURIComponent(dict[messageKeys.survey_sit_7])        +
    '&sit8='        + encodeURIComponent(dict[messageKeys.survey_sit_8])        +
    '&sit8T='       + encodeURIComponent(dict[messageKeys.survey_sit_8_text])
    ;
  }

  // Send to the server first and then to the watch.
  sendToServer(url, function (err, status, response, responseText) {
    if (err || status !== 200) {
      log.info(err || status);

      // Send settings values to watch side even if cannot logging to the server.
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
        var s = JSON.parse(response);
        parseServerConfig(s);
        saveConfigToWatch(s);
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

    if (parseServerConfig(settings)) {
      // Essential to update settings on the watch side. Not need if there is only
      // 'step_upload_time' in the settings from the server.
      settings.first_config = 0;
    }

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
 * @return Boolean whether the settings contain more than just step_upload_time.
 */
function parseServerConfig (settings) {
  var res = false;

  if (settings.step_upload_time !== undefined) {
    settings.step_upload_time = parseInt(settings.step_upload_time, 10);
  }

  for (var key in settings) {
    if (key === 'messages') {
      log.info("SHOULD NOT HAVE KEY messages!");
    } else if (key === 'random_messages') {
      //log.info(JSON.stringify(settings[key]));
      //for (var id in settings[key]) {
      var randomMessages = settings[key];
      for (var i = 0; i < randomMessages.length; i++) {
        var messageID = 'random_message_' + i;
        var messageContent = randomMessages[i].id + ":" + randomMessages[i].content;
        //log.info(messageID);
        //log.info(messageContent);
        settings[messageID] = messageContent;
        clay.setSettings(messageID, messageContent);
      }
      delete settings[key];
      res = true;
    } else {
      //log.info(key + ":" + settings[key]);
      clay.setSettings(key, settings[key]);
      if (key !== 'step_upload_time') {
        res = true;
      }
    }
  }

  settings.AppKeyServerReceived = 1;

  return res;
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
  } else {
    callback();
  }
}

function sendLaunchExitData(configRequest, msgID, launchTime, exitTime, scoreDiff, score,
                            launchReason, exitReason, date, weightData, appConfig) {
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

  if (weightData.length > 0) {
    url = url + '&weightmsgid=' + weightData[0] + '&weightnew=' + weightData[1];
  }

    // send daily config
  if (configRequest == 1) {
    sendToServer(generateBaseConfigUrl(appConfig), function (err, status, response, responseText) {
      if (err || status !== 200) {
        log.info(err || status);
        log.info("Daily config failed to send");
      } else {
        log.info("Daily config sent");
      }
      sendToServer(url, receiveServerConfigACK);
    });
  } else {
    sendToServer(url, receiveServerConfigACK);
  }
}

function generateBaseConfigUrl(dict) {
  var date = Math.floor(new Date().getTime() / 1000);
  var url = '/config' + '?date=' + date +
    '&watch='       + Pebble.getWatchToken()                                    +
    '&timezone='    + encodeURIComponent(dict[messageKeys.time_zone])           +
    '&startTime='   + encodeURIComponent(dict[messageKeys.daily_start_time])    +
    '&endTime='     + encodeURIComponent(dict[messageKeys.daily_end_time])      +
    '&breakFreq='   + encodeURIComponent(dict[messageKeys.break_freq])          +
    '&breakLen='    + encodeURIComponent(dict[messageKeys.break_len])           +
    '&threshold='   + encodeURIComponent(dict[messageKeys.step_threshold])      +
    '&group='       + encodeURIComponent(dict[messageKeys.group])               +
    '&vibrate='     + encodeURIComponent(dict[messageKeys.vibrate])             +
    '&displayDuration=' + encodeURIComponent(dict[messageKeys.display_duration]);
  return url;
}

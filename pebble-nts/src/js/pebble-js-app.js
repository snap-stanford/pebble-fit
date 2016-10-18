// version as of Oct 5, 2015

/* >>>>> GLOBAL NOTES <<<<<<
* 1. If change server file structure, then must change the localStorage naming
* 2. For local variables that will NOT be transmitted to the server, they MUST
*   NOT have a prefix of 'wp-pebble' or else the transmit server code will send
*   those keys along, which would just pollute the name space, *very* annoying
*/

// NOTE : DEBUGGING
// 1. change the comm signals from acti, pinteract, and send_to_server
//
// 2.  changed all local storage key prefixs from 'pk' to 'wp'

/* >>>>> GLOBAL CONSTANTS <<<<<< */
var wpURL = 'http://wearpsych-test-dev.us-west-1.elasticbeanstalk.com';
var baseRoute = 'api/pebble/upload';
var servicePlatform = 'pebble';
var deviceType = 'pebblewatch';
var jsDataFormat = 2;
var jsVersion = 4;
var DEBUG_RESET_LOCAL_STORAGE = false;


/* >>>>> GLOBAL VARIABLES <<<<<< */
// TRANSMIT TO S3
// we create these so they can persist through calls to S3
var reqS3Array = [];
var keyS3Array = [];

// TRANSMIT General
// we create these so they can persist through calls to the general web
// Currently unused
var reqGenArray = [];
var keyGenArray = [];

/* >>>>>>>>> DEFINE SYSTEM CALLBACKS  <<<<<< */
Pebble.addEventListener("ready", function(e) {
  console.log('Starting Wearable Psych, send ready message to pebble');
  if(DEBUG_RESET_LOCAL_STORAGE){
    removeAllKeys();
    listAllKeys();
  }

  startPebbleWatchTransmission();

  // // check version
  // if(localStorage.getItem(WPKeyToLSKey('jsVersion')) != jsVersion.toString()){
  //   // if version changes, then delete everything and start over?
  //   // re-init
  //
  //   // finally, write the current version
  //   localStorage.setItem(WPKeyToLSKey('jsVersion'), jsVersion);
  // }
  // // if version changes, then delete everything and start over?
});


Pebble.addEventListener('appmessage', function(e) {
  // NOTE : Assume that all data is segregated into distinct types
  // 1. acti -> actigraphy
  // 2. pinteract -> patient interactions, ie: survey results
  // 3. push to server -> signal to push all data to the server

  touchAllKeys(); // VOODOO, no idea why this is needed, a bug in JS code?

  if(e.payload.AppKeyActiData != undefined){
    // if actigraphy data, then save to local storage
    saveNewToLocalStorage(e.payload.AppKeyActiData,'acti');
  }else if(e.payload.AppKeyHealthEventData != undefined){
    // if pinteract data, then save to local storage
    saveNewToLocalStorage(e.payload.AppKeyHealthEventData,'healthevent');
  }else if(e.payload.AppKeyPinteractData != undefined){
    // if pinteract data, then save to local storage
    saveNewToLocalStorage(e.payload.AppKeyPinteractData,'pinteract');
  }else if(e.payload.AppKeyPushToServer != undefined ){
    // If we want to push to server.
    attemptToTransmitAllKeysLS();
  }

});

// OPEN THE PATIENT REGISTRATION PAGE
Pebble.addEventListener('showConfiguration', function(e){
  Pebble.openURL(wpURL + '/users/pebble/register?pebble_account_token='
  + Pebble.getAccountToken());
  console.log('open the web config page');
})

// note, this is used if we need to save any data on the device
Pebble.addEventListener('webviewclosed', function(e) {
  // var config = JSON.parse(decodeURIComponent(e.response));
  // console.log(JSON.stringify(config));
  // // the parameters ONLY return IF they are validated by server
});


/* >>>>>>>>> DEFINE FUNCTIONALITY METHODS  <<<<<< */

function lEndianByteToTimestamp(byteArray){
  // note, this only takes the first 4 bytes of the array
  // so it is suitable for the arrangement of the data byte
  var value = 0;
  for(var i = 3; i >= 0; i--){
    value = (value*256) + byteArray[i];
  }
  return value;
}


function timestampToLEndianByte(timestamp,byteArray,startInd){
  // writes the timestamp into the given byteArray starting at startInd
  for( var i = 0; i< 4; i++){
    byteArray[i+startInd] = timestamp >> (8*i);
  }
}


function saveNewToLocalStorage(data,dataType){
  //console.log('made it to saveNewToLocalStorage: dataType: ' + dataType );
  var dataByteArray = new Uint8Array(data)
  var timestamp = lEndianByteToTimestamp(dataByteArray);
  // NOTE :
  //  LSKey -> the name of the key as stored in Local Storage on this device
  // NOTE: ALL keys for this app start with 'wp-', and we split using the '-'
  // !! HOWEVER, there are two types of keys,
  // PREFIX = 'wp-*' is a local storage key for pebble
  // PREFIX = 'wp-pebble' is a key for an item meant to be sent to S3 server
  var LSKey = 'wp-pebble-' + dataType + '-' + timestamp.toString();
  // get the watch info
  if(Pebble.getActiveWatchInfo) {
    var watch = Pebble.getActiveWatchInfo();
  } else {
    // Not available, handle gracefully
    var watch = {plaform:'aplite',model:'pebble_black',
      firmware:{major:2,minor:9,patch:1 }};
  }

  // create a JSON object with the meta data associated to it
  var LSitem = {
    jsVersion: jsVersion,
    jsDataFormat : jsDataFormat,
    servicePlatform: servicePlatform,
    pebbleAccountToken : Pebble.getAccountToken(),
    deviceType:deviceType,
    pebbleWatchToken : Pebble.getWatchToken(),
    watchPlatform: watch.platform,
    model: watch.model,
    firmware: (watch.firmware.major + '.'
      + watch.firmware.minor + '.'  + watch.firmware.patch),
    timestamp : timestamp,
    UTCoffset : (new Date().getTimezoneOffset()/60),
    dataType : dataType,
    data: data
  };
  console.log('LSitem start:' + JSON.stringify(LSitem));
  localStorage.setItem(LSKey, JSON.stringify(LSitem));
}

function attemptToTransmitAllKeysLS(){
  //console.log('made it to attemptToTransmitAllKeysLS');
  // Some console.log() commented out so Data not appear when pebble logging
  // to prevent the patientCode and data from being compromised

  // initialize all global variables
  var nKeys = localStorage.length;
  var hostUrl = wpURL;

  // reinitialize the arrays to zero
  reqS3Array = [];
  keyS3Array = [];
  requestPending = 0;

  /* CREATE THE KEY ARRAY TO BE SENT*/
  // NOTE : Must create keyArray because
  //   1. when remove keys, cause the length of localStorage to shrink
  //   2. certain keys may need to be excluded (ie: a unique patient password)

  keyS3Array = arrayOfAllS3WPKeys();
  // report the initial number of keys that need to be transmitted
  //console.log('keyArray.length : ' +  keyS3Array.length);

  /* ASYNC DATA POST REQUESTS, USING CLOSURES */
  // NOTE, only need to iterate over the keyArray
  console.log('keyS3Array.length: ' + keyS3Array.length)
  // If there are no keys to send, then tell the pebble app to go ahead and shutdown
  if(keyS3Array.length == 0){
    closePebbleWatchApp();
  // If there are keys, then try to transmit them all
  }else{
    for(var i = 0; i < keyS3Array.length; i++){
      // create the closure environment for each 'i' key index
      (function(i){
        // Declare all vars as local for async XMLHTTPRequest callback function
        // otherwise, the vars will change over the course of the loop, and
        // the removeKey will fail

        /* >>> DECLARE MAJOR VARS AS LOCAL : REQUIRED : DO NOT TOUCH <<< */
        // Could use 'key' as arg to closure function, but use i for illustration
        var key = keyS3Array[i];
        var dataJSONString = localStorage.getItem(key);

        // NOTE : These lines are from the old method of sending data in the url,
        // unusable now due to the size of the data being sent up exceeds the
        // 2048 character limit of url request strings
          // var dataArray = new Uint8Array( (localStorage.getItem(key)).split(',').map(Number));
          // var postUrl = hostUrl + '/' + baseRoute + '?' + encodeURIComponent(dataJSONString);

        var postUrl = hostUrl + '/' + baseRoute;
        console.log('postUrl: ' + postUrl); // COMMENT OUT

        // ++++++++++++++++++++

        reqS3Array[i] = new XMLHttpRequest();

        /* >>> DECLARE MAJOR VARS AS LOCAL : REQUIRED : DO NOT TOUCH <<< */
        reqS3Array[i].open("POST", postUrl, true);
        // log the post that is sent
        // console.log('posted Url: ' + postUrl);  // >> commented out to prevent data compromise
        reqS3Array[i].onreadystatechange = function(oEvent){
          // AND, since the lexical scope is within the overall function,
          //  'key' and 'req' is constant
          // gate that the request is finished and returned --> == 4
          if(reqS3Array[i].readyState == 4){
            // console.log('key :' + key); // >> commented out to prevent data compromise
            // if request finished (readyState==4), check if successful
            if( reqS3Array[i].status == 200 ){
              // remove the key that was successfully pushed to server
              localStorage.removeItem(key);

              console.log('Successful transmission '
                + reqS3Array[i].statusText + ' & ' + reqS3Array[i].responseText);
              requestPending = requestPending - 1; // decrement pending request count
            }else{
              // log errors
              console.log('Error: ' + reqS3Array[i].statusText + ' & '
                + reqS3Array[i].responseText);
            }
            // --> When completed should be 1 left (wp-jsVersion)
            console.log(' # of keys remain : ' + localStorage.length);
            // once all the requests have returned, signal the pebble to close
            if(requestPending == 0){
              closePebbleWatchApp();
            }
          }
        }
        requestPending = requestPending+ 1;
        // send the formatted XMLHttpRequest
        // reqS3Array[i].send();
        reqS3Array[i].setRequestHeader("Content-Type", "application/json");
        reqS3Array[i].send( dataJSONString );
        // ++++++++++++++++++++
        console.log('request ' + i + ' sent');
      })(i); // we make this a self executing function
    }
  }
}

// CONVIENENCE FUNCTIONS
function startPebbleWatchTransmission(){
  console.log('attempt to transmit to pebble');
  Pebble.sendAppMessage( { 'AppKeyJSReady': 0},
    function(e) {
      console.log('Successfully delivered message with transactionId='
        + e.data.transactionId);
    },
    function(e) {
      Pebble.sendAppMessage( { 'AppKeyJSReady': 0});
      console.log('Unable to deliver message with transactionId='
        + e.data.transactionId
        + ' Error is: ' + e.error.message);
  });
}

function closePebbleWatchApp(){
  Pebble.sendAppMessage( { 'AppKeySentToServer': 0},
    function(e) {
      console.log('Successfully delivered message with transactionId='
        + e.data.transactionId);
    },
    function(e) {
      Pebble.sendAppMessage( { 'AppKeySentToServer': 0});
      console.log('Unable to deliver message with transactionId='
        + e.data.transactionId
        + ' Error is: ' + e.error.message);
  });
}

function LSKeyToWPKey(key){
  return key.replace(/wp-/i,''); // remove the leading 'wp-' ONLY
}

function WPKeyToLSKey(key){
  return ('wp-' + key); // append 'wp-' tag to beginning
}

function isWPKey(key){
  return /wp-.*/g.test(key); // find if key has a leading 'wp-' tag
}

function isWPS3Key(key){
  // determine if a wp S3 key, by finding if a wp key, then testing
  // if it is S3
  return /wp-pebble.*/g.test(key);
}


function arrayOfAllWPKeys(){
  console.log('made it to arrayOfAllWPKeys');
  var key;
  var tmpKeyArray= [];
  var nKeys = localStorage.length;

  for(var i = 0; i < nKeys; i++){
    key = localStorage.key(i);
    if(isWPKey(key)){
      tmpKeyArray.push(key);
    }
  }
  // console.log('tmpKeyArray: ' + tmpKeyArray);
  return tmpKeyArray;
}


function arrayOfAllS3WPKeys(){
  console.log('made it to arrayOfAllS3WPKeys');
  var key;
  var tmpKeyArray= [];
  var nKeys = localStorage.length;

  for(var i = 0; i < nKeys; i++){
    key = localStorage.key(i);
    if(isWPS3Key(key)){
      tmpKeyArray.push(key);
    }
  }
  // console.log('tmpKeyArray: ' + tmpKeyArray);
  return tmpKeyArray;
}


function removeAllKeys(){
  console.log('made it to removeAllKeys');
  var key;
  var nKeys = localStorage.length;
  // console.log('remaining keys: ' + localStorage.length);

  for(var i = 0; i < nKeys; i++){
    key = localStorage.key(0)
    localStorage.removeItem(key);
    console.log('ORIGINAL nKeys: ' + nKeys + ', removed ' + key);
  }
}


function listAllKeys(){
  console.log('made it to listAllKeys');
  var key;
  var nKeys = localStorage.length;
  console.log('remaining keys: ' + localStorage.length);

  for(var i = 0; i < nKeys; i++){
    key = localStorage.key(i)
    console.log('key: '  + key);
  }
}


function touchAllKeys(){
  // NOTE: VOODOO, THIS IS VOODOO, DONT TOUCH EVER
  // no idea why this function is needed, but seems to do something important
  // to get the JS localStorage to recognize the local keys
  var key;
  var nKeys = localStorage.length;
  for(var i = 0; i < nKeys; i++){ key = localStorage.key(i) }
}


// EXTRA CODE
// function Uint8ToInt8(a) {
//  // http://blog.vjeux.com/2013/javascript/conversion-from-uint8-to-int8-x-24.html
//  return a << 24 >> 24;
// }

/*  */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* ++++++++++++++++++ NOTES FOR PEBBLE JS : START ++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */


// NOTES : START :: function attemptToTransmitAllKeysLS()

// if we just iterate i = 0:(nKeys-1), then there is a conflict IF
// (in the unlikely event) that a response returns before all requests
// are made, thereby deleting a key and shortening the localStorage array,
// hence causing the i var to go out of bounds and crash everything. SO
// we must just create an array of keys themselves, and only iterate over
// THAT to create the requests
// we need a way to cache all the keys names to be sent, and then,
// only after they have been sent, we remove the key and associated data from
// the the local storage

// we want to  create an XMLHttpRequest for each key in the
// localStorage and to have it handle asynchronously. Hence, the most
// logical approach is to create a closure for each localStorage key
// and have that environment be self executing.

// NOTE, onreadystatechange event stores a function that is called EACH
//   time the readyState changes
// NOTE, the status property holds the status of the XMLHttpRequest
// NOTE, remember that there are several response for the readyState
// readyState =>
//   0: request not initialized
//   1: server connection established
//   2: request received
//   3: processing request
//   4: request finished and response is ready
// SO, we need to add the function to .onreadystatechange but filter
//   that readyState == 4 and status == 200

// NOTES : END :: function attemptToTransmitAllKeysLS()


/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* ++++++++++++++++++ NOTES FOR PEBBLE JS :  END  ++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ EXTRA SOURCESFOR PEBBLE JS : START ++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

// Good for ACK/NACK sending of messages
// => https://github.com/pebble-examples/pebble-faces/blob/master/src/js/pebble-js-app.js

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ EXTRA SOURCESFOR PEBBLE JS:  END  +++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */


/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ DEPRECIATED CODE FOR PEBBLE JS: START +++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */


/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ DEPRECIATED CODE FOR PEBBLE JS: END +++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

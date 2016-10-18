// version as of Oct 5, 2015

/* >>>>> GLOBAL NOTES <<<<<<
* 1. If change server file structure, then must change the localStorage naming
*   conventions as they directly rflect the S3 server filename heirarchy
* 2. For local variables that will NOT be transmitted to the server, they MUST
*   NOT have a prefix of 'pk-pebble' or else the transmit server code will send
*   those keys along, which would just pollute the name space, *very* annoying
*
*/

/* >>>>> GLOBAL CONSTANTS <<<<<< */
var pkURL = 'http://projectkraepelin.org';
var baseRoute = 'dataflow/upload';
var servicePlatform = 'pebble';
var deviceType = 'pebblewatch';
var jsDataFormat = 2;
var jsVersion = 3;


/* >>>>> GLOBAL VARIABLES <<<<<< */
// TRANSMIT TO S3
// we create these so they can persist through calls to S3
var reqS3Array = [];
var keyS3Array = [];

// TRANSMIT General
// we create these so they can persist through calls to the general web
var reqGenArray = [];
var keyGenArray = [];

/* >>>>>>>>> DEFINE SYSTEM CALLBACKS  <<<<<< */
Pebble.addEventListener("ready", function(e) {
  console.log('Starting Project Kraepelin, send ready message to pebble');
  Pebble.sendAppMessage( { 'js_status': 1});

  // // check version
  // if(localStorage.getItem(PKKeyToLSKey('jsVersion')) != jsVersion.toString()){
  //   // if version changes, then delete everything and start over?
  //   // re-init
  //
  //   // finally, write the current version
  //   localStorage.setItem(PKKeyToLSKey('jsVersion'), jsVersion);
  // }
  // // if version changes, then delete everything and start over?
});


Pebble.addEventListener('appmessage', function(e) {
  // console.log('made it to appmessage listener');
  // console.log('localStorage.length: ' + localStorage.length);

  // assume that pinteract and acti might come across on the
  // same message
  touchAllKeys(); // voodoo, see if this works

  if(e.payload.acti != undefined){
    saveNewToLocalStorage(e.payload.acti,'acti');
  }
  if(e.payload.pinteract != undefined){
    saveNewToLocalStorage(e.payload.pinteract,'pinteract');
  }
  if(e.payload.config != undefined){
    saveNewToLocalStorage(e.payload.config,'config');
  }

  // console.log('got closer to transmit all');
  //   removeAllKeys(); // MUST COMMENT OUT
  // listAllKeys(); // MUST COMMENT OUT
  if(e.payload.pushtoserver != undefined ){
    attemptToTransmitAllKeysLS();
  }
  // we notify the pebble that we have stored the message
  Pebble.sendAppMessage( { 'js_status': 2});
});

Pebble.addEventListener('showConfiguration', function(e){
  Pebble.openURL(pkURL + '/' + 'pebble' + '/' + 'account'
  + '/' + 'main' + '?' + 'pebble_account_id' + '='
  + Pebble.getAccountToken());
  console.log('open the web config page');
})

Pebble.addEventListener('webviewclosed', function(e) {
  // var config = JSON.parse(decodeURIComponent(e.response));
  // console.log(JSON.stringify(config));
  // // the parameters ONLY return IF they are valid
  // // MAJOR BUG. that if return the page, since we are not checking the password
  // // the results will just overwrite these will null or blanks. Moreover,
  // // unless we make some control, the removeAllKeys() will delete everything
  // // even if we hit the back button
  //
  // // remove all keys to clear way for new patient
  // // However, I need a way to ensure that this is not triggered accidentially
  // //   removeAllKeys();
  // attemptToTransmitAllKeysLS();
  // console.log('add trial parameters to keys');
  // localStorage.setItem('trialCode', config.trialCode );
  // localStorage.setItem('trialPassword', config.trialPassword );
  // localStorage.setItem('patientCode', config.patientCode );
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

  // NOTE!! ALL keys for this app start with 'pk-', and we split using the '-'
  // var S3Key = 'pebble' + '/' + Pebble.getAccountToken() + '/' + 'pebblewatch'
  //   + '/' + dataType + '/' + timestamp.toString() + '-raw-'+dataType+'.dat';
  // var LSKey = PKKeyToLSKey(S3Key);
  var LSKey = 'pk-pebble-' + dataType + '-' + timestamp.toString();
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
  var hostUrl = pkURL;

  // reinitialize the arrays to zero
  reqS3Array = [];
  keyS3Array = [];
  requestPending = 0;

  /* CREATE THE KEY ARRAY TO BE SENT*/
  // NOTE : Must create keyArray because
  //   1. when remove keys, cause the length of localStorage to shrink
  //   2. certain keys need to be excluded (ie: trialCode)

  keyS3Array = arrayOfAllS3PKKeys();
  // Key the initial number of keys that need to be transmitted
  //console.log('keyArray.length : ' +  keyS3Array.length);

  /* ASYNC DATA POST REQUESTS, USING CLOSURES */
  // NOTE, only need to iterate over the keyArray
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
      // when want to change to a numerical array
      // var dataArray = new Uint8Array( (localStorage.getItem(key)).split(',').map(Number));

      var postUrl = hostUrl + '/' + baseRoute + '?' + encodeURIComponent(dataJSONString);
      // console.log('postUrl: ' + postUrl); // COMMENT OUT

      reqS3Array[i] = new XMLHttpRequest();
      /* >>> DECLARE MAJOR VARS AS LOCAL : REQUIRED : DO NOT TOUCH <<< */
      /* POST DATA IN THE URL THROUGH QUERYSTRING */
      reqS3Array[i].open("POST", postUrl, true);
      // log the post that is sent

      // console.log('posted Url: ' + postUrl);  // >> commented out to prevent data compromise
      reqS3Array[i].onreadystatechange = function(oEvent){
        // AND, since the lexical scope is within the overall function,
        //  'key' and 'req' is constant
        // gate that the request is finished and returned --> ==4
        if(reqS3Array[i].readyState == 4){
          // console.log('key :' + key); // >> commented out to prevent data compromise
          // if request finished (readyState==4), check if successful
          if( reqS3Array[i].status == 200 ){
            // remove the key that was successfully stored
            localStorage.removeItem(key);
            // --> When completed should be 3 left (patientCode,trialCode,trialPassword)
            console.log('Successful transmission');
            requestPending = requestPending -1; // decrement pending request count
          }else{
            // log errors
            console.log('Error: ' + reqS3Array[i].statusText + ' & ' + reqS3Array[i].responseText);
          }
          // --> When completed should be 1 left (pk-jsVersion)
          console.log(' # of keys remain : ' + localStorage.length);
          // once all the requests have returned, signal the pebble to close
          if(requestPending == 0){
            Pebble.sendAppMessage( { 'js_status': 3});
          }
        }
      }
      requestPending = requestPending+ 1;
      // send the formatted XMLHttpRequest
      reqS3Array[i].send();
      console.log('request ' + i + ' sent');
    })(i); // we make this a self executing function
  }
}


function sendDataInUrlToServer(postUrl){
  // this is a super basic function for sending just string data to the server
  // console.log('sendDataInUrlToServer , postUrl: ' + postUrl); // COMMENT OUT

  var req = new XMLHttpRequest();
  /* >>> DECLARE MAJOR VARS AS LOCAL : REQUIRED : DO NOT TOUCH <<< */

  /* POST DATA IN THE URL THROUGH QUERYSTRING */
  req.open("POST", postUrl, true);
  // log the post that is sent

  // console.log('posted Url: ' + postUrl);  // >> commented out to prevent data compromise
  req.onreadystatechange = function(oEvent){
    // AND, since the lexical scope is within the overall function,
    //  'key' and 'req' is constant

    // gate that the request is finished and returned --> ==4
    if(req.readyState == 4){
      // console.log('key :' + key); // >> commented out to prevent data compromise

      // if request finished (readyState==4), check if successful
      if( req.status == 200 ){
        // --> When completed should be 3 left (patientCode,trialCode,trialPassword)
        console.log('Successful transmission');
      }else{
        // log errors
        console.log('Error: ' + req.statusText + ' & ' + req.responseText);
      }
      // --> When completed should be 3 left (patientCode,trialCode,trialPassword)
      console.log('Success : # of keys remain : ' + localStorage.length);
    }
  }
  // send the formatted XMLHttpRequest
  req.send();
}

// CONVIENENCE FUNCTIONS

function LSKeyToPKKey(key){
  return key.replace(/pk-/i,''); // remove the leading 'pk-' ONLY
}

function PKKeyToLSKey(key){
  return ('pk-' + key); // append 'pk-' tag to beginning
}

function isPKKey(key){
  return /pk-.*/g.test(key); // find if key has a leading 'pk-' tag
}

function isPKS3Key(key){
  // determine if a PK S3 key, by finding if a PK key, then testing
  // if it is S3
  return /pk-pebble.*/g.test(key);
}


function arrayOfAllPKKeys(){
  console.log('made it to arrayOfAllPkKeys');
  var key;
  var tmpKeyArray= [];
  var nKeys = localStorage.length;

  for(var i = 0; i < nKeys; i++){
    key = localStorage.key(i);
    if(isPKKey(key)){
      tmpKeyArray.push(key);
    }
  }
  // console.log('tmpKeyArray: ' + tmpKeyArray);
  return tmpKeyArray;
}


function arrayOfAllS3PKKeys(){
  console.log('made it to arrayOfAllS3PKKeys');
  var key;
  var tmpKeyArray= [];
  var nKeys = localStorage.length;

  for(var i = 0; i < nKeys; i++){
    key = localStorage.key(i);
    if(isPKS3Key(key)){
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



//
// function Uint8ToInt8(a) {
//  // http://blog.vjeux.com/2013/javascript/conversion-from-uint8-to-int8-x-24.html
//  return a << 24 >> 24;
// }

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* ++++++++++++++++++ NOTES FOR PEBBLE JS : START ++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
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
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* ++++++++++++++++++ NOTES FOR PEBBLE JS :  END  ++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ EXTRA SOURCESFOR PEBBLE JS : START ++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

// Good for ACK/NACK sending of messages
// => https://github.com/pebble-examples/pebble-faces/blob/master/src/js/pebble-js-app.js

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ EXTRA SOURCESFOR PEBBLE JS:  END  +++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

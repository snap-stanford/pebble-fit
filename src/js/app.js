var DEBUG = false;
var server = 'http://192.168.1.5:3000'

function info(content) {
  console.log(content);
}

function debug(content) {
  if(DEBUG) info(content);
}

function ajax(url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

Pebble.addEventListener('ready', function() {
  info('PebbleKit JS ready! Version ');

  Pebble.sendAppMessage({'AppKeyJSReady': 0});
});



Pebble.addEventListener('appmessage', function(dict) {
  debug('Got appmessage: ' + JSON.stringify(dict.payload));

  if(dict.payload['AppKeyNumDataItems'] != undefined) {
    numItems = dict.payload['AppKeyNumDataItems'];
    debug('Number of items: ' + numItems);
  }

  if(dict.payload['AppKeyDate'] != undefined) {
    date = dict.payload['AppKeyDate'];
    debug('Start timestamp: ' + date);
  }

  if(dict.payload['AppKeyIndex'] != undefined) {
    var index = dict.payload['AppKeyIndex'];

    if(index == 0) {
      // New data set
      debug('Clearing data');
      lastData = [];
    }

    // Store this data item
    var value = dict.payload['AppKeyData'];
    lastData[index] = value;
    debug('Got item ' + index + ': ' + value);

    if(index == numItems - 1) {
      // Last item, transmit to server
      debug('Last item received. Uploading...');

      // Convert to string
      var str = '' + lastData[0];
      for(var i = 1; i < numItems; i++) {
        str += ',' + lastData[i];
      }
      info('Sending data string: ' + str);

      var url = server + '/collect'
      + '?date=' + date
      + '&data=' + str
      + '&watch=' + Pebble.getWatchToken();

      ajax(url, 'GET', function(responseText) {
        info('Server response: ' + responseText);
      });
    }
  }
});
// Minimal level based logging
var LOGGING_LEVEL = 2;

var info = function (content) {
  if (LOGGING_LEVEL > 2) { console.log(content); }
};

var debug = function (content) {
  if (LOGGING_LEVEL > 1) { console.log(content); }
};

var error = function (content) {
  if (LOGGING_LEVEL > 0) { console.log(content); }
};

var set_level = function (level) {
  LOGGING_LEVEL = level;
};

module.exports = {
  debug: debug,
  info: info,
  error: error,
  set_level: set_level
}

var Reference = require('../models/reference');

/* 
 * Get the typical score of the specific user with the watchID watch.
 */
exports.getUser = function (watch, numBreak, next) {
  Reference.findOne({ 'watch': watch }).
    select('scores').
    lean().
    exec(next);
};

/* 
 * Get the average typical score across all users. 
 * It is represented with a special watchID "all".
 */
exports.getAll = function (numBreak, next) {
  Reference.findOne({ 'watch': 'all' }).
    select('scores').
    lean().
    exec(next);
}

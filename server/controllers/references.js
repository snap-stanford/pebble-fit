var Reference = require('../models/reference');

/*
 * Create a reference score for the given user with fake data (for DEBUG purpose).
 */
exports.save = function (watch, next) {
  var obj = { 'watch': watch, 'average': [1,1,2,2,3,3,4,4,5,5,6,6], 'best': 10 };

  var reference = new Reference(obj);
  reference.save(next);
};

/*
 * Update the reference score for the given user.
 */
exports.update = function (watch, average, best, next) {
  Reference.update({ 'watch': watch }, 
    { $set: { 'average':    average,
              'best':       best
    } }, 
    { upsert: true },
    next
  );
};

/* 
 * Get the typical score of the specific user with the watchID watch.
 */
exports.getUser = function (watch, numBreak, next) {
  Reference.findOne({ 'watch': watch }).
    select('average best').
    lean().
    exec(next);
};

/* 
 * Get the average typical score across all users. 
 * It is represented with a special watchID "all".
 */
exports.getAll = function (numBreak, next) {
  Reference.findOne({ 'watch': 'all' }).
    select('average best').
    lean().
    exec(next);
}

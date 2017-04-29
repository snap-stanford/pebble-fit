var Reference = require('../models/reference');

/*
 * Create a reference score for the given user with fake data (for DEBUG purpose).
 */
exports.save = function (watch, next) {
  var obj = { 'watch': watch, 
              'average': [20,30,20,30,20,30,20,30,20,30,20,30], 
              'count': 12,
              'best': 5 };

  var reference = new Reference(obj);
  reference.save(next);
};

/*
 * Update the reference score for the given user.
 */
exports.update = function (watch, average, best, next) {
  Reference.update({ 'watch': watch }, 
    { $set: { 'average':        average,
              'count':          average.length,
              'best':           best
    } }, 
    { upsert: true,
      setDefaultsOnInsert: true },
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
  Reference.findOne({ 'watch': 'median_all' }).
    select('average best').
    lean().
    exec(next);
}

var Group = require('../models/group');
var _ = require('lodash');
var moment = require('moment');

exports.getConfigFile = function (name, date, force, next) {
  //Group.findOne({ 'name': name }).
  //  where('configUpdatedAt').gt(date).
        //      select('file').
  //  exec(next);
    
    /*
  Group.findOne({ 'name': name }).
    exec(function (err, group) {
      if (err) return next(err);

      if (!group.configUpdatedAt) {
        // Attach a timestamp to the group if not already existed.
        group.configUpdatedAt = new Date();

        group.save(function (err) {
          if (err) return next(err); 

          next(null, group);
        });
      } else {
        console.log(date);
        console.log(group.configUpdatedAt);
        if (force || group.configUpdatedAt > date) {
          next(err, group);
        } else {
          next(err, null);
        }
      }
    });
*/

  // TODO: debugging force to return config file always
  Group.findOne({ 'name': name }).
    select('file').
    exec(next);
};

exports.random_pick = function (next) {
  Group.aggregate([{ $sample: { size: 1 } }]).
                project('name').
    exec(next);
};

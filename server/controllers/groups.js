var Group = require('../models/group');
var _ = require('lodash');
var moment = require('moment');

exports.findGroup = function (name, next) {
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
        next(null, group);
      }
    });
};

exports.random_pick = function (next) {
  Group.aggregate([{ $sample: { size: 1 } }]).
    project('name').
    exec(next);
};


/**
 * @deprecated. 
 */
exports.getConfigFile = function (name, date, force, next) {
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
          next(err, group, true);
        } else {
          next(err, null);
        }
      }
    });

  // FIXME force to return config file always since we want to send new random messages.
  //Group.findOne({ 'name': name }).
  //  select('file').
  //  exec(next);
};


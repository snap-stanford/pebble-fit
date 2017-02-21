var Group = require('../models/groups');
var _ = require('lodash');
var moment = require('moment');

exports.getConfigFile = function (name, date, next) {
  //Group.findOne({ 'name': group }).
  //  where('configUpdatedAt').gt(date).
	//	select('file').
  //  exec(next);
    
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
        next(err, group);
      }
    });

  // TODO: debugging force to return config file always
  //Group.findOne({ 'name': group }).
	//	select('file').
  //  exec(next);
};

exports.random_pick = function (next) {
  Group.aggregate([{ $sample: { size: 1 } }]).
		project('name').
    exec(next);
};

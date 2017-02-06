var Group = require('../models/groups');
var _ = require('lodash');
var moment = require('moment');

exports.getConfigFile = function (group, date, next) {
  Group.findOne({ 'name': group }).
    where('configUpdatedAt').gt(date).
		select('file').
    exec(next);
};

// TODO: this might not be the best way to randomly pick a group. It seems to be
// not random.
exports.random_pick = function (next) {
  Group.aggregate([{ $sample: { size: 1 } }]).
		project('name').
    exec(next);
};

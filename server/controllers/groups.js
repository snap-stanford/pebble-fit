var Group = require('../models/groups');
var _ = require('lodash');
var moment = require('moment');

exports.getConfigFile = function (group, date, next) {
  Group.findOne({ 'name': group }).
    where('configUpdatedAt').gt(date).
		select('file').
    exec(next);
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

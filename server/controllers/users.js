var User = require('../models/users');
var _ = require('lodash');
var moment = require('moment');
var groups = require('./groups');

var save = function (watchToken, data, next) {
  console.log("Creating new user: " + watchToken);
  var obj = {watch: watchToken};
  groups.random_pick(function (err, group) {
    console.log("random_pick callback: err=" + err +"; group="+group[0].name);
    obj.group = group[0].name;

    //TODO: debugging force to be real_time_random.
    obj.group = "real_time_random";

    if (data) obj.data = data;
    var user = new User(obj);
    user.save(next);
  });
  console.log("Creating new user. ");
};
exports.save = save;

exports.getConfigFile = function (watchToken, next) {
  User.findOne({ 'watch': watchToken }).
    select('group configUpdatedAt').
    lean().
    exec(function (err, user) {
      if (err) return next(err);
      if (!user) {
	save(watchToken, null, function (err, user) {
	  console.log("save callback");
	  if (err) return next(err);
	  updateConfig(user, next);
	});
      } else {
	updateConfig(user, next);
      }
    });
};

var updateConfig = function (user, next) {
  //console.log("in updateConfig: user = "user.); console.log(user);
  groups.getConfigFile(user.group, user.configUpdatedAt, function (err, group) {
    console.log("group = " + group);
    if (err) return next(err);
    if (!group) return next(null, null);
    next(null, group.file);
  });
};

var User = require('../models/user');
var _ = require('lodash');
var moment = require('moment');
var groups = require('./groups');
var messages = require('./messages')
var references = require('./references')

// Since config files are updated by deployment, we can read it once at startup.
var configs = {
  'passive_tracking': require('../config/passive_tracking'),
  'daily_message': require('../config/daily_message'),
  'real_time_random': require('../config/real_time_random')
};

exports.save = function (watch, data, next) {
  console.log("Creating new user: " + watch);
  var obj = {watch: watch};
  groups.random_pick(function (err, group) {
    console.log("random_pick callback: err=" + err +"; group="+group[0].name);
    obj.group = group[0].name;

    //TODO: debugging force to be real_time_random.
    obj.group = 'real_time_random';

    if (data) obj.data = data;
    var user = new User(obj);
    user.save(next);
  });
  console.log("Creating new user. ");
};

/**
 * Get the new configuration settings for a given user. The settings might include general
 * settings, new random messages, and/or the reference scores.
 */
exports.getConfig = function (watch, next) {

  var addRefScores = function (watch, newConfig, next) {
    console.log("in addRefScores, watch=" + watch);
    references.getUser(watch, null, function (err, reference) { // TODO: numBreak not used
      if (err) return next(err);

      newConfig['ref_scores'] = reference.scores.join(','); // Convert to string

      console.log('AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA');
      console.log(newConfig);
      console.log('AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA');

      next(null, newConfig);
    });
  };

  checkUpdate(watch, function (err, groupName, isNewConfig) {
    //console.log("in checkUpdate callback");
    if (err) return next(err);
    
    var newConfig = {};
    if (isNewConfig) { 
      // New update available. Read the new configuration.
      newConfig = configs[groupName];
    } 


    // Add random messages if neccessary.
    //if (config.hasOwnProperty('random_messages')) {
    if (groupName === 'real_time_random') {
      var messagesCount = configs['real_time_random']['random_messages'];
      //newConfig['random_messages'] = messages.getRandomMessages(messagesCount); // FIXME
      newConfig['random_messages'] = messages.getRandomMessages(2);

      // Record the random messages.
      messages.save(watch, newConfig['random_messages'], function (err) {
        if (err) return next(err);
        
        addRefScores(watch, newConfig, next); 
      });   
    } else {
      addRefScores(watch, newConfig, next); 
    }
  });
};

/**
 * Check if there is a new configuration update for a specific user.
 */
//var updateConfig = function (watch, next) {
//  console.log("in checkUpdate: watch = "watch);
//
//  groups.findGroup(user.group, function (err, group) {
//    console.log("group = " + group);
//
//    if (err) return next(err);
//
//    if (!group) return next(null, null); // TODO: or assign a new group to user.
//
//    next(null, group);
//  });
//};

/**
 * Fetch user and group info from the DB and compare their configUpdatedAt timestamp. 
 * If group's configuration is newer, there is new config available.
 * Callback: function (err, groupName, isNewConfig)
 */
var checkUpdate = function (watch, next) {

  var updateConfig = function (user, force, next) {
    groups.findGroup(user.group, function (err, group) {
      console.log("group = " + group);
      if (err) return next(err);
  
      if (!group) return next(null, null); // TODO: or assign a new group to user.
  
      if (force || group.configUpdatedAt > user.configUpdatedAt) {
        next(null, group.name, true);
      } else {
        next(null, group.name, false);
      }
    });
    //groups.getConfigFile(user.group, user.configUpdatedAt, force, function (err, group) {
    //  console.log("group = " + group);
    //  if (err) return next(err);
    //  if (!group) return next(null, null);
    //  next(null, group.file);
    //});
  };

  User.findOne({ 'watch': watch }).
    select('group configUpdatedAt').
    lean().
    exec(function (err, user) {
      if (err) return next(err);

      if (!user) {
        // If the user does not exist, create a new entry.
	save(watch, null, function (err, user) {
	  console.log("save callback");

	  if (err) return next(err);

	  updateConfig(user, true, next);
	});
      } else {
	updateConfig(user, false, next);
      }
    });
};

/*
 * Fetch configuration settings from a given file, messages.json files, and the reference
 * collection in the DB.
 */
var fetchConfig = function (req, res, file, next) {
  // Read the corresponding configuration file.
  fs.readFile(configDir + file, 'utf8', function (err, data) {
    if (err) throw err;

    // Read the general configuration settings.
    var config = JSON.parse(data);

    // Read the random messages if neccessary.
    // FIXME: should generate new random messages even config not change.
    if (config.hasOwnProperty('random_messages')) {
      var messagesCount = config['random_messages'];
      //config['random_messages'] = messages.getRandomMessages(messagesCount);
      config['random_messages'] = messages.getRandomMessages(2);
    }

    reference.getUser(req.watch, null, function () { // TODO: numBreak not used
      // Record the random messages before sending.
      messages.save(req.query.watch, config['random_messages'], function (err) {
        if (err) return next(err);

        // Finally, save the launch event and send the new config to the user.
        saveEvent(req, res, config, next);
      });   
    }); 
  });
}




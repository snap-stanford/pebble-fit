module.exports = function(minified) {
  var clayConfig = this;
  //var $ = minified.$;
  //var _ = minified._;
  //var HTML = minified.HTML;

  function changeEnableApp() {
    if (this.get()) {
      clayConfig.getItemByMessageKey('sleep_minutes').enable();
      clayConfig.getItemByMessageKey('step_threshold').enable();
      clayConfig.getItemByMessageKey('daily_start_time').enable();
      clayConfig.getItemByMessageKey('daily_end_time').enable();
      clayConfig.getItemByMessageKey('display_duration').enable();
      clayConfig.getItemById('optin-text').hide();
    } else {
      clayConfig.getItemByMessageKey('sleep_minutes').disable();
      clayConfig.getItemByMessageKey('step_threshold').disable();
      clayConfig.getItemByMessageKey('daily_start_time').disable();
      clayConfig.getItemByMessageKey('daily_end_time').disable();
      clayConfig.getItemByMessageKey('display_duration').disable();
      clayConfig.getItemById('optin-text').show();
    }
    //clayConfig.build();
  }

  clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
    var optinToggle = clayConfig.getItemByMessageKey('optin');
    optinToggle.on('change', changeEnableApp);
    changeEnableApp.call(optinToggle);
  });

};

module.exports = function(minified) {
  var clayConfig = this;
  var _ = minified._;
  var $ = minified.$;
  var HTML = minified.HTML;

  function changeEnableApp() {
    if (this.get()) {
      clayConfig.getItemByMessageKey('sleep_minutes').enable();
      clayConfig.getItemByMessageKey('step_threshold').enable();
      clayConfig.getItemByMessageKey('daily_start_time').enable();
      clayConfig.getItemByMessageKey('daily_end_time').enable();
      clayConfig.getItemById('optin-text').hide();
    } else {
      clayConfig.getItemByMessageKey('sleep_minutes').disable();
      clayConfig.getItemByMessageKey('step_threshold').disable();
      clayConfig.getItemByMessageKey('daily_start_time').disable();
      clayConfig.getItemByMessageKey('daily_end_time').disable();
      clayConfig.getItemById('optin-text').show();
    }
  }

  clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
    var enableToggle = clayConfig.getItemByMessageKey('optin');
    changeEnableApp.call(enableToggle);
    enableToggle.on('change', changeEnableApp);
  });

};

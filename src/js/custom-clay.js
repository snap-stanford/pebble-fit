module.exports = function(minified) {
  var clayConfig = this;
  var $ = minified.$;
  var _ = minified._;
  var HTML = minified.HTML;

  var isEligible = false;
  var eligible_section = ['eligible_heading', 'eligible_text', 'eligible_button',
    'eligible_1', 'eligible_2', 'eligible_3', 'eligible_8', 
    'eligible_4', 'eligible_5', 'eligible_6', 'eligible_7'];
  var eligible_result_section = ['eligible_result_text', 'eligible_result_button'];
  var config_section = ['settings', 'optin', 'vibrate', 
    'sleep_minutes', 
    'step_threshold', 'daily_start_time', 'daily_end_time', 
    'display_duration', 'optin_text'];
  var sub_config_section = ['vibrate', 'sleep_minutes', 
    'step_threshold', 'daily_start_time', 'daily_end_time', 
    'display_duration', 'optin_text'];


  /* Hide/Show the configuration section, i.e. getItemById('config_section').hide()
   * or getItemById('config_section').show(). We have do it for each element since
   * from Clay GitHub page: "currently no way to disable or hide an entire section. 
   * You must disable/hide each item in the section to achieve this effect."
   */
  function showEligibleSection() {
    eligible_section.forEach(function(c) { clayConfig.getItemById(c).show(); });
  }
  function hideEligibleSection() {
    eligible_section.forEach(function(c) { clayConfig.getItemById(c).hide(); });
  }

  function showWarningText() {
    // TODO: testing
    //clayConfig.getItemById('eligible_warn_text').set('<button type="button" \
    //  id="test_yes" style="width:20px;height:40px;display:inline">Yes</button><button \
    //  type="button" id="test_no" style="width:20px;height:40px;display:inline">No</button>');
    clayConfig.getItemById('eligible_warn_text').set('<button style="display:inline">AAA</button><button>BBB</button>');
    clayConfig.getItemById('test').set('<input type="button" value="Test">');
    clayConfig.getItemById('eligible_warn_text').show();
  }
  function hideWarningText() {
    clayConfig.getItemById('eligible_warn_text').hide();
  }

  function showEligibleResultSection() {
    eligible_result_section.forEach(function(c) { clayConfig.getItemById(c).show(); });
  }
  function hideEligibleResultSection() {
    eligible_result_section.forEach(function(c) { clayConfig.getItemById(c).hide(); });
  }

  function showConfigSection() {
    config_section.forEach(function(c) { clayConfig.getItemById(c).show(); });
  }
  function hideConfigSection() {
    config_section.forEach(function(c) { clayConfig.getItemById(c).hide(); });
  }
  
  /* Check if the user is eligible to participate.   
   * Participants are eligible if they answer yes to questions 1-2 and no to questions 3-9
   */
  function eligibleButtonClick() {
    console.log('eligibleButtonClick');
    if (clayConfig.getItemById('eligible_3').get() === 'null' ||
        //clayConfig.getItemById('eligible_4').get() === 'null' ||
        //clayConfig.getItemById('eligible_5').get() === 'null' ||
        //clayConfig.getItemById('eligible_6').get() === 'null' ||
        //clayConfig.getItemById('eligible_7').get() === 'null' ||
        clayConfig.getItemById('eligible_8').get() === 'null') {
      showWarningText();
    } else {
      if (clayConfig.getItemById('eligible_1').get()  === true &&
               clayConfig.getItemById('eligible_2').get()  === true &&
               !clayConfig.getItemById('eligible_3').get() === false &&
               //!clayConfig.getItemById('eligible_4').get() === false &&
               //!clayConfig.getItemById('eligible_5').get() === false &&
               //!clayConfig.getItemById('eligible_6').get() === false &&
               //!clayConfig.getItemById('eligible_7').get() === false &&
               !clayConfig.getItemById('eligible_8').get() === false) {
        isEligible = true;
        clayConfig.getItemById('eligible_result_text').set('You are eligible to join the study. Tap the button below to begin the consent process.');
        clayConfig.getItemById('eligible_result_button').set('Start Consent');
      } else {
        isEligible = false;
        clayConfig.getItemById('eligible_result_text').set('Sorry, you are not eligible to join the study. Tap the button below to exit the app.');
        clayConfig.getItemById('eligible_result_button').set('Exit');
      }
      hideWarningText();
      hideEligibleSection();
      showEligibleResultSection();
    }
  }

  function eligibleResultButtonClick() {
    if (isEligible) {
      hideEligibleResultSection();
      showConfigSection();
    } else {
      exit();
    }
  }

  function changeEnableApp() {
    if (this.get()) {
      clayConfig.getItemByMessageKey('vibrate').enable();
      clayConfig.getItemByMessageKey('sleep_minutes').enable();
      clayConfig.getItemByMessageKey('step_threshold').enable();
      clayConfig.getItemByMessageKey('daily_start_time').enable();
      clayConfig.getItemByMessageKey('daily_end_time').enable();
      clayConfig.getItemByMessageKey('display_duration').enable();
      clayConfig.getItemById('optin_text').hide();
    } else {
      clayConfig.getItemByMessageKey('vibrate').disable();
      clayConfig.getItemByMessageKey('sleep_minutes').disable();
      clayConfig.getItemByMessageKey('step_threshold').disable();
      clayConfig.getItemByMessageKey('daily_start_time').disable();
      clayConfig.getItemByMessageKey('daily_end_time').disable();
      clayConfig.getItemByMessageKey('display_duration').disable();
      clayConfig.getItemById('optin_text').show();
    }
    //clayConfig.build();
  }

  /* Define actions once the Clay page is built. */
  clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
    var optinToggle = clayConfig.getItemByMessageKey('optin');

    // Register event callbacks
    optinToggle.on('change', changeEnableApp);
    clayConfig.getItemById('eligible_button').on('click', eligibleButtonClick);
    clayConfig.getItemById('eligible_result_button').on('click', eligibleResultButtonClick);

    // Show/Hide components accordinglly
    // TODO: hiding these for now
    clayConfig.getItemById('eligible_4').hide();
    clayConfig.getItemById('eligible_5').hide();
    clayConfig.getItemById('eligible_6').hide();
    clayConfig.getItemById('eligible_7').hide();

    clayConfig.getItemById('submit').hide();
    hideWarningText();
    hideEligibleResultSection();
    if (!isEligible) { 
      hideConfigSection();
    } else {
      hideEligibleSection();
    }

    changeEnableApp.call(optinToggle);
  });

};

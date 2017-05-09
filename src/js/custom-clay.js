var _clayConfig;

module.exports = function (minified) {
  var clayConfig = this;
  _clayConfig = clayConfig;

  var $ = minified.$;
  var _ = minified._;
  var HTML = minified.HTML;

  var isEligible = false;
  var isConsent = false;
  
  // List IDs of components in each section.
  var eligible_section = ['eligible_heading', 'eligible_text', 'eligible_button',
    'eligible_1', 'eligible_2', 'eligible_3', 'eligible_4', 'eligible_5', 
    'eligible_6', 'eligible_7', 'eligible_8'
  ];
  var pfbuttons = ['pfbutton_deprecated'];
  var eligible_result_section = ['eligible_result_text', 'consent_button_start'];
  //var consent_start_section = ['consent_heading_start', 'consent_text_start', 
  //  'consent_button_start'
  //];
  var consent_section = [
    'consent_heading_top', 'consent_text_top', //'consent_icon_top', 
    'consent_heading_bottom', 'consent_text_bottom', //'consent_icon_bottom', 
    'consent_button_next'
  ];
  var consent_review_section = ['consent_review_heading', 'consent_review_text', 
    'consent_review_heading_form', 'consent_review_approve_text', 'consent_review_expire_text',
    'consent_name', 'consent_email',
    'consent_review_text_form', 'consent_button_agree', 'consent_button_disagree'
  ];
  var consent_result_section = ['consent_result_text', 'consent_result_button']; 
  var survey_section = ['survey_heading_0', 'survey_text_0', 'survey_heading_1', 'survey_text_1',
    'survey_heading_2', 'survey_text_2', 'survey_text_3',
    'survey_age_text', 'survey_age', 'survey_gender', 'survey_height', 'survey_height_unit', 
    'survey_weight', 'survey_weight_unit', 'survey_race', 'survey_school', 
    'survey_occupation_text', 'survey_occupation', 'survey_deskwork', 'survey_income', 
    'survey_country_text', 'survey_country', 'survey_zipcode_text', 'survey_zipcode',
    'survey_sit_1', 'survey_sit_2', 'survey_sit_3', 'survey_sit_4', 'survey_sit_5', 
    'survey_sit_6', 'survey_sit_7', 'survey_sit_8', 'survey_sit_8_text'
  ];
  var config_section = ['settings',  'vibrate', 
    //'break_freq', 'break_len', 'step_threshold',
    //'activate', 'dynamic_wakeup', 'sliding_window', 'display_duration',
    'daily_start_time', 'daily_end_time', 'config_summary', 
    'version', 'watchtoken'
  ];
  // Elements in this section will be enabled/disabled by the activate button.
  var sub_config_section = ['vibrate', 'break_freq', 'break_len',
    'daily_start_time', 'daily_end_time', 'config_summary'
  ];

  // Components that should always be hidden.
  var disabled_components = [//'step_threshold', 
    'dynamic_wakeup', 'sliding_window', 'display_duration'];
  var hidden_components = [ 'activate', 
    'is_consent', 'first_config', 'config_update_interval', 'time_warn_text',
    'message_summary', 'message_pass', 'message_fail', 
    'total_break', 'group', 'score_p_average', 'score_p_best', 'score_p_count',
    'score_a_average', 'score_a_best', 'score_a_count', 'time_zone',
    'random_message_0', 'random_message_1', 'random_message_2',  'random_message_3', 
    'random_message_4', 'random_message_5', 'random_message_6',  'random_message_7', 
    'random_message_8', 'random_message_9', 'random_message_10', 'random_message_11',
    'survey_age2', 'survey_age3',
    'break_freq', 'break_len', 'step_threshold', // Invisible since user cannot modify them.
    'dynamic_wakeup', 'sliding_window', 'display_duration', 
    'consent_icon_top', 'consent_icon_bottom'];

  // Text to be displayed on each of the content pages. The idea is to use the same 
  // components, but change the text displayed for them. 
  // Must make sure :w
  // consent_heading and consent_text arrays have the same length.
  // Note that the texts of the first page is already supplied in the config.json file.
  var consentPageIndex = 0;
  var consent_heading = ["Privacy", "Data Use", "Time Commitment", 
    "Withdrawing", "Study Surveys", "Notifications", "Study Activities",
    "Risks and Potential Benefits"
  ];
  var consent_text = [
    "Your data will be encrypted and sent to a secure database. If you choose to provide us with you name and email, which is not mandatory, this information will be stored separately within our database to prevent you from being identifiable.",
    "Your coded data (without your name) will be used for research by Stanford University and our research partners.",
    "Our study will last up to two years and you can be part of the study as long as you like. It is useful for us even if you only participate in our study for as little as 1 week.",
    "You can withdraw your consent and stop participating in this study at any time by deleting the app from your smart watch.",
    "At the beginning of the study we will ask you some questions about you, your current physical activity, and lifestyle. These should only take about 15 minutes. We may also ask you some additional questions at other times during the study, but you are never obligated to answer.",
    "Throughout the day, you might be sent notifications through your smartwatch to encourage you to get up and move. Sometimes the notifications will be written text; other times it may be vibrations. You will always have the ability to personalize your app settings to increase or decrease the frequency of notifications.",
    "The physical activities you are encouraged to perform in this study will not be very strenuous. For example, you may be encouraged to add light and short bouts of walking to your day.",
    "As with any increase in walking activity, there are small risks of temporary and minor injury, such as muscle fatigue and soreness. It is possible that you will benefit physically and emotionally from increasing your physical activity levels — <strong>but we cannot guarantee this study will have any benefit to you.</strong>"
  ];

  /* Hide/Show various configuration sections, i.e. getItemById('config_section').hide()
   * or getItemById('config_section').show(). We have do it for each element since
   * from Clay GitHub page: "currently no way to disable or hide an entire section. 
   * You must disable/hide each item in the section to achieve this effect."
   * WARN: this function is error prone. Put this function at the end of event handler
   * for safety.
   */
  function showSection (section) {
    section.forEach(function (c) { clayConfig.getItemById(c).show(); });
  }
  function hideSection (section) {
    section.forEach(function (c) { clayConfig.getItemById(c).hide(); });
  }

  /* Show/Hide the configuration page: including config_section, the submit button, 
   * and the consent form button. */
  function showConfigSection () {
    clayConfig.getItemById('submit').show();

    showSection(config_section);
    
    //if (showConsentButton) {
    //  clayConfig.getItemById('view_consent_button').show();
    //}
  }
  function hideConfigSection () {
    clayConfig.getItemById('view_consent_button').hide();

    hideSection(config_section);

    clayConfig.getItemById('submit').hide();
  }

  /* Show the warnning text for the eligibility page if required field is answered. */
  function showWarningText () {
    //clayConfig.getItemById('eligible_warn_text').set(clayConfig.getItemById('eligible_8').$element.get('ans') === 'false');
    clayConfig.getItemById('eligible_warn_text').show();
  }
  function hideWarningText () {
    clayConfig.getItemById('eligible_warn_text').hide();
  }

  /* Hide all the pfbutton (yes-no buttons) components. */
  function hidePFButtons () {
    var yesButtons = $('.yes');
    for (i = 0; i < yesButtons.length; i++) {
      yesButtons[i].style.display = 'none';
    }
    var noButtons = $('.no');
    for (i = 0; i < noButtons.length; i++) {
      noButtons[i].style.display = 'none';
    }
  }

  /**
   * Check if the user is eligible to participate.
   * Participants are eligible if they answer yes to questions 1-2 and no to questions 3-9.
   */
  function eligibleButtonClick () {
    if (clayConfig.getItemById('eligible_1').get() === undefined ||
        clayConfig.getItemById('eligible_2').get() === undefined ||
        clayConfig.getItemById('eligible_3').get() === undefined ||
        clayConfig.getItemById('eligible_4').get() === undefined ||
        clayConfig.getItemById('eligible_5').get() === undefined ||
        clayConfig.getItemById('eligible_6').get() === undefined ||
        clayConfig.getItemById('eligible_7').get() === undefined ||
        //clayConfig.getItemById('eligible_8').$element.get('ans') === undefined)
        clayConfig.getItemById('eligible_8').get() === undefined) {
      showWarningText();
    } else {
      hidePFButtons();
      hideWarningText();
      hideSection(eligible_section);
      showSection(eligible_result_section);

      if (clayConfig.getItemById('eligible_1').get() === 'true' &&
          clayConfig.getItemById('eligible_2').get() === 'true' &&
          clayConfig.getItemById('eligible_3').get() === 'false' &&
          clayConfig.getItemById('eligible_4').get() === 'false' &&
          clayConfig.getItemById('eligible_5').get() === 'false' &&
          clayConfig.getItemById('eligible_6').get() === 'false' &&
          clayConfig.getItemById('eligible_7').get() === 'false' &&
          //clayConfig.getItemById('eligible_8').$element.get('ans') === 'false')
          clayConfig.getItemById('eligible_8').get() === 'false') {
        isEligible = true;
        clayConfig.getItemById('eligible_result_text').set("<p align='justify'>You are eligible to join the study.</p><br><p align='justify'>Next, we will use short descriptions to explain the research study, how participating may affect you, and what it means to consent to participate.</p>");
        //clayConfig.getItemById('eligible_result_button').set('Start Consent');
        clayConfig.getItemById('consent_button_start').show();
      } else {
        isEligible = false;
        clayConfig.getItemById('eligible_result_text').set("<p align='justify'>Sorry, you are not eligible to join the study. Tap the button below to exit the app.</p>");
        clayConfig.getItemById('consent_button_start').hide();
        clayConfig.getItemById('submit').set('Exit');
        clayConfig.getItemById('submit').show();
      }
    }
  }

  function consentButtonStartClick () {
    hideSection(eligible_result_section);
    showSection(consent_section);
  }

  function consentButtonNextClick () {
    if (consentPageIndex == consent_heading.length) {
      hideSection(consent_section);

      showSection(consent_review_section); 

      //clayConfig.getItemById('consent_review_text_form').hide(); // Not display the long form.
    } else {
      clayConfig.getItemById('consent_heading_top').set(consent_heading[consentPageIndex]);
      clayConfig.getItemById('consent_text_top').set("<p align='justify'>"+
        consent_text[consentPageIndex++]+'</p>');

      clayConfig.getItemById('consent_heading_bottom').set(consent_heading[consentPageIndex]);
      clayConfig.getItemById('consent_text_bottom').set("<p align='justify'>"+
        consent_text[consentPageIndex++]+'</p>');
    }
  }

  function consentButtonAgreeClick () {
    // TODO: consent_email input field does not verify if input format is valid

    clayConfig.getItemByMessageKey('is_consent').set(true);
    clayConfig.getItemByMessageKey('activate').set(true);

    hideSection(consent_review_section);
    showSection(consent_result_section);
  }

  function consentButtonDisagreeClick () {
    clayConfig.getItemByMessageKey('activate').set(false);
    clayConfig.getItemByMessageKey('is_consent').set(false);

    clayConfig.getItemById('consent_result_text').set('Okay, you have chosen not to participle in our study.');
    clayConfig.getItemById('submit').set('Confirm & Exit');
    hideSection(consent_review_section);
    clayConfig.getItemById('consent_result_text').show();
    clayConfig.getItemById('submit').show();
  }

  function consentResultButtonClick () {
    clayConfig.getItemById('submit').set('Submit');
    clayConfig.getItemById('submit').show();

    hideSection(consent_result_section);
    showSection(survey_section);

    clayConfig.getItemById('first_config').set(1); // Indicate the first config after consent.
    showConfigSection();
  }

  function viewConsentButtonClick () {
    hideSection(survey_section);
    hideConfigSection();

    showSection(consent_review_section);
  }

  function changeEnableApp () {
    if (this.get()) {
      sub_config_section.forEach(function (c) {
        clayConfig.getItemByMessageKey(c).enable();
      });
    } else {
      sub_config_section.forEach(function (c) {
        clayConfig.getItemByMessageKey(c).disable();
      });
    }
  }

  /**
   * Update the total_break value whenever the value of daily_start_time, daily_end_time, 
   * break_freq, or break_len changed. Display a warning message is time is invalid 
   * (start time must be less than end time).
   */
  function updateConfigSummary () {
    var start = clayConfig.getItemById('daily_start_time').get();
    var end = clayConfig.getItemById('daily_end_time').get();
    
    if (start < end) { // Valid time range.
      start = start.split(':');
      end = end.split(':');

      var breakFreq = parseInt(clayConfig.getItemById('break_freq').get());
      var breakLen = clayConfig.getItemById('break_len').get();

      var totalBreak = Math.floor((end[0]*60+parseInt(end[1])-start[0]*60-parseInt(start[1])) / breakFreq);

      //var message = " Great, that means there will be " + totalBreak + 
      //  " breaks in a day. Let's see if you can take a " + breakLen + 
      //  " minute walking break during each of them.";
      var message = "<p align='justify'>Great, let’s see if you can take a short " + breakLen +
        " min walking break during each hour. Given your settings, there will " + totalBreak +
        " possible walking breaks in the day.</p> <br>" + 
        " <p align='justify'>You’re all set, best of luck!</p>";

      clayConfig.getItemById('total_break').set(totalBreak);
      clayConfig.getItemById('config_summary').set(message);

      clayConfig.getItemById('config_summary').show();
      clayConfig.getItemById('time_warn_text').hide();

      clayConfig.getItemById('submit').enable();
    } else { // Invalid time range.
      clayConfig.getItemById('config_summary').hide();
      clayConfig.getItemById('time_warn_text').show();

      clayConfig.getItemById('submit').disable();
    }
  }

  /**
   * This function is called after the page is built (similar to the main function).
   */
  clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function () {
    //var activateToggle = clayConfig.getItemByMessageKey('activate');

    // Register event callbacks.
    //activateToggle.on('change', changeEnableApp);

    clayConfig.getItemById('eligible_button').on('click', eligibleButtonClick);
    clayConfig.getItemById('consent_button_start').on('click', consentButtonStartClick);
    clayConfig.getItemById('consent_button_next').on('click', consentButtonNextClick);
    clayConfig.getItemById('consent_result_button').on('click', consentResultButtonClick);
    clayConfig.getItemById('consent_button_agree').on('click', consentButtonAgreeClick);
    clayConfig.getItemById('consent_button_disagree').on('click',consentButtonDisagreeClick);
    clayConfig.getItemById('view_consent_button').on('click',viewConsentButtonClick);

    clayConfig.getItemById('daily_start_time').on('change', updateConfigSummary);
    clayConfig.getItemById('daily_end_time').on('change', updateConfigSummary);
    clayConfig.getItemById('break_freq').on('change', updateConfigSummary);
    clayConfig.getItemById('break_len').on('change', updateConfigSummary);

    // By default, every component is visible. We have to manually make some sections invisible.
    // We only want to display either the eligibility section or the configuration section.
    hideSection(hidden_components);
    hideWarningText();
    hideSection(eligible_result_section);
    hideSection(consent_section);
    hideSection(consent_review_section);
    hideSection(consent_result_section);
    hideSection(survey_section);
    hidePFButtons(); // Not using these custom buttons for now

    // If the user has already completed the onboarding process, directly 
    // show the configuration page by hiding the eligibility section.
    if (clayConfig.getItemByMessageKey('is_consent').get() === true) {
      hideSection(eligible_section);

      updateConfigSummary.call();

      clayConfig.getItemById('watchtoken').set(clayConfig.meta.watchToken);
      clayConfig.getItemById('version').set('v1.18.0');
    } else {
      hideConfigSection();
    }

    // Reset 'first_config' everytime we initialize the page.
    // 'first_config' serves as: 
    //   (1) a dummy key to allow Enamel automatically parse the settings sent
    //   (2) the indicator that this is the first config after user has provided consent.
    clayConfig.getItemById('first_config').set(0);

    // Disable certain sections so that users will not be able to edit those.
    disabled_components.forEach(function (c) { clayConfig.getItemById(c).disable(); });

    // Initialze some values.
    clayConfig.getItemById('time_zone').set(new Date().getTimezoneOffset());

    // Update the page view.
    //changeEnableApp.call(activateToggle);
  });

};

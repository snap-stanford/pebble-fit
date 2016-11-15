module.exports = function(minified) {
  var clayConfig = this;
  var $ = minified.$;
  var _ = minified._;
  var HTML = minified.HTML;

  var isEligible = false;
  var isConsent = false;

  /* List IDs of components in each section */
  var eligible_section = ['eligible_heading', 'eligible_text', 'eligible_button',
    'eligible_1', 'eligible_2', 'eligible_3', 'eligible_7', 'eligible_8'];
    //'eligible_4', 'eligible_5', 'eligible_6'];
  var pfbuttons = ['pfbutton_8'];
  var eligible_result_section = ['eligible_result_text', 'eligible_result_button'];
  var consent_start_section = ['consent_heading_start', 'consent_text_start', 
    'consent_button_start'];
  var consent_section = ['consent_heading', 'consent_text', 'consent_icon', 
    'consent_button_next'];
  var consent_review_section = ['consent_review_heading', 'consent_review_text', 
    'consent_review_heading_form', 'consent_review_approve_text', 'consent_review_expire_text',
    'consent_review_text_form', 'consent_button_agree', 'consent_button_disagree'];
  var consent_result_section = ['consent_result_text', 'consent_result_button']; 
  var config_section = ['settings', 'optin', 'vibrate', 
    'sleep_minutes', 
    'step_threshold', 'daily_start_time', 'daily_end_time', 
    'display_duration', 'optin_text'];
  var sub_config_section = ['vibrate', 'sleep_minutes', 
    'step_threshold', 'daily_start_time', 'daily_end_time', 
    'display_duration', 'optin_text'];

  /* Text to be displayed on each of the content pages. The idea is to use the same 
     components, but change the text displayed for them. Note that the texts of the
     first page is already supplied in the config.json file. */
  var consentPageIndex = 0;
  var consent_heading = ["Data Gathering", "Privacy", "Data Use"];
  var consent_text = [
    "This study will gather physical activity data from your smartphone and smartwatch as you go about your day. This will include the number of steps you take and the time you are sedentary. No other data will be accessed or gathered.",
    "Your data will be encrypted and sent to a secure database. If you choose to provide us with you name and email, which is not mandatory, this information will be stored separately within our database to prevent you from being identifiable.",
    "Your coded data (without your name) will be used for research by Stanford University and our research partners."
  ];

  /* Hide/Show various configuration sections, i.e. getItemById('config_section').hide()
   * or getItemById('config_section').show(). We have do it for each element since
   * from Clay GitHub page: "currently no way to disable or hide an entire section. 
   * You must disable/hide each item in the section to achieve this effect."
   * FIXME: this function is error prone. Put this function at the end of event handler
   * for safety.
   */
  function showSection(section) {
    section.forEach(function(c) { clayConfig.getItemById(c).show(); });
  }
  function hideSection(section) {
    section.forEach(function(c) { clayConfig.getItemById(c).hide(); });
  }

  /* Show/Hide the configuration page: including config_section, the submit button, 
   * and the consent form button. */
  function showConfigSection() {
    clayConfig.getItemById('submit').show();
    clayConfig.getItemById('view_consent_button').show();
    showSection(config_section);
  }
  function hideConfigSection() {
    clayConfig.getItemById('view_consent_button').hide();
    clayConfig.getItemById('submit').hide();
    hideSection(config_section);
  }

  /* Show the warnning text for the eligibility page if required field is answered. */
  function showWarningText() {
    //clayConfig.getItemById('eligible_warn_text').set(clayConfig.getItemById('eligible_8').$element.get('ans') === 'false');
    clayConfig.getItemById('eligible_warn_text').show();
  }
  function hideWarningText() {
    clayConfig.getItemById('eligible_warn_text').hide();
  }

  /* Hide all the pfbutton (yes-no buttons) components. */
  function hidePFButtons() {
    var yesButtons = $('.yes');
    for (var i = 0; i < yesButtons.length; i++) {
      yesButtons[i].style.display = 'none';
    }
    var yesButtons = $('.no');
    for (var i = 0; i < yesButtons.length; i++) {
      yesButtons[i].style.display = 'none';
    }
  }

  /* Check if the user is eligible to participate.
   * Participants are eligible if they answer yes to questions 1-2 and no to questions 3-9.
   */
  function eligibleButtonClick() {
    if (clayConfig.getItemById('eligible_3').get() === 'null' ||
        //clayConfig.getItemById('eligible_4').get() === 'null' ||
        //clayConfig.getItemById('eligible_5').get() === 'null' ||
        //clayConfig.getItemById('eligible_6').get() === 'null' ||
        clayConfig.getItemById('eligible_7').get() === undefined ||
        clayConfig.getItemById('eligible_8').$element.get('ans') === undefined) {
      showWarningText();
    } else {
      hidePFButtons();
      hideWarningText();
      hideSection(eligible_section);
      showSection(eligible_result_section);

      if (clayConfig.getItemById('eligible_1').get()  === true &&
          clayConfig.getItemById('eligible_2').get()  === true &&
          clayConfig.getItemById('eligible_3').get()  === 'false' &&
          //clayConfig.getItemById('eligible_4').get() === 'false' &&
          //clayConfig.getItemById('eligible_5').get() === 'false' &&
          //clayConfig.getItemById('eligible_6').get() === 'false' &&
          clayConfig.getItemById('eligible_7').get() === 'false' &&
          clayConfig.getItemById('eligible_8').$element.get('ans') === 'false') {
        isEligible = true;
        clayConfig.getItemById('eligible_result_text').set('You are eligible to join the study. Tap the button below to begin the consent process.');
        clayConfig.getItemById('eligible_result_button').set('Start Consent');
      } else {
        isEligible = false;
        clayConfig.getItemById('eligible_result_text').set('Sorry, you are not eligible to join the study. Tap the button below to exit the app.');
        clayConfig.getItemById('eligible_result_button').hide();
        clayConfig.getItemById('submit').set('Exit');
        clayConfig.getItemById('submit').show();
      }
    }
  }

  function eligibleResultButtonClick() {
    //if (isEligible) {
      hideSection(eligible_result_section);
      showSection(consent_start_section);
    //} else {
    //  clayConfig.getItemById('submit').trigger('click'); // FIXME: no effect
    //  clayConfig.destroy(); // This only destroy the page but not exit the config
    //}
  }

  function consentButtonStartClick() {
    hideSection(consent_start_section);
    showSection(consent_section);
  }

  function consentButtonNextClick() {
    if (consentPageIndex == consent_heading.length) {
      hideSection(consent_section);
      showSection(consent_review_section);
    } else {
      clayConfig.getItemById('consent_heading').set(consent_heading[consentPageIndex]);
      clayConfig.getItemById('consent_text').set(consent_text[consentPageIndex++]);
    }
  }

  function consentButtonAgreeClick() {
    clayConfig.getItemByMessageKey('is_consent').set(true);

    hideSection(consent_review_section);
    showSection(consent_result_section);
  }

  function consentButtonDisagreeClick() {
    clayConfig.getItemByMessageKey('is_consent').set(false);

    clayConfig.getItemById('consent_result_text').set('Okay, you have chosen not to participle in our study.');
    clayConfig.getItemById('submit').set('Exit');
    hideSection(consent_review_section);
    clayConfig.getItemById('consent_result_text').show();
    clayConfig.getItemById('submit').show();
  }

  function consentResultButtonClick() {
    hideSection(consent_result_section);
    showConfigSection();
  }

  function viewConsentButtonClick() {
    showSection(consent_review_section);
    hideConfigSection();
  };


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

    /* Register event callbacks. */
    optinToggle.on('change', changeEnableApp);
    clayConfig.getItemById('eligible_button').on('click', eligibleButtonClick);
    clayConfig.getItemById('eligible_result_button').on('click', eligibleResultButtonClick);
    clayConfig.getItemById('consent_button_start').on('click', consentButtonStartClick);
    clayConfig.getItemById('consent_button_next').on('click', consentButtonNextClick);
    clayConfig.getItemById('consent_result_button').on('click', consentResultButtonClick);
    clayConfig.getItemById('consent_button_agree').on('click', consentButtonAgreeClick);
    clayConfig.getItemById('consent_button_disagree').on('click',consentButtonDisagreeClick);
    clayConfig.getItemById('view_consent_button').on('click',viewConsentButtonClick);

    /* Show only the eligible section at the beginning. */
    // FIXME: if user has already opted in, should skip this and directly show config page
    // TODO: hiding these for now
    clayConfig.getItemById('eligible_4').hide();
    clayConfig.getItemById('eligible_5').hide();
    clayConfig.getItemById('eligible_6').hide();

    //clayConfig.getItemByMessageKey('is_consent').hide();
    clayConfig.getItemByMessageKey('is_consent').hide();
    hideWarningText();
    hideSection(eligible_result_section);

    hideSection(consent_start_section);
    hideSection(consent_section);
    hideSection(consent_review_section);
    hideSection(consent_result_section);

    /* If the user has already completed the onboarding process, directly 
     * show the configuration page by hiding the eligibility section. */
    if (clayConfig.getItemByMessageKey('is_consent').get() === true) {
      hideSection(eligible_section);
      hidePFButtons();
    } else {
      hideConfigSection();
    }

    //changeEnableApp.call(optinToggle);
  });

};

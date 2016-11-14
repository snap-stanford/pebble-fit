'use strict';
/* This is a custom Pebble Clay component represents two buttons (yes & no) 
 * placed side-by-side. Whenever one button is clicked, it is highlighted
 * and disabled while the other one is de-highlighted and enabled.
 * (The default Clay button component is placed vertically one per line)
 */

/* Method which will be called after the item has been added to the page. 
 * It will be called with the ClayItem as the context (this) and with minified 
 * as the first parameter and clayConfig as the second parameter
 */
function initialize(minified, clayConfig) {
  self = this;
  // FIXME: adding button click handler dose not work 

  //function clickYes() {
  //  var noBtn = this.parentNode.getElementsByClassName('no')[0];
  //  noBtn.disabled = false;
  //  noBtn.style.color = 'black';
  //  this.style.color = 'red';
  //  this.disabled = true;
  //}

  //clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function () {
  //  clayConfig.getItemById('eligible_text').on('click', function() {
  //    this.set("test");
  //  });
  //});
}

/* This is the actual HTML content of the component. Make sure there is only 
 * one root node in the HTML.
 * FIXME: currently using a workaround to make sure the effect of this button
 *        clicking is recored (stored in a text item prior to this pfbutton item).
 */
var template = '\
  <html> \
    <input type="button" class="yes" value="Yes" \
      onclick="noBtn=this.parentNode.getElementsByClassName(\'no\')[0]; \
               noBtn.disabled=false; \
               noBtn.style.color=\'black\'; \
               this.previousSibling.previousSibling.ans=\'true\'; \
               this.style.color=\'red\'; \
               this.disabled=true" > \
    <input type="button" class="no" value="No" \
      onclick="yesBtn=this.parentNode.getElementsByClassName(\'yes\')[0]; \
               yesBtn.disabled=false; \
               yesBtn.style.color=\'black\'; \
               this.previousSibling.previousSibling.previousSibling.previousSibling.ans=\'false\'; \
               this.style.color=\'red\'; \
               this.disabled=true" > \
  </html>';

// FIXME: consider using a custom manipulator instead of default 'html' one
//        since its hide() function does not seeme to work
//var manipulator = {
//  get: function() { return 1; }
//  hide: function() {
//};

var PFButton = {
  name: 'pfbutton',
  manipulator: 'html',
  initialize: initialize,
  style: '',
  template: template
};

/* This somehow does not work */
//var PFButton = function () {
//    this.name = 'pfbutton';
//    this.template = template;
//    this.style = '';
//    this.manipulator = 'html';
//};

module.exports = PFButton;

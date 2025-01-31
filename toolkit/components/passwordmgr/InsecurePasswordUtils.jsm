/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

this.EXPORTED_SYMBOLS = [ "InsecurePasswordUtils" ];

const { classes: Cc, interfaces: Ci, results: Cr, utils: Cu } = Components;
const STRINGS_URI = "chrome://global/locale/security/security.properties";

Cu.import("resource://gre/modules/Services.jsm");
Cu.import("resource://gre/modules/XPCOMUtils.jsm");

XPCOMUtils.defineLazyModuleGetter(this, "devtools",
                                  "resource://devtools/shared/Loader.jsm");
XPCOMUtils.defineLazyServiceGetter(this, "gContentSecurityManager",
                                   "@mozilla.org/contentsecuritymanager;1",
                                   "nsIContentSecurityManager");
XPCOMUtils.defineLazyServiceGetter(this, "gScriptSecurityManager",
                                   "@mozilla.org/scriptsecuritymanager;1",
                                   "nsIScriptSecurityManager");
XPCOMUtils.defineLazyGetter(this, "WebConsoleUtils", () => {
  return this.devtools.require("devtools/server/actors/utils/webconsole-utils").Utils;
});

this.InsecurePasswordUtils = {
  _formRootsWarned: new WeakMap(),
  _sendWebConsoleMessage(messageTag, domDoc) {
    let windowId = WebConsoleUtils.getInnerWindowId(domDoc.defaultView);
    let category = "Insecure Password Field";
    // All web console messages are warnings for now.
    let flag = Ci.nsIScriptError.warningFlag;
    let bundle = Services.strings.createBundle(STRINGS_URI);
    let message = bundle.GetStringFromName(messageTag);
    let consoleMsg = Cc["@mozilla.org/scripterror;1"].createInstance(Ci.nsIScriptError);
    consoleMsg.initWithWindowID(message, domDoc.location.href, 0, 0, 0, flag, category, windowId);

    Services.console.logMessage(consoleMsg);
  },

  /**
   * Checks if there are insecure password fields present on the form's document
   * i.e. passwords inside forms with http action, inside iframes with http src,
   * or on insecure web pages. If insecure password fields are present,
   * a log message is sent to the web console to warn developers.
   *
   * @param {FormLike} aForm A form-like object. @See {LoginFormFactory}
   */
  checkForInsecurePasswords(aForm) {
    if (this._formRootsWarned.has(aForm.rootElement) ||
        this._formRootsWarned.get(aForm.rootElement)) {
      return;
    }

    let domDoc = aForm.ownerDocument;
    let isSafePage = domDoc.defaultView.isSecureContext;

    if (!isSafePage) {
      if (domDoc.defaultView == domDoc.defaultView.parent) {
        this._sendWebConsoleMessage("InsecurePasswordsPresentOnPage", domDoc);
      } else {
        this._sendWebConsoleMessage("InsecurePasswordsPresentOnIframe", domDoc);
      }
      this._formRootsWarned.set(aForm.rootElement, true);
    }

    let isFormSubmitHTTP = false, isFormSubmitSecure = false;
    if (aForm.rootElement instanceof Ci.nsIDOMHTMLFormElement) {
      let uri = Services.io.newURI(aForm.rootElement.action || aForm.rootElement.baseURI,
                                   null, null);
      let principal = gScriptSecurityManager.getCodebasePrincipal(uri);

      if (uri.schemeIs("http")) {
        isFormSubmitHTTP = true;
        if (gContentSecurityManager.isOriginPotentiallyTrustworthy(principal)) {
          isFormSubmitSecure = true;
        } else if (isSafePage) {
          // Only warn about the action if we didn't already warn about the form being insecure.
          this._sendWebConsoleMessage("InsecureFormActionPasswordsPresent", domDoc);
          this._formRootsWarned.set(aForm.rootElement, true);
        }
      } else {
        isFormSubmitSecure = true;
      }
    }

    // The safety of a password field determined by the form action and the page protocol
    let passwordSafety;
    if (isSafePage) {
      if (isFormSubmitSecure) {
        passwordSafety = 0;
      } else if (isFormSubmitHTTP) {
        passwordSafety = 1;
      } else {
        passwordSafety = 2;
      }
    } else if (isFormSubmitSecure) {
      passwordSafety = 3;
    } else if (isFormSubmitHTTP) {
      passwordSafety = 4;
    } else {
      passwordSafety = 5;
    }

    Services.telemetry.getHistogramById("PWMGR_LOGIN_PAGE_SAFETY").add(passwordSafety);
  },
};

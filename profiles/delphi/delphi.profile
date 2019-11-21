<?php
/**
 * @file
 * Enables modules and site configuration for a standard site installation.
 */



/**
 * Implements hook_form_FORM_ID_alter() for install_configure_form().
 *
 * Allows the profile to alter the site configuration form.
 */
function delphi_form_install_configure_form_alter(&$form, $form_state) {
  // Pre-populate the site name with the server name.
  $form['site_information']['site_name']['#default_value'] = 'Real Time Delphi Tool';
  $form['site_information']['site_mail']['#default_value'] = 'hoferj@uni-hildesheim.de';


}
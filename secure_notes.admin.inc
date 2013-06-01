<?php

/**
 * @file
 * Provide the admin related functions for Secure Notes.
 */

/**
 * Defines the Secure Notes admin settings form.
 */
function secure_notes_admin_settings_form() {
  // Make sure the secure note type hasn't been deleted.
  $default = in_array('secure_note', array_keys(node_type_get_names())) ? array('secure_note' => 'secure_note') : array();
  $form = array();

  $form['secure_notes_allowed_types'] = array(
    '#title' => t('The content types that are allowed to be viewed or created as secure notes.'),
    '#description' => t('Only specified node types will be allowed to be accessed or created using secure notes.'),
    '#type' => 'checkboxes',
    '#options' => node_type_get_names(),
    '#default_value' => $default,
  );

  $form['secure_notes_default_expiration'] = array(
    '#title' => t('Token expiration'),
    '#description' => t('Secure note tokens will expire if not used after this amount of time.'),
    '#type' => 'select',
    '#options' => array(
      1 => t('1 hour'),
      2 => t('2 hours'),
      4 => t('4 hours'),
      8 => t('8 hours'),
      12 => t('12 hours'),
      24 => t('24 hours (1 day)'),
      48 => t('48 hours (2 days)'),
      72 => t('72 hours (3 days)'),
      120 => t('120 hours (5 days)'),
      168 => t('168 hours (1 week)'),
    ),
    '#default_value' => 48,
  );

  return system_settings_form($form);
}

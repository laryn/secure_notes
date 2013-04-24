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
  return system_settings_form($form);
}

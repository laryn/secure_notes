<?php

/**
 * @file
 * Provide the admin related functions for Secure Notes.
 */

/**
 * Defines the Secure Notes admin settings form.
 */
function secure_notes_admin_settings_form() {
  $form = array();
  $form['secure_notes_allowed_types'] = array(
    '#title' => t('The content types that are allowed to be viewed or created as secure notes.'),
    '#description' => t('Only specified node types will be allowed to be accessed or created using secure notes.'),
    '#type' => 'checkboxes',
    '#options' => node_type_get_names(),
    '#default_value' => variable_get('secure_notes_allowed_types', array('secure_note' => 'Secure Note')),
  );
dpm(node_type_get_names());
  return system_settings_form($form);
}

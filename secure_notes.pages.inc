<?php

/**
 * Generate a secure token.
 */
function secure_notes_generate_token($time = REQUEST_TIME) {
  $token = $time . drupal_get_hash_salt();
  // Hash the token 2^16 times.
  for ($i = 0; $i < pow(2, 16); $i++) {
    $token = drupal_hmac_base64($token , drupal_random_bytes(10) . drupal_get_hash_salt());
  }
  return $token;
}

/**
 * TODO
 */
function secure_notes_add_link_invite($form, &$form_state) {
  global $base_url;
  $token = isset($form_state['token']) ? 'Secure note creation link: ' . $base_url . '/secure-notes/add/' . $form_state['token'] : '';
  $options = secure_notes_get_allowed_types();
  $types = node_type_get_names();
  foreach ($options as $key => $value) {
    $options[$key] = $types[$key];
  }

  $form = array();

  $form['type'] = array(
    '#title' => t('Node Type'),
    '#description' => t('What type of node is the user being invited to create?'),
    '#type' => 'select',
    '#options' => $options,
  );

  $form['email'] = array(
    '#title' => t('Email'),
    '#description' => t('The email address of the person that will create the note.'),
    '#type' => 'textfield',
  );

  $form['token'] = array(
    '#type' => 'markup',
    '#markup' => $token,
    '#prefix' => '<div id="token"><h3>',
    '#suffix' => '</h3></div>',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Generate Link',
    '#ajax' => array(
      'callback' => 'secure_notes_ajax_add_link_invite_submit',
      'wrapper' => 'token',
    ),
    '#submit' => array('secure_notes_add_link_invite_submit'),
  );

  return $form;
}

/**
 * TODO
 */
function secure_notes_add_link_invite_submit($form, &$form_state) {
  $token = secure_notes_generate_token();
  $form_state['token'] = $token;
  $form_state['rebuild'] = TRUE;
  $expires = REQUEST_TIME + variable_get('secure_notes_default_expiration', 48) * 60 *60;
  try {
    db_insert('secure_notes_create')
      ->fields(array(
        'token' => $token,  
        'expires' => $expires,
        'type' => $form_state['values']['type'],
        'email' => $form_state['values']['email'],
      ))->execute();
  }
  catch (PDOException $e) {
    if ($e->errorInfo[1] != 1062) {
      throw $e;
    }
    else {
      $form_state['token'] = NULL;
      drupal_set_message(t('You have just experienced a rare and amazing occurance. A cryptographic collision has occured! Do not fret, nothing was harmed. While this is really rare and interesting it just means that we need you to generate another token. Submit the form again to leave this strange paralell universe.'), 'error');
      return;
    }
  }
  drupal_set_message(t('A secure note creation token has been generated and stored. It will expire in TK hours.'));
}

/**
 * Submit handler for token creation.
 */
function secure_notes_ajax_add_link_invite_submit($form, &$form_state) {
  global $base_url;
  $element = $form['token'];
  $element['#markup'] = 'Secure note creation link: ' . $base_url . '/secure-notes/add/' . $form_state['token'];
  return $element;
}


/**
 * TODO
 */
function secure_notes_get_allowed_types() {
  $default = in_array('secure_note', array_keys(node_type_get_names())) ? array('secure_note' => 'secure_note') : array();
  return variable_get('secure_notes_allowed_types', $default);
}


/**
 * TODO
 */
function secure_notes_render_add_form($token) {
  $type = db_select('secure_notes_create', 'snc')
    ->fields('snc', array('type'))
    ->condition('token', $token, '=')
    ->execute()
    ->fetchField();
  if (!$type) {
    drupal_access_denied();
  }
  module_load_include('inc', 'node', 'node.pages');
  $form = node_add($type);
  $output = drupal_render($form);
  return $output;
}

/**
 * TODO
 */
function secure_notes_render_node_view($token) {
  $nid = db_select('secure_notes_view', 'snv')
    ->fields('snv', array('nid'))
    ->condition('token', $token, '=')
    ->execute()
    ->fetchField();
  db_delete('secure_notes_view')
    ->condition('token', $token, '=')
    ->execute();
  $output = node_view(node_load($nid));
  return $output;
}

/**
 * TODO
 */
function secure_notes_node_form_submit($form, &$form_state) {
  // TODO make this a real redirect.
  $form_state['redirect'] = 'node/14';
}

/**
 * TODO
 */
function secure_notes_form_node_form_alter(&$form, &$form_state) {
  $menu = menu_get_item();
  if ($menu['path'] == 'secure-notes/add/%') {
    $form['actions']['submit']['#submit'][] = 'secure_notes_node_form_submit';
  }
}

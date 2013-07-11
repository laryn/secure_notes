<?php

/**
 * Generate a secure token.
 */
function secure_notes_generate_token($time = REQUEST_TIME) {
  $token = $time . drupal_get_hash_salt();
  // Hash the token 2^14 times.
  for ($i = 0; $i < pow(2, 14); $i++) {
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
  $options = secure_notes_get_types();
  $types = node_type_get_names();
  foreach ($options as $key => $value) {
    if (empty($value)) {
      unset($options[$key]);
    }
    else {
      $options[$key] = $types[$key];
    }
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

  $form['email_send'] = array(
    '#title' => t('Send email'),
    '#description' => t('If checked the system will automatically email the user for you using the text below.'),
    '#type' => 'checkbox',
  );

  $default_message = secure_notes_default_email_message('view');
  $form['email_message'] = array(
    '#title' => t('Email Message'),
    '#description' => t('The following email will be automatically send to the email address above. You may alter this message but make sure to include the [invite-token] somewhere in the message or else your invitation will not be useful at all.'),
    '#type' => 'textarea',
    '#default_value' => check_plain(variable_get('secure_notes_default_message_add', $default_message)),
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
    '#validate' => array('secure_notes_invite_validate'),
    '#submit' => array('secure_notes_add_link_invite_submit'),
  );

  return $form;
}

/**
 * TODO
 */
function secure_notes_view_link_invite($form, &$form_state) {
  global $base_url;
  if (empty($form_state['values']['nid'])) {
    $item = menu_get_item();
    $nid = $item['map'][1];
  }
  else {
    $nid = $form_state['values']['nid'];
  }

  $token = isset($form_state['token']) ? 'Secure note creation link: ' . $base_url . '/secure-notes/view/' . $form_state['token'] : '';

  $form = array();

  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $nid,
  );

  $form['email'] = array(
    '#title' => t('Email'),
    '#description' => t('The email address of the person that will create the note.'),
    '#type' => 'textfield',
  );

  $form['email'] = array(
    '#title' => t('Email'),
    '#description' => t('The email address of the person that will create the note.'),
    '#type' => 'textfield',
  );

  $form['email_send'] = array(
    '#title' => t('Send email'),
    '#description' => t('If checked the system will automatically email the user for you using the text below.'),
    '#type' => 'checkbox',
  );

  $default_message = secure_notes_default_email_message('view');
  $form['email_message'] = array(
    '#title' => t('Email Message'),
    '#description' => t('The following email will be automatically send to the email address above. You may alter this message but make sure to include the [invite-token] somewhere in the message or else your invitation will not be useful at all.'),
    '#type' => 'textarea',
    '#default_value' => check_plain(variable_get('secure_notes_default_message_view', $default_message)),
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
      'callback' => 'secure_notes_ajax_view_link_invite_submit',
      'wrapper' => 'token',
    ),
    '#validate' => array('secure_notes_invite_validate'),
    '#submit' => array('secure_notes_view_link_invite_submit'),
  );

  return $form;
}

/**
 * Validation function for secure notes invitations forms.
 *
 * @see TODO
 * @see TODO
 */
function secure_notes_invite_validate($form, &$form_state) {
  if ($form_state['values']['email_send']) {
    $message = $form_state['values']['email_message'];
    $email = $form_state['values']['email'];
    if (!valid_email_address($email)) {
      form_set_error('email', 'The email address is invalid.');
    }
    if (strpos($message, '[invite-token]') === FALSE) {
      form_set_error('email_message', 'The invite token [invite-token] was not included in the message.');
    }
  }
}

/**
 * TODO
 */
function secure_notes_add_link_invite_submit($form, &$form_state) {
  global $user;
  $token = secure_notes_generate_token();
  $form_state['token'] = $token;
  $form_state['rebuild'] = TRUE;
  $expires_hours = variable_get('secure_notes_default_expiration', 48);
  $expires = REQUEST_TIME + $expires_hours * 60 *60;
  try {
    db_insert('secure_notes_create')
      ->fields(array(
        'token' => $token,
        'expires' => $expires,
        'type' => $form_state['values']['type'],
        'email' => $form_state['values']['email'],
        'uid' => $user->uid,
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
  // If the box is checked. Attempt to send message to the user.
  if ($form_state['values']['email_send']) {
    $message = $form_state['values']['email_message'];
    $module = 'secure_notes';
    $key = 'secure_notes_add';
    // Specify 'to' and 'from' addresses.
    $to = $form_values['email'];
    $from = variable_get('site_mail', 'admin@example.com');

    // Load the message into params
    $params = $form_values;
    $language = language_default();
    $send = TRUE;
    // Send the mail, and check for success.
    $result = drupal_mail($module, $key, $to, $language, $params, $from, $send);
    if ($result['result'] == TRUE) {
      drupal_set_message(t('Your message has been sent.'));
    }
    else {
      drupal_set_message(t('There was a problem sending your message and it was not sent.'), 'error');
    }
  }
  drupal_set_message(t('A secure note creation token has been generated and stored. It will expire in @hours hours.', array('@hours' => $expires_hours)));
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
function secure_notes_view_link_invite_submit($form, &$form_state) {
  global $user;
  $token = secure_notes_generate_token();
  $form_state['token'] = $token;
  $form_state['rebuild'] = TRUE;
  $expires_hours = variable_get('secure_notes_default_expiration', 48);
  $expires = REQUEST_TIME + $expires_hours * 60 *60;
  try {
    db_insert('secure_notes_view')
      ->fields(array(
        'token' => $token,
        'expires' => $expires,
        'email' => $form_state['values']['email'],
        'nid' => $form_state['values']['nid'],
        'uid' => $user->uid,
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
  drupal_set_message(t('A secure note creation token has been generated and stored. It will expire in @hours hours.', array('@hours' => $expires_hours)));
}

/**
 * Submit handler for token creation.
 */
function secure_notes_ajax_view_link_invite_submit($form, &$form_state) {
  global $base_url;
  $element = $form['token'];
  $element['#markup'] = 'Secure note view link: ' . $base_url . '/secure-notes/view/' . $form_state['token'];
  return $element;
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
  $node = node_load($nid);
  $node->note_pickup = TRUE;
  $output = node_view($node);
  return $output;
}

/**
 * TODO
 */
function secure_notes_node_form_submit($form, &$form_state) {
  $item = menu_get_item();
  $token = $item['page_arguments'][0];
  db_delete('secure_notes_create')
    ->condition('token', $token, '=')
    ->execute();
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

/**
 * TODO
 */
function secure_notes_review($form, $form_state, $invitation_type, $nid = NULL) {
  // Direct them to the confirm form submission if they have been through once
  // already.
  if (isset($form_state['storage']['confirm'])) {
    $tokens = $form_state['values']['tokens'];
    $form = secure_notes_delete_confirm($form, $form_state, $invitation_type, $tokens);
    $form['#submit'] = array('secure_notes_delete_confirm_submit');
    return $form;
  }

  // Otherwise build out the normal administration form.
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Delete Selected'),
    '#validate' => array('secure_notes_manage_validate'),
    '#submit' => array('secure_notes_manage_submit'),
  );

  // Build the sortable table header.
  $header = array();
  if ($invitation_type == 'add') {
    $header['type'] = array('data' => t('Type'), 'field' => 't.type');
    $abrv = 'a';
  }
  if ($invitation_type == 'view') {
    if (!$nid) {
      $header['title'] = array('data' => t('Title'), 'field' => 'n.title');
    }
    $abrv = 'v';
  }
  $header = $header + array(
    'email' => array('data' => t('Email'), 'field' => 't.email'),
    'generator' => array('data' => t('Invited By'), 'field' => 't.uid'),
    'token' => array('data' => t('Token'), 'field' => 't.token'),
    'expires' => array('data' => t('Expires'), 'field' => 't.expires'),
    'operations' => t('Operations'),
  );

  if ($invitation_type == 'add') {
    $query = db_select('secure_notes_create', 't')->extend('PagerDefault')->extend('TableSort');
    $query->fields('t', array('token', 'uid', 'email', 'expires', 'type'));
  }
  if ($invitation_type == 'view') {
    $query = db_select('secure_notes_view', 't')->extend('PagerDefault')->extend('TableSort');
    $query->fields('t', array('token', 'uid', 'email', 'expires', 'nid'))->join('node', 'n', 'n.nid = t.nid');
    if (!empty($nid)) {
      $query->condition('t.nid', $nid, '=');
    }
  }
  $tokens = $query->limit(50)->orderByHeader($header)->execute();

  $destination = drupal_get_destination();

  $options = array();
  while($token = $tokens->fetchAssoc()) {
    $options[$token['token']] = array();
    if (!empty($token['type'])) {
      $options[$token['token']]['type'] = check_plain(node_type_get_name($token['type']));
    }
    else if (!empty($token['nid'])) {
      $node = node_load($token['nid']);
      $options[$token['token']]['title'] = l($node->title, 'node/' . $node->nid);
    }
    $options[$token['token']] = $options[$token['token']] + array(
      'email' => isset($token['email']) ? check_plain($token['email']) : '',
      'generator' => theme('username', array('account' => user_load($token['uid']))),
      'token' => $token['token'],
      'expires' => format_date($token['expires'], 'short'),
    );
    // We use an abbriviaation in the delete link to avoid confusion. It would
    // be weird to see a link such as secure-note/add/token/delete.
    $link = array(
      'title' => t('delete'),
      'href' => 'secure-notes/' . $abrv . '/' . $token['token'] . '/delete',
      'query' => $destination,
    );

    $options[$token['token']]['operations'] = array(
      'data' => array(
        '#type' => 'link',
        '#title' => $link['title'],
        '#href' => $link['href'],
        '#options' => array('query' => $link['query']),
      ),
    );
  }
  $form['tokens'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
    '#empty' => t('No content available.'),
  );
  $form['pager'] = array('#markup' => theme('pager'));
  return $form;
}

/**
 * Form constructor for the token deletion confirmation form.
 *
 * @see secure_notes_delete_confirm_submit()
 */
function secure_notes_delete_confirm($form, &$form_state, $type, $tokens) {
  if ($type == 'a') {
    $type = 'add';
  }
  else if ($type == 'v') {
    $type = 'view';
  }
  // If the incoming value is a string then convert it to a single value array.
  if (is_string($tokens)) {
    $tokens = array(
      $tokens,
    );
  }
  else {
    $tokens = array_filter($tokens);
  }
  $form['secure-notes-type'] = array('#type' => 'value', '#value' => $type);
  $form['secure-notes-tokens'] = array('#type' => 'value', '#value' => $tokens);

  $question = format_plural(count($tokens), 'Are you sure you want to delete this token?', 'Are you sure you want to delete these tokens?');
  $destination = drupal_get_destination();

  return confirm_form(
    $form,
    $question,
    $destination['destination'],
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Executes token deletion.
 *
 * @see secure_notes_delete_confirm()
 */
function secure_notes_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $tokens = $form_state['values']['secure-notes-tokens'];
    $type = $form_state['values']['secure-notes-type'];
    secure_notes_token_delete($type, $tokens);
    $count = count($tokens);
    watchdog('secure notes', 'Deleted @count secure notes tokens.', array('@count' => $count));
    drupal_set_message(format_plural($count, 'Deleted 1 secure notes token.', 'Deleted @count secure notes tokens.'));
  }
}

function secure_notes_manage_validate($form, &$form_state) {
  if (!is_array($form_state['values']['tokens']) || !count(array_filter($form_state['values']['tokens']))) {
    form_set_error('', t('No items selected.'));
  }
}

function secure_notes_manage_submit($form, &$form_state) {
  if (empty($form_state['storage']['confirm'])) {
    $tokens = array_filter($form_state['values']['tokens']);
    $form_state['rebuild'] = TRUE;
    $form_state['storage']['confirm'] = TRUE;
  }
  else {
    secure_notes_delete_confirm_submit($form, $form_state);
    $form_state['rebuild'] = TRUE;
  }
}

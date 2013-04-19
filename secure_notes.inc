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

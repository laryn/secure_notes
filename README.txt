Secure Notes helps solve the problem of exchanging sensitive information with
people who do not have the same tool-set or skill-set. It provides a system,
similar to password reset links in which a one-time node creation link can be
sent to a person's email address. This is meant to solve the problem of
exchanging sensitive passwords via email.

Secure Notes links allow the holder to create a normal node once and only
once. The person does not need to be a user of the site. An additional
one-time read link can also be generated, so that site users can provide
access to a piece of content to single person once and only once.

An additional module provides a mechanism to destroy the secure notes after
they are viewed. The reasoning behind this is that Drupal, by default, is not
a particularly good place to store secret or sensitive information.
The Secure Notes module is meant to make the exchange of this secret
information easy but is not meant to provide long term storage for these
secrets.

It should also be noted that this method of exchanging information, while much
better than unencrypted email, is still quite vulnerable unless your site is
running under SSL. If your site is using HTTPS to secure information between
the server and end-users then this method is fairly strong as the only
weak-points are Drupal and the server itself.

Secure notes menu items can be found at
 - /admin/content/secure-notes
 - /admin/config/content/secure-notes

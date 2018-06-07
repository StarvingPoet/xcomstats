"Installation Instructions"

The XCOM Battlestat Client requires installation of PHP 7.2+ to be installed with the PECL YAML extension.

You can find PHP here: https://secure.php.net/downloads.php
You can find the YAML ext here: https://secure.php.net/manual/en/yaml.installation.php

Once the above is installed and the YAML extension is enabled in php.ini, extract this repository into its own directory.

From the command-line simple type "(pathtophp)php battlestats.php" or edit the included .bat file.

All user defined variables are in battlestats.php:

  *  root     = 'E:/Games/OpenXcom - Extended/';
  *  $user     = 'user/.XCF/';
  *  $saves    = 'xcom1/';

/** The following is for making your BattleStats visible online **/
  *  $options['broadcast']   = true;                 // true or false - if true, supply below
  *  $options['displayName'] = "Display Name";       // What's your handle?
  *  $options['passKey']     = "RandomWordsGoHere";  // Just pick a couple random words, not cryptographically secure - DONT USE A PASSWORD
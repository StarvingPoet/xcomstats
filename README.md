##Installation Instructions

**The XCOM Battlestat Client requires installation of PHP 7.2+ to be installed with the PECL YAML extension.**

You can find PHP here: https://secure.php.net/downloads.php

You can find the YAML ext here: https://secure.php.net/manual/en/yaml.installation.php


Once the above is installed and the YAML extension is enabled in php.ini, extract this repository into its own directory.
From the command-line simple type "(pathtophp)php battlestats.php"


All user defined variables are in userVariables.php:

```php
  root     = 'E:/Games/OpenXcom - Extended/';  //base xcom install
  $user     = 'user/.XCF/';                    //user folder (where the mod and save directories are)
  $saves    = 'xcom1/';                        //save folder 

  /** The following is for making your BattleStats visible online **/
  $options['broadcast']   = true;                 // true or false - if true, supply below
  $options['displayName'] = "Display Name";       // What's your handle?
  $options['passKey']     = "RandomWordsGoHere";  // Just pick a couple random words, not cryptographically secure - DONT USE A PASSWORD
```

Your overlay will be visible at http://www.openpaedia.com/xcomstats_overlay
type 'list' to find a link to your unique url.
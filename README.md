## Installation Instructions

**The XCOM Battlestat Client requires installation of PHP 7.2+ to be installed with the PECL YAML extension.**

You can find PHP here: https://secure.php.net/downloads.php

You can find the YAML ext here: https://secure.php.net/manual/en/yaml.installation.php


Once the above is installed and the YAML extension is enabled in php.ini, extract this repository into its own directory.
From the command-line simple type "(pathtophp)php battlestats.php"


All user defined variables are in userVariables.php:

```php
  $root     = 'E:/Games/OpenXcom - Extended/';  //base xcom install
  $user     = 'user/.XCF/';                     //user folder (where the mod and save directories are)
  $saves    = 'xcom1/';                         //save folder

  /** The following is for making your BattleStats visible online **/
  $options['broadcast']   = true;                              // true or false - if true, supply below
  $options['displayName'] = "Display Name";                    // What's your handle?
  $options['passKey']     = "RandomWordsGoHere";               // Just pick a couple random words, not cryptographically secure - DONT USE A PASSWORD
  $options['defaultNames'] = array('Case Officer', 'Al', 'K9') // Used in conjunction with the clone function to determine which soldiers are named or not.  Soldiers with these names will not be cloned and will be pulled from training as needed.  Highly recommended that you use a naming mod to change the default names
```

If broadcast is true, Your overlay will be visible at http://www.openpaedia.com/xcomstats_overlay
type 'list' to find a link to your unique url.

### Clone and Resupply Functions
  * These functions require internet connectivity to work, sorry.

## Resupply

save name syntax `resupply temp=x base=y`

`temp` is the ship loadout template you want to fulfill.  You can distribute armor by manually adding armor to the loadout via save editing.

`base` are the bases you want to distribute across.
  * base=2 means supply the first two bases
  * base=3-5 means supply base 3 through 5
  * base=1,4,7 means supply base 1,4, and 7 and ignore the rest.
  * We start counting at 1 and not 0.

The resupply function will attempt, storage space permitting, to resupply bases with the items in the craft loadout template provided.  If the item can be bought, it will be bought and put in transfers.  If it can be manufactured (all research unlocked) then it will be manufactured.  If the manufactured item has pre-requisites, then those will be acquired as well, as the item above it.   In large mods, you can choose which manufacturing project it will default to by hiding the ones you don't want to queue.   Manufacturing projects will be queued into the base with the most amount of total workshop space.  The number of bases you select will determine the total number of items to procure.  If you tell it to supply all 8 bases with 100 grenades, but only have the combined storage space for 200 grenades, it will charge you for all 800 and just dump the remaining 600.  I will not change this.  Don't be lazy.
Note:
  * This WILL run you into negative cash if you are not careful.
  * This WILL give you negative workshop space if you are not careful.
  * Don't be greedy

## Cloning Soldiers:

save name syntax: `clone soldier=x base=y`

`soldier` is the number of soldiers you want to clone

`base` is the number of bases to distribute soldiers across.  See **resupply**

The clone soldier function will replace named soldiers and increment their death counts as needed.  It will transfer the diaries over to the new soldiers as well as any nationality / look variant / gender / etc.  It will strip the diaries from dead soldiers, to save on save game space and prefix their name with `..` in the memorial so you can keep track of which soldiers are without clones.  You must have the default names set for this to work.

Soldiers will be 'balanced' across the number of bases you select.  Soldiers will be balanced via class and high/low psi strength if relevant.  After a couple iterations you will have the same number of each soldier 'class' in each base.
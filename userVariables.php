<?PHP
$options = array();

/************************************************
 * User Config                                  *
 *                                              *
 * For best results set autosave freq to 1      *
 ************************************************/

/** use forward slashes instead of backslashes, leave trailing slash in place **/
$root     = 'E:/Games/OpenXcom - Extended/';    //Base oxc install - this is where the 'standard' folder is
$user     = 'user/.XCF/';                       //User Folder - where mods and save folders are
$saves    = 'xcom1/';                           //Save Folder

/** The following is for making your BattleStats visible online **/
  $options['broadcast']   = true;                              // true or false - if true, supply below
  $options['displayName'] = "Display Name";                    // What's your handle?
  $options['passKey']     = "RandomWordsGoHere";               // Just pick a couple random words, not cryptographically secure - DONT USE A PASSWORD
$options['defaultNames'] = array('Case Officer', 'Al', 'K9') // Used in conjunction with the clone function to determine which soldiers are named or not.  Soldiers with these names will not be cloned and will be pulled from training as needed.  Highly recommended that you use a naming mod to change the default names
?>
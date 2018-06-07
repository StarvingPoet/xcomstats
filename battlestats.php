<?PHP

/** check for newer versions **/


require_once('classCLI.inc');
require_once('classBattleStatsClient.inc');
require_once('classYML.inc');
require_once('classMyDirectory.inc');

require_once('websocketBase.inc');
require_once('websocketClient.inc');

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
$options['broadcast']   = true;                                     // true or false - if true, supply below
$options['displayName'] = "Display Name";                           // What's your handle?
$options['passKey']     = "justputawholebunchofrandomwordshere";    // Just pick a couple random words, not cryptographically secure - DONT USE A PASSWORD
try
{
    $client = new battleStatsClient($root, $user, $saves, $options);
    $client->run();
}
catch (Exception $e)
{
    file_put_contents('error.log', print_r($e,true));
}
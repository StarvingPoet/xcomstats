<?PHP

require_once('library/include.inc');
require_once('classBattleStatsClient.inc');
include('userVariables.php');

try
{
    $client = new battleStatsClient($root, $user, $saves, $options);
    $client->run();
}
catch (Exception $e)
{
    file_put_contents($GLOBALS['global']['logPath'].'error.log', print_r($e,true));
}
?>
<?PHP

include ('websocketBase.inc');
include ('websocketClient.inc');

$client = new Client('ws://www.starvingpoet.net:9000');
$client->send('test');
echo $client->receive();

?>
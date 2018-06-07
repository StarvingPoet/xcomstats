<?PHP
class battleStatsClient
{
    var $CLI;

    //var $factions = array('Gals', 'Scallywags', 'LandLubbers');
    //var $factions = array('X-COM', 'Aliens', 'Civilians');
    var $factions = array('Operatives', 'Opposition', 'Civilians');

    var $saves = '';
    var $geo    = '';
    var $battle = '';
    var $geoTime    = 0;
    var $battleTime = 0;
    var $timer      = 0;
    var $alive      = false;

	function __construct($root, $user, $saves, $options = array())
	{
        if (!array_key_exists('broadcast', $options))
        {
            $options['broadcast'] = false;
        }
        $this->CLI = new CLI("Battle Stats Save Game Client", 80, 25);

		/** User Config - End **/
        $this->saves  = $root.$user.$saves;
		$this->geo    = $root.$user.$saves.'_autogeo_.asav';
		$this->battle = $root.$user.$saves.'_autobattle_.asav';

		/* Check for Missing Files */
		if (!is_file($this->geo))
		{
			$this->CLI->clerr('FILE DNE: '. $geo);
		}
		if (!is_file($this->battle))
		{
			$this->CLI->clerr('FILE DNE: '. $battle);
		}

        /** Connect to ws server **/
        $this->ws = new Client('ws://www.openpaedia.com:9000', array('timeout'=>30));


        /** login **/
        $login['method']    = 'whoami';
        $login['broadcast'] = 0;
        if ($options['broadcast'] == true)
        {
            $login['broadcast']   = 1;
            $login['displayName'] = $options['displayName'];
            $login['passKey']     = $options['passKey'];
        }

        $this->send($login);
	}

    private function listen()
    {
        if ($this->ws->listen() > 0)
        {
            $data = $this->ws->receive();
            $this->process($data);
        }
        else
        {
            /** Pretty little sleep nugget **/
            $str = str_repeat('.', $this->timer);
            if ($this->timer == 0)
            {

            }
            $this->CLI->cloutf($str);
            if ($this->timer < 3)
            {
                $this->timer++;
            }
            else
            {
                $this->timer = 0;
            }
            sleep(1);
        }
    }

    /**
     * We have to send arrays in single chunks using hybil 10, otherwise we can send in plain text
     */
    private function send($data) {
        if (is_array($data) || is_object($data)) {       
            $this->CLI->cloutf('sending');
            $this->ws->send(json_encode($data), 'hybil10');            
        }
        else {
            $this->ws->send($data);
        }
    }

    /**
     * Processes data returned from server
     */
    private function process($data)
    {
        $data = trim($data);

        $result = json_decode($data,true);

        if (json_last_error() === 0)
        {
            if ($result['method'] == 'whoareyou')
            {
                $this->alive = true;
            }
        }
        else if ($data == 'cls')
        {
            $this->CLI->cls();
        }
        else /** Catchall - output data **/
        {
            $this->CLI->clout($data.PHP_EOL);
        }

    }


    public function tick()
    {
        /** OVERWITE THIS FUNCTION IF YOU WANT ANY CUSTOM BATTLESCAPE STUFF **/
    }

    private function _tick()
    {
        clearstatcache();
        $geoNow    = filemtime($this->geo);
        $battleNow = filemtime($this->battle);
        $kills      = array();
        $stuns      = array();
        $counts     = array();
        $transmit   = array();

        if ($geoNow > $battleNow)
        {
            if ($geoNow <= $this->geoTime)
            {
                /** Every 3 seconds when idle, check to see if we need to clone **/
                if (time() % 3 == 0)
                {
                    $this->checkForInput();
                }
                return true;
            }
            $this->geoTime = $geoNow;

            $this->CLI->cls();
            $this->CLI->clout("Geoscape Active");

            $save = new YAML($this->geo);            

            $transmit['method']       = 'geoscape';
            $transmit['time']         = $battleNow;
            $transmit['bases']        = $save->YAMLArray[1]['bases'];
            $transmit['deadSoldiers'] = $save->YAMLArray[1]['deadSoldiers'];
        }
        else
        {
            if ($battleNow <= $this->battleTime)
            {
                return true;
            }
            $this->battleTime = $battleNow;

            $this->CLI->cls();
            $this->CLI->clout("Battlescape Active");

            $save = new YAML($this->battle);

            file_put_contents('debug.txt', print_r($save,true));


            $transmit['method']     = 'battlescape';
            $transmit['time']       = $battleNow;
            $transmit['turn']       = $save->YAMLArray[0]['turn'];
            $transmit['units']      = $save->YAMLArray[1]['battleGame']['units'];
        }

        $this->send($transmit);
        
    }

    public function run()
    {
        while (true)
        {
            /** Look for incoming data**/
            $this->listen();

            if ($this->alive === true)
            {
                $this->_tick();
                $this->tick();
            }
        }
    }

    private function checkForInput()
    {
        $saves = new MyDirectory($this->saves, false);
        foreach ($saves->Contents AS $file) {            
            if (stripos($file['name'], 'clone') !== false) {
                echo 'time to clone';
                exit;
            }
            else if (stripos($file['name'], 'resupply') !== false) {           
                echo 'resupply time';
                exit;                
            }
        }        
        $this->CLI->clout(' Check'.PHP_EOL);
    }
}
?>
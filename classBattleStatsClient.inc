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
    var $oxc;

	function __construct($root, $user, $saves, $options = array())
	{
        if (!array_key_exists('broadcast', $options))
        {
            $options['broadcast'] = false;
        }
        //$this->CLI = new CLI("Battle Stats Save Game Client", 80, 24);
        $this->CLI = new CLI("Battle Stats Save Game Client", 80, 80);

		/** User Config - End **/
        $this->root   = $root;
        $this->mods   = $root.$user.'mods';
        $this->saves  = $root.$user.$saves;
		$this->geo    = $root.$user.$saves.'_autogeo_.asav';
		$this->battle = $root.$user.$saves.'_autobattle_.asav';


        /** Initialize OXC Class **/
        $this->loadOXC();
        $this->oxc->defaultNames = $options['defaultNames'];

        //$this->oxc->loadSave('_autogeo_.asav', $this->saves);
        //$this->oxc->fixAmmoManufacturing();

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
            sleep(1);
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
        }
    }

    /**
     * We have to send arrays in single chunks using hybil 10, otherwise we can send in plain text
     */
    private function send($data) {
        if (is_array($data) || is_object($data)) {
            //$this->CLI->cloutf('sending');
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
			$output = array($data, PHP_EOL);
            $this->CLI->clout($output);

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
            if ($geoNow <= $this->geoTime) {
                if($this->timer == 0) /** once every four seconds, check for save **/
                {
                    $this->checkForInput();
                }
                return true;
            }
            $this->geoTime = $geoNow;

            $this->CLI->cls();
			$output = array("Geoscape Active", PHP_EOL);
            $this->CLI->clout($output);

            $save = new YAML($this->geo);

            /** prune data **/
            foreach ($save->YAMLArray[1]['bases'] as $key=>$base) {
                foreach ($base AS $key2=>$data) {
                    if ($key2 != 'soldiers')
                    {
                        unset($save->YAMLArray[1]['bases'][$key][$key2]);
                    }
                }
            }
            foreach ($save->YAMLArray[1]['deadSoldiers'] as $key=>$soldier)
            {
                if (stripos($soldier['name'], '..') !== false) {
                    unset($save->YAMLArray[1]['deadSoldiers'][$key]);
                }
            }

            $transmit['method']       = 'geoscape';
            $transmit['time']         = $battleNow;
            $transmit['bases']        = $save->YAMLArray[1]['bases'];
            $transmit['deadSoldiers'] = $save->YAMLArray[1]['deadSoldiers'];
        }
        else
        {
            if ($battleNow <= $this->battleTime) {
                return true;
            }
            $this->battleTime = $battleNow;

            $this->CLI->cls();
            $this->CLI->clout("Battlescape Active");

            $save = new YAML($this->battle);

            //file_put_contents($GLOBALS['global']['logPath'].'debug.txt', print_r($save,true));


            $transmit['method']     = 'battlescape';
            $transmit['time']       = $battleNow;
            $transmit['turn']       = $save->YAMLArray[0]['turn'];
            $transmit['units']      = $save->YAMLArray[1]['battleGame']['units'];
        }

        $this->send($transmit);
        unset($save);
        unset($transmit);

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
        $return = false;
        foreach ($saves->Contents AS $file) {
            if (stripos($file['name'], 'clone') !== false && $file['ext'] == 'sav') {
                sleep(3); //after finding the file we need to sleep to prevent locking
                $return = $this->cloneSoldiers($file);
                break;
            }
            else if (stripos($file['name'], 'resupply') !== false && $file['ext'] == 'sav') {
                sleep(3);
                $return = $this->resupply($file);
                break;
            }

        }
		unset($saves);
        //$this->CLI->clout(' Check'.PHP_EOL);
    }

    /** returns an array of variables from the file name **/
    private function getVariables($file) {
        $return    = array();
        $clean     = trim(str_ireplace('.'.$file['ext'], '', $file['name']));         /** strip off extension **/


        /** explode on spaces **/
        $parts = explode(' ', $clean);

        if (!is_array($parts)) {
            $this->CLI->clerr('File Naming Error:  `Clone/Resupply $var1 $var2`');
            return false;
        }
        if (strtolower($parts[0]) != 'clone' && strtolower($parts[0]) != 'resupply') {
            $this->CLI->clerr('File Naming Error:  `Clone or Resupply are the only supported commands`');
            return false;

        }
        foreach($parts AS $variable) {
            if (stripos($variable, '=') === false) {  /** skip the first / command variable **/
                continue;
            }
            $temp    = explode('=', $variable);
            $temp[0] = strtolower($temp[0]);

            if (strpos($temp[1], ',') !== false) {/** if variable is comma delimited, explode it as well. **/
                $temp[1] = explode(',', $temp[1]);
                if ($temp[0] == 'base') { /** these arrays start at 0, but people like to start with 1 **/
                    foreach ($temp[1] as $i) {
                        $i = $i - 1;
                    }
                }
            }
            else if (strpos($temp[1], '-') !== false) { /** if variable is a range (x-y), build the array between and including those numbers **/
                $range = explode('-', $temp[1]);
                $temp[1] = array();
                for ($i = $range[0]-1; $i < $range[1]; $i++)
                {
                    $temp[1][] = $i;
                }
            }
            else if ($temp[0] == 'temp') { /** reindex template as well **/
                $temp[1] = $temp[1]-1;
            }
            $return[$temp[0]] = $temp[1];
        }

        if (empty($return))
        {
            $return = false;
        }

		unset($parts);
		unset($temp);

        return $return;
    }

    private function loadOXC() {
        if (!is_object($this->oxc)) {
            $this->oxc = new OXC($this->mods);
            $this->oxc->collectMods($this->root.'standard');
        }
    }

    private function cloneSoldiers($file) {
        $error = true;

        if (!is_array($file)) {
            $this->CLI->clerr('Incorrect $file format');
            return false;
        }
        $this->loadOXC();

        $variables = $this->getVariables($file);

        if (!$variables) {
            $this->CLI->clerr('Clone file name syntax incorrect: clone base=x soldier=x');
            $this->CLI->clerr('Unable to parse '.$file['name']);
            return false;
        }

        /** Do the thing **/
        $this->oxc->loadSave($file['name'], $this->saves);

        $returned = $this->oxc->cloneSoldiers($variables['soldier'], $variables['base']);
        if ($returned !== false) {
            $this->oxc->saveAs('New Soldiers Available', $this->saves);
            if (is_file($file['fullPath'])) {
                sleep(2);
                unlink($file['fullPath']);
            }
            if ($returned != false) {
                $json = array();
                $json['method'] = 'newSoldiers';
                $json['info']   = $returned;
                $this->send($json);
            }
            return true;
        }

    }

    private function resupply($file) {
        if (!is_array($file)) {
            return false;
        }

        $this->loadOXC();
        $variables = $this->getVariables($file);

        if (!$variables) {
            $this->CLI->clerr('Clone file name syntax incorrect: resupply temp=x (base=x)');
            $this->CLI->clerr('Unable to parse '.$file['name']);
            return false;
        }

        $this->CLI->clout(PHP_EOL.'Template: '. $variables['temp']);

        /** Do the thing **/
        $this->oxc->loadSave($file['name'], $this->saves);

        $returned = $this->oxc->resupplyBases(intval($variables['temp'],10), $variables['base']);

        if ($returned !== false) {
            $this->oxc->saveAs('Logistics Finished', $this->saves);
            $this->CLI->clout('file saved');

            if (is_file($file['fullPath'])) {
                sleep(2);
                unlink($file['fullPath']);
            }
            return true;
        }

    }

}
?>
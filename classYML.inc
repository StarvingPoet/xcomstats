<?PHP
/**
 * This page contains the YAML classes
 *
 * @package
 * @author     Nick Green (Poet) <starvingpoet@clan-quant.net>
 * @copyright  2005-2016 Nick Green
 *
 * @version     2016.07.15 Initial Creation
 *
 * @filesource
 * @requires the PECL yaml parser
 *   http://pecl.php.net/package/yaml
 */

class YAML {
    var $fileName   = '';
    var $fileArray  = array();
    var $YAMLArray  = array();
    private $error       = false;
    private $errorStatus = 0;
    private $errorLog    = '';

    public function __construct($fileName, $pos = -1)
    {
        $this->fileName = $fileName;

        if (!is_file($this->fileName))
        {
            $this->_errorHandler(__FUNCTION__, 'File: ('.$this->fileName.') does not exist');
        }
        else
        {
            if (extension_loaded('yaml'))
            {
                $this->YAMLArray = yaml_parse_file($this->fileName, $pos, $ndocs, array(YAML_BOOL_TAG => array($this, 'yaml_tag_callback'),
                                                                                      'true' => array($this, 'yaml_tag_callback')));
            }
            else
            {
                $this->fileArray = explode(PHP_EOL, file_get_contents($this->fileName));
                $this->YAMLArray = $this->YAMLtoArray();
            }
        }

        $this->fileArray = array();
    }

    function __destruct()
    {
        $this->fileArray = NULL;
        $this->YAMLArray = NULL;
    }

    /**
     * This throws an exception that will be caught in the constructor.
     *
     * @method   method the error occured in.
     * @details  text string that will be returned in the email and in the exception.
     * @return   array that will be returned in the errorStatus variable to be processed by the script that called the class
     */
    private function _errorHandler($method, $details, $return = null)
    {
        $this->error       = true;
        $this->errorStatus = 1;
        $this->errorLog    = $GLOBALS['global']['logPath'].'classYML.log';

        $error = date('[Y.m.d H:i:s]').":\t Error in (".$method.") - ".$details."\r\n";
        file_put_contents($this->errorLog, $error, FILE_APPEND);

        //throw new exception ($details, $this->errorStatus);
    }

    /**
     * This is a fix so that y: scalars don't get interpretad as true:
     */
    private function yaml_tag_callback($value, $tag, $flags)
    {
        if ($value === 'y' or $value === 'Y')
        {
            return '"y"';
        }
        else if ($value == 'STR_AKSU74')
        {
            var_dump(func_get_args());

        }
        return;
    }

    /**
     * DEPRECATED - for use without
     * Transforms YAML string into an array.
     * @TODO    Create an object based filereader so I don't need to take this extra step.
     *
     * @offset  Used to keep track of how deep into the file we are recursing.
     * @return  An assosiative array containing the entire file.
     */
    private function YAMLtoArray($offset = 0)
    {
        $return = array();
        $k = null;
		$i = -1;

        /* flip the file array arround and use pop instead of shift to prevent re-indexing significantly speeds up processing of extremely large late-game save games */
        if ($offset === 0)
        {
            $this->fileArray = array_reverse($this->fileArray);
        }

        /* Go by line, delete line as you go to prevent memory bloat */
        for ($i = 0; $line = array_pop($this->fileArray), !is_null($line); $i++)
        {
            //$line   = array_pop($this->fileArray);

            /* Checks to see if the line has the right offset */
            $search = '/^\s{'.$offset.'}(- )?\S+:?/';
            $check1 = preg_match($search, $line, $ret);


            if ($check1 != false)
            {
                /* Check to see if line is an encapsulated string */
                $ret1          = array();
                $escapedString = false;
                $search2       = '/- ".*"$/';
                $check2        = preg_match($search2, $line, $ret1);

                /* else standard use */
                if ($check2 == 0)
                {
                    $search2  = '/:.+$/';
                    $check2   = preg_match($search2, $line, $ret1);
                }
                else
                {
                    $escapedString = true;
                }

                $k = trim(str_replace(':', '', str_replace(' - ', '', $ret[0])));

				/* If line = '---' - only used to demarcate save meta and save data */
				if ($line == '---')
				{
					$return[$i] = $line;
				}
                /* Row is simple key: value pair */
                else if ($check2 != false && stripos($ret[0], "-") === false)
                {
                    $v = trim(str_replace(':','',$ret1[0]));
                    $return[$k] = $v;
                }
                /* if row is just '-' (only used in shipping lane missions )*/
                else if (trim($ret[0]) == '-')
                {
                    $k = $i;
                }
                /* If row is a list of escaped strings */
                else if (stripos($ret[0], str_repeat(" ", $offset)."-") !== false  && $escapedString === true)
                {
                    $v = trim(str_replace("- ","", $ret1[0]));
                    $return[$i] = $v;
                }
                /* If row is a simple list or row is an escaped string */
                else if (stripos($ret[0], str_repeat(" ", $offset)."-") !== false  && stripos($ret[0], ":") === false)
                {
                    $v = trim(str_replace("- ","", $ret[0]));
                    $return[$i] = $v;
                }
                /* if row is the first line in a list of arrays */
                else if (stripos($ret[0], str_repeat(" ", $offset)."-") !== false  && stripos($ret[0], ":") !== false)
                {
                    /* remove the dash and mov the line back onto the array */
                    $line = str_replace("- ", "  ", $line);
                    array_push($this->fileArray, $line);
                    $return[$i] = $this->YAMLtoArray($offset+2);
                }
                /* Row is an empty list */
                else if (stripos($line,'{') !== false)
                {
                    $return[$i] = trim($line);
                }
                /** Row is just a key  - array should start on the next line; here for debug **/
                else
                {
                    //FB::log($line);
                }
            }

            /* if offset is greater, recurse in steps of 2*/
            else if (preg_match('/^\s{'.($offset+2).',}\S+/', $line) != false)
            {
                array_push($this->fileArray, $line);
                $recurse = $this->YAMLtoArray($offset+2);

                if ($i === 0) /* this only occurs if the indent is a multistep */
                {
                    $return = $recurse;
                }
                else
                {
                    $return[$k] = $recurse;
                }
            }
            /* if offset is less, return */
            else if (preg_match('/^\s{0,'.($offset-2).'}\S+/', $line) != false)
            {
                array_push($this->fileArray, $line);
                return $return;
            }
            /* Nothing should be in here */
            else
            {
                $this->_errorHandler(__FUNCTION__, "I shouldn't be:".  @$offset." ".@$ret[0]." -- ".str_replace(" ", "*", $line));
            }
        }

        return $return;
    }

    /**
     * Converts an array back into YAML format
     * Need to do this manually for oxc for some reason.
     *
     * @array   Array to convert
     * @depth   Keeps track of depth during recursion
     *
     * $returns Returns a string
     */
    private function ArraytoYAML($array = null, $depth = 0)
    {
        if (is_null($array))
        {
            $array = $this->YAMLArray;
        }

        foreach ($array AS $k=>$row)
        {
            if ($k == 'y')
            {
                //echo $row.'<br>';
            }

            $key = $k.": ";
            if (is_int($k))
            {
                $key = "- ";
            }

            /* Single - case scenario */
            if ($k == 7 && $depth == 0)
            {
                $row = '--';
            }

            /* Some needed escaping using yaml parser */
            if (is_string($row))
            {
                if (stripos($row, '"') !== false)
                {
                    $row = '"'.str_replace('"', '\"', $row).'"';
                }
                if (stripos($row, ": ") !== false)
                {
                    $row = '"'.$row.'"';
                }
            }
            /** Weird use case any scalar set as 'true' gets parsed to NULL **/
            if (is_null($row) && (strpos($k, 'STR_') !== false || $k == 'training'))
            {
                $row = 'true';
            }

            $array[$k] = str_repeat("  ", $depth).$key;
            if (is_array($row) && strpos($key, "- ") !== false)
            {
                /* Strip excess white space */
                $pattern     = '/-\s+/';
                $replacement = '- ';

                $array[$k] .= $this->ArraytoYAML($row, $depth+1);
                $array[$k] = preg_replace($pattern, $replacement, $array[$k]);
            }
            else if	(is_array($row))
            {
                $array[$k] .= PHP_EOL.$this->ArraytoYAML($row, $depth+1);
            }
            else
            {
                $array[$k] .= $row;
            }
        }
        $string = implode(PHP_EOL, $array);

        /* Only run on final pass fixes a single instance */
        if ($depth === 0)
        {
            $string = str_replace('- --', '---', $string);
            $string = str_replace('- {}', '{}', $string);
			$string = preg_replace('/-{3,}/', '---', $string);
        }

        return $string;
    }

    /**
     * Writes a YAML string from the array to a file
     * note yaml_emit has an indentation problem with lists - use manual encoder
     *
     * @file    Filename to write to, if null, uses original filename
     */
    public function saveFile($file = NULL)
    {
        if (extension_loaded(yaml))
        {
            if (is_numeric(key($this->YAMLArray))) /** this assumes the yaml parser loaded multiple files **/
            {
                foreach ($this->YAMLArray AS $array)
                {
                    ksort($array);
                    $string .= $this->ArraytoYAML($array);
                    $string .= PHP_EOL."---".PHP_EOL;
                }
            }
            else
            {
                $string = $this->ArraytoYAML();
            }
        }
        else
        {
            $string = $this->ArraytoYAML();
        }

        if (is_null($file))
        {
            $file = $this->fileName;
        }
        return file_put_contents($file, $string);
    }
}
?>
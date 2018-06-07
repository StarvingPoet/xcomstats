<?PHP
/**
 * This page contains the CLI Class
 *
 * @package    DQA_Library
 * @author     Nick Green <nick@dqai.com>
 * @copyright  2005-2007 Data Quality Analytics, Inc.
 * @filesource
 */

/**
 * This Class contains all the functions that we need for normal CLI operation
 *
 * @version 2008.2.4 - Initial Creation
 */

class CLI {
    var $cols;
    var $rows;
    var $multiline;

    var $stdout;
    var $stdin;


    /**
     * Old style constructor - defaults are for standard windows cmd line.  $cols = width - 1 to allow for EOL.
     */
    function __construct($title, $cols = 80, $rows = 25)
    {
        $this->stdout = fopen('php://stdout' , 'w');
        $this->stdin  = fopen('php://stdin' , 'r');

        /* Initialie variables */
        $this->cols      = ($cols -1); /** have to shrink by one to prevent Wordwrap **/
        $this->rows      = $rows;
        $this->multiline = 0;

        /** Resize the cmd window **/
        $this->_resize($cols, $rows);

        $this->cls();
        $this->clit($title);
    }

    function __destruct()
    {
        // windows default cmd settings
        //$this->_resize(80,25);
    }

    private function _resize($cols = NULL, $rows = NULL)
    {
        if (is_null($cols))
        {
            $cols = $this->cols;
        }
        if (is_null($rows))
        {
            $rows = $this->rows;
        }
        shell_exec(escapeshellcmd("mode con: cols=".$cols." lines=".$rows));
    }


    /**
     * This saves some typing, simply a cli-fwrite echo
     * @=echo can be a string or array of strings.
     */
    function clout($echo, $truncate = true, $cols = NULL)
    {
        if (is_array($echo) || (strlen($echo) > $this->cols && $truncate == false))
        {
            $this->clmulti($echo, $truncate, $cols);
        }
        else
        {
            /* Reset any previous multiline output */
            $this->multiline = 0;
            fwrite($this->stdout, $echo);
        }
        @ob_flush();
    }

    /**
     * This is a formatted clout designed to span an entireline.
     */
    function cloutf($echo)
    {
        fwrite($this->stdout, sprintf("\r%-79.79s", $echo));
    }

    /**
     * This is a formatted clout designed to span an entireline and truncate extra large lines.
     */
    function cloutft($echo)
    {
        $length = strlen($echo);
        if ($length > 79)
        {
            $end = substr($echo, $length-9);

            $difference = $length - 79;

            for ($i = 0; $i <= $difference ; $i++)
            {
                if ($i > 5)
                {
                    break;
                }
                $middle .= '.';
            }

            $stop = 70-$i;

            $beginning = substr($echo, 0, $stop);
            $echo = $beginning.$middle.$end;
        }
        fwrite($this->stdout, sprintf("\r%-79.79s", $echo));
    }

    /* Clears out the current line and moves the cursor up one row */
    function clearLine()
    {
        /** formula for sprintf to fill the entire line **/
        $format = "%".$this->cols."s";

        fwrite($this->stdout, chr(27) . "[0G"); // Set cursor to the first col
        fwrite($this->stdout, sprintf($format, ""));
        fwrite($this->stdout, chr(27) . "[1A"); // Set cursor up one row
    }

    function clearLineTest()
    {

        /** formula for sprintf to fill the entire line **
        $format = "%".$this->cols."s";

        fwrite($this->stdout, "\$e[0G"); // Set cursor to the first col
        fwrite($this->stdout, sprintf($format, ""));
        fwrite($this->stdout, "\$e[1A"); // Set cursor up one row
        */
    }

    /*
     * Windows CMD friendly CLS
     */
    function cls()
    {
        fwrite($this->stdout, chr(27).chr(91).'H'.chr(27).chr(91).'J');

        /* work back up cleaning every line as you go, do a few extra to make sure*/
        for ($i = -5; $i <= $this->rows; $i++)
        {
            $this->clearLineTest();
        }
    }

    /**
     * Multiline output that overwrites itself
     * Original idea from nolanpro: http://stackoverflow.com/questions/4320081/clear-php-cli-output/11424357
     *
     * @str Multiline strine
     * @col Number of columns - windows default of (80-1)
     * @truncate Whether or not to truncate long lines.
     */
    function clmulti($str, $truncate = true, $cols = NULL)
    {
        if (is_null($cols))
        {
            $cols = $this->cols;
        }

        /** formula for sprintf to fill the entire line **/
        $format = "\r%-".$cols.".".$cols."s";

        /** fix extra long lines */
        if (is_string($str))
        {
            $output = explode("\n", $str);
        }
        else if (is_array($str))
        {
            $output = $str;
        }
        else
        {
            $this->clerr('clmulti: invalid $str type');
            return false;
        }
        $str = NULL;

        for ($i = 0; $i < count($output); $i++)
        {
            if (strlen($output[$i]) >= $cols)
            {
                if ($truncate == true)
                {
                    $output[$i] = substr($output[$i], 0, $cols);
                }
                else
                {
                    $wordWrap = str_split($output[$i], $cols);
                    array_splice($output, $i, 1, $wordWrap);
                    unset($wordWrap);
                }
            }

            /* Format each line to take up the full space */
            $output[$i] = sprintf($format, $output[$i]);
        }

        /* if previous multiline entry was larger than this one, pad out the old lines */
        for ($y; $y < $this->multiline; $y++)
        {
            $this->clearLine();
        }
        $this->multiline = $i-1;

        /*  And, finally, output the line */
        $str = implode("\n", $output);
        fwrite($this->stdout,"\r"); // Set cursor to first column
        fwrite($this->stdout,$str);

        $output = NULL;
        $str    = NULL;

        return true;
    }

    /**
     * This saves some typing, simply a cli-fwrite echo that is meant to be sent to a log
     */
    function clog($echo)
    {
        fwrite($this->stdout, "* ".$echo);
    }

    /**
     * This sets a window and prefixes the output
     */
    function clit($title)
    {
        $length = strlen($title);

        if ($length > 0)
        {
            $cmd = "TITLE ".$title;
            shell_exec(escapeshellcmd($cmd));

            $this->clout("\r");
            $this->clout(str_repeat("-", $length)."\n");
            $this->clout($title."\n");
            $this->clout(str_repeat("-", $length)."\n");
        }
    }

    /**
     * This takes input and clears the EOL from the input
     */
    function clint(&$return, $length)
    {
        $echo = $this->clint($echo, $length);
        $echo = trim($echo);
        $return = $echo;
    }

    /**
     * outputs a formatted error message
     */
    function clerr($error)
    {
        $error = "** ".$error." **";
        $length = strlen($error);

        if ($length > 79)
        {
            $length = 79;
            $error = substr($error,0,79);
        }

        $this->clout("\r");
        $this->clout(str_repeat("*", $length)."\n");
        $this->clout($error."\n");
        $this->clout(str_repeat("*", $length)."\n");
    }

    /**
     * This method prints the Menu arrays to the screen
     */
    function menu($menuArray, $title, $maximize = FALSE)
    {
        if ($maximize)
        {
            $max = 81;
        }
        else
        {
            $max = 23;
        }

        $width = strlen($title);

        $echo  = $title ."\n";
        $echo .= str_repeat("-", $width);
        $echo .= "\n";

        $i = 0;
        foreach ($menuArray AS $key=>$menuRow)
        {
            $i++;
            if ($i == $max)
            {
                $i = 1;
                $echo .= "  -- MORE --";
                $this->clout($echo);
                $temp = $this->clint(2);
                $echo = "\n";
            }

            if (!is_numeric($key))
            {
                $i++;
                $line = "\n-- ".$menuRow."\n";
            }
            else
            {
                $line = "(".$key.")";
                if ($key < 10)
                {
                    $line .= "   ";
                }
                else if ($key < 100)
                {
                    $line .= "  ";
                }
                else
                {
                    $line .= " ";
                }
                $line .= "- ".$menuRow."\n";
            }

            $echo .= $line;
            if (strlen($line) > $width)
            {
                $width = strlen($line);
            }
        }

        /* pad small tables */
        for ($i; $i < ($max - 3); $i++)
        {
            $echo .= "\n";
        }

        $echo .= "\n";
        $echo .= str_repeat("-", $width);
        $echo .= "\n";
        $echo .= "\rMenu Selection: ";
        $this->clout($echo);
        $this->clint($return, 50);

        while (!(array_key_exists($return, $menuArray)))
        {
            $this->clout("\rInvalid Selection - Please try again: ");
            $this->clint($return, 50);
        }

        return $return;
    }

    /**
     * This is only used for the LTLFinder site searcher - forks all the scrapes into their own process
     * and runs them simultaneously.  We could use psexec, but this waits for them all to be done before
     * unblocking.
     */
    function forkCLIScript($path, $script, $id, $sites)
    {
        /*
        $batch = $GLOBALS['global']['tempdir'].$script.'_'.$id.'.bat';

        if (!is_file($batch))
        {
            $bat  = "@echo off\r\n";
            $bat .= "cls\r\n";
            $bat .= substr($path, 0, 2)."\r\n";
            $bat .= "cd \"".$path."\"\r\n";
            foreach($sites AS $scraper)
            {
                $bat .= "start php.exe -f \"".$script."\" ".$id." ".$scraper."\r\n";
            }

            file_put_contents($batch, $bat);
        }
        exec($batch);
        */

        foreach($sites AS $scraper)
        {
            $cmd = 'psexec.exe /accepteula -d php.exe -f "'.$path.'/'.$script.'" '.$id.' '.$scraper.' '.$GLOBALS['global']["Redirect"];

            exec($cmd);

            //$bat .= "start php.exe -f \"".$script."\" ".$id." ".$scraper."\r\n";
        }
    }

    /**
     * This runs a php script in the same window with line parsing
     */
    function runCLIScript($path, $script, $arguments = "", $verbose = FALSE, $log = FALSE, $fork = FALSE)
    {
        if ($verbose == TRUE)
        {
            $direct = " 2>&1";
        }
        else
        {
            $direct = $GLOBALS['global']['Redirect'];
        }


        if ($log)
        {
            $fh = fopen($GLOBALS['global']['tempdir']."CLI.log", "w");
        }

        if ($fork)
        {

        }

        $descriptorspec = array(0 => array("pipe", "r"),
                                1 => array("pipe", "w"),
                                2 => array("pipe", "w"));
        $env = NULL;

        $procString = 'php.exe -f '.$path.$script." ".$arguments.$direct;
        print_r($procString);
        $process = proc_open($procString, $descriptorspec, $pipes, $path, $env);

        if (is_resource($process))
        {
            // $pipes now looks like this:
            // 0 => writeable handle connected to child stdin
            // 1 => readable handle connected to child stdout
            // Any error output will be appended to /tmp/error-output.txt

            $blink = TRUE;
            $this->clout("\n");
            while(!feof($pipes[1]))
            {
                $echo = fgets($pipes[1], 512);
                $echo = rtrim($echo);

                /** if the line is empty, we sleep for a bit **/
                if (strlen($echo) == 0)
                {
                    winSleep(50);
                    continue;
                }

                /** If logging is enabled, we send the lind to the lag parser **/
                if ($log)
                {
                    $this->publishLog($fh, $echo);
                }

                /** if there is an HTML tag in the line, we send the line to the HTML parse **/
                if (!(strpos($echo, "<") === FALSE))
                {
                    $echo = $this->parseHTMLtoCLI($echo);
                }
                else
                {
                    $echo = $this->parseCLI($echo);
                }
                $this->checkError($echo);
                $this->clout($echo);

                //winSleep(1);
            }

            fclose($pipes[0]);
            fclose($pipes[1]);
            fclose($pipes[2]);

             if ($log)
             {
                 fclose($fh);
             }


            // It is important that you close any pipes before calling
            // proc_close in order to avoid a deadlock
            $return = proc_close($process);
        }
        sleep(2);
        return $return;
    }

    function parseHTMLtoCLI($echo)
    {
        /** convert tags to Ascii characters **/
        //$echo = str_ireplace("\n"              , ""       ,$echo);
        $echo = str_ireplace("<br>"            , "<br />" , $echo);
        $echo = str_ireplace("<br />"          , "\n"     , $echo);
        $echo = str_ireplace("</td><td>"       , "\t"     , $echo);
        $echo = str_ireplace("</tr><tr>"       , "\n"     , $echo);
        $echo = str_ireplace("</p>"            , "\n"     , $echo);
        //$echo = str_ireplace("\r"              , "\n"     , $echo);
        $echo = eregi_replace("<(/?)b[^\>]*\>" , "***"    , $echo);



        /** clean unwanted tags **/
        //$echo = eregi_replace("<(/?)[^\>]*\>"        , "", $echo);
        $echo = eregi_replace("<(/?)a[^\>]*\>"        , "", $echo);
        $echo = eregi_replace("<(/?)p[^\>]*\>"        , "", $echo);
        $echo = eregi_replace("<(/?)div[^\>]*\>"      , "", $echo);
        $echo = eregi_replace("<(/?)font[^\>]*\>"     , "", $echo);
        $echo = eregi_replace("<(/?)form[^\>]*\>"     , "", $echo);
        $echo = eregi_replace("<(/?)caption[^\>]*\>"  , "", $echo);
        $echo = eregi_replace("<(/?)h[^\>]*\>"        , "", $echo);
        $echo = eregi_replace("<(/?)img[^\>]*\>"      , "", $echo);
        $echo = eregi_replace("<(/?)input[^\>]*\>"    , "", $echo);
        $echo = eregi_replace("<(/?)iframe[^\>]*\>"   , "", $echo);
        $echo = eregi_replace("<(/?)nobr[^\>]*\>"     , "", $echo);
        $echo = eregi_replace("<(/?)span[^\>]*\>"     , "", $echo);
        $echo = eregi_replace("<(/?)strong[^\>]*\>"   , "", $echo);
        $echo = eregi_replace("<(/?)sup[^\>]*\>"      , "", $echo);
        $echo = eregi_replace("<(/?)table[^\>]*\>"    , "", $echo);
        $echo = eregi_replace("<(/?)tbody[^\>]*\>"    , "", $echo);
        $echo = eregi_replace("<(/?)th[^\>]*\>"       , "", $echo);
        $echo = eregi_replace("<(/?)td[^\>]*\>"       , "", $echo);
        $echo = eregi_replace("<(/?)tr[^\>]*\>"       , "", $echo);
        $echo = eregi_replace("<(/?)style[^\>]*\>"    , "", $echo);
        $echo = eregi_replace("<(/?)script[^\>]*\>"   , "", $echo);
        $echo = eregi_replace("<(/?)noscript[^\>]*\>" , "", $echo);
        $echo = eregi_replace("<(/?)pre[^\>]*\>" , "", $echo);
        $echo = eregi_replace("<\!(/?)[^\>]*--\>"     , "", $echo);

        $echo = str_ireplace("<>"              , "" , $echo);
        $echo = ereg_replace("\n{3,}"          , "\n" , $echo);


        /** create an array of lines **/
        $echoArray = explode("\n", $echo);

        /** set max length of each line **/
        $line = "";
        $empty = 0;
        foreach ($echoArray AS $echo)
        {
            $echo = trim($echo);
            $echo = substr($echo, 0, 79);


            /** Parse Audtor Scrape Section **/
            if (!(strpos($echo, "***File***") === FALSE))
            {
                $echo = str_replace("***File***", "\n\n***File***", $echo);
                $temp = explode("\t", $echo);
                $echo = $temp[0]."\t\t\t\t\t".$temp[1]."\t".$temp[2];
            }

            if (!(strpos($echo, ".csv") === FALSE))
            {
                $temp = explode("\t", $echo);
                $temp2 = explode("_", $temp[0]);

                /** Fix BBC **
                if ($temp2[2] == "BBC" || $temp2[3] == "US")
                { */
                    $temp2[2] = $temp2[2]."_".$temp2[3];
                //}


                if (strlen($temp[0]) < 32)
                {
                    $echo = $temp[0]."\t\t\t\t".$temp[1]."\t\t".$temp[2];
                }
                else if (strlen($temp[0]) < 40)
                {
                    $echo = $temp[0]."\t\t\t".$temp[1]."\t\t".$temp[2];
                }
                else if (strlen($temp[0]) > 47)
                {
                    $echo = $temp[0]."\t".$temp[1]."\t\t".$temp[2];
                }
                else
                {
                    $echo = $temp[0]."\t\t".$temp[1]."\t\t".$temp[2];
                }
            }

            /** Count Empty Output **/
            if (strlen($echo) == 0 && $empty < 2)
            {
                $empty++;
            }
            if ($empty == 0 || $empty == 2)
            {
                $line .= $echo."\n";
            }
        }

        return $line;
    }

    function publishLog($handle, $echo)
    {
        if ($echo != "")
        {
            $echo = str_replace("\r", "\n", $echo);
            $echoArray = explode("\n", $echo);
            foreach ($echoArray AS $line)
            {
                $line = eregi_replace("<(/?)[^\>]*\>"        , "", $line);
                $line = trim($line);
                if (strlen($line) !== 0)
                {
                    fwrite($handle, date("[Y.m.d H:i:s] ").$line."\n");
                }
            }
        }
    }

    function parseCLI($echo)
    {
        $echoArray = explode("\n", $echo);
        foreach ($echoArray AS $line)
        {
            $line = rtrim($line);
            $return .= substr($line, 0, 78)."\n";
        }

        return $return;
    }

    function checkError($echo)
    {
        if (!(stripos($echo, "error") === FALSE))
        {
            exec("COLOR 4F");
        }
    }

    /**
     * This should offer a screen by screen print_r function for CLI
     */
    function viewArray($data)
    {
        if (is_array($data))
        {
            $i = count($data);
        }

        foreach ($data AS $key=>$row)
        {
            $n++;

            $this->cloutf("[".$key."]".var_export($row,TRUE));
            $this->clout("\n");

            if ($n % 26 == 0)
            {
                $this->clout("Press any key to continue");
                $this->clint($null, 5);
                $n = 0;
            }
        }
    }
}

?>
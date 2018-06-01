<?PHP
/**
 * This class controls an easy way to parse directories
 *
 * @package
 * @author     Nick Green <nick@dqai.com>
 * @copyright  2009
 * @filesource
 *
 * @version    2009.02.23 - No idea when this was originally made, pulled from DQAI root.
 */

class MyDirectory {
	var $Path = "";
	var $Contents = array();

	/*********************************************************************
	Returns array with following info:
	FullPath, (File)Name, (File)Extension, (File)Path, DateModified, Type

	Called By: Contents()
	**********************************************************************/
	function File($Path)
	{
		$result = array();
		$result["fullPath"] = $Path;
		$pathinfo = pathinfo($Path);

		/** Fix undefined index notice for files without extensions **/
		$desired_keys = array("basename"=>"name", "extension"=>"ext", "dirname"=>"path");
		foreach ($desired_keys AS $key=>$name)
		{
			if(isset($pathinfo[$key]))
			{
				$result[$name] = $pathinfo[$key];
			}
		}

		$result["dateModified"] = filemtime($Path);
		if (is_dir($Path))
		{
			$result["type"] = "Directory";
	    }
		else
		{
			$result["type"] = "File";
		}
		return $result;
	}

	/*********************************************************************
	Returns sorted array with following info:
    File(array) for each file in directory

	Called By: Self, MyDirectory()
	**********************************************************************/
	function Contents($Path, $recurse)
	{
		$result = array();
		$dh = opendir("$Path");
		while (($FileName = readdir($dh)) !== false)
		{
			if (($FileName == '.') || ($FileName == '..'))
			{
				continue;
			}
			$FilePath = $Path."/".$FileName;
			$result[] = $this->File($FilePath);
			if (is_dir($FilePath) && $recurse === true)
			{
				$result = array_merge($result,$this->Contents($FilePath, $recurse));
			}
		}
		closedir($dh);
		sort($result);
		return $result;
	}

	/*********************************************************************
    Sets Contents to be an array of Contents($Paths)
	**********************************************************************/
	function MyDirectory($Path, $recurse = true)
	{
		$this->Path = $Path;
		$this->Contents = $this->Contents($Path, $recurse);
	}

	/*********************************************************************
    Deletes Files and/or Directories @ $this->Path

    @param $fileglob mixed If string, must be a file name (foo.txt), glob pattern (*.txt), or directory name.
                           If array, must be an array of file names, glob patterns, or directories.

    Written By: bishop @ www.php.net
	**********************************************************************/
	function Remove($fileglob="")
	{
	   if ($fileglob == "")
	   {
		   $fileglob = $this->Path;
	   }

	   if (is_string($fileglob))
	   {
		   if (is_file($fileglob))
		   {
			   return unlink($fileglob);
		   }
		   else if (is_dir($fileglob))
		   {
			   $ok = $this->Remove("$fileglob/*");
			   if (! $ok)
			   {
				   return false;
			   }
			   return rmdir($fileglob);
		   }
		   else
		   {
			   $matching = glob($fileglob);
			   if ($matching === false)
			   {
				   trigger_error(sprintf('No files match supplied glob %s', $fileglob), E_USER_WARNING);
				   return false;
			   }
			   $rcs = array_map(array('MyDirectory', 'Remove'), $matching);
			   if (in_array(false, $rcs))
			   {
				   return false;
			   }
		   }
	   }
	   else if (is_array($fileglob))
	   {
		   $rcs = array_map(array('MyDirectory', 'Remove'), $fileglob);
		   if (in_array(false, $rcs))
		   {
			   return false;
		   }
	   }
	   else
	   {
		   trigger_error('Param #1 must be filename or glob pattern, or array of filenames or glob patterns', E_USER_ERROR);
		   return false;
	   }
	   return true;
	}

    /**
     * This is pretty damn slow, also dependant on the order of files
     */
    public static function MD5Dir($dir)
    {
        if (!is_dir($dir))
        {
            return false;
        }
        
        $filemd5s = array();
        $d = dir($dir);

        while (false !== ($entry = $d->read()))
        {
            if ($entry != '.' && $entry != '..')
            {
                 if (is_dir($dir.'/'.$entry))
                 {
                     $filemd5s[] = MyDirectory::MD5Dir($dir.'/'.$entry);
                 }
                 else
                 {
                     $filemd5s[] = md5_file($dir.'/'.$entry);
                 }
             }
        }
        $d->close();
        return md5(implode('', $filemd5s));
    }
}
?>
<?php

//FIXME only for png at the moment !! 

$GLOBALS["ext"] = "png";

//usage php _genmaterial_.php > materials.cs

function getFileExt($filename)
{
	$arr = explode('.', $filename);
	$cnt = count($arr);
	if ($cnt>1)
	{
		return strtolower($arr[$cnt-1]);
	}
	return "";
}



function isNM($filename)
{
    if (strpos($filename,"_nm.".$GLOBALS["ext"]) > 0)
		return true;
	return false;
}

/* example:
singleton Material(tset_12_arms_Material)
{
   mapTo = "tset_12_arms";
   diffuseMap[0] = "art/shapes/avatar/tset_12_arms";
   normalMap[0] = "art/shapes/avatar/tset_12_arms_nm.jpg";
   materialTag0 = "Avatar";
   castDynamicShadows = "1";
};
*/


$l = scandir("./");


$matText = "";
foreach ($l as $f)
{
    if (getFileExt($f) == $GLOBALS["ext"] )
	{
	    $bn = basename($f,".".$GLOBALS["ext"]);
		
		
		
		if (isNM($f))
		{
			//echo("NORMALMAP!");
			continue;
		}
		
        	$matName = preg_replace('/\./', '__', $bn."_Material");
		
	    $temp="";
	    $temp.="singleton Material(".$matName.")\n";
	    $temp.="{\n";
	    $temp.="\tmapTo = \"".$bn."\";\n";
	    $temp.="\tdiffuseMap[0] = \"./".$bn."\";\n";
	    if (in_array($bn."_nm".$GLOBALS["ext"],$l)) {
	    	$temp.="\tnormalMap[0] = \"./".$bn."_nm\";\n";	
	    }
	    $temp.="\tmaterialTag0 = \"PlayerMats\";\n";
//no!	    $temp.="\tcastShadows = \"0\";\n";
	    $temp.="\tcastDynamicShadows = \"1\";\n";
	    $temp.="};\n";
	     
	    
	    $matText .= $temp;
	    
		//echo($f."\n");
		
	}
} //foreach
echo($matText);


?>

<?php
/**
 * @author       Mayank Jain<mayankkrjain@gmail.com>
 * @about        Developed for the Sahana Foundation (GSOC 13). 
 * @link         http://sahanafoundation.org
 * @license      http://www.gnu.org/licenses/lgpl-2.1.html GNU Lesser General Public License (LGPL)
 * @lastModified 2013.1907
 */



global $global;
//path of sahana.conf file
$global['confroot']   =   realpath(dirname(dirname(dirname(__FILE__))));
$global['approot']  =   realpath(dirname(dirname(dirname(__FILE__)))).'/www/../';
require($global['approot'].'conf/sahana.conf');
require($global['confroot'].'/inc/handler_db.inc');

// Disaster Category
$category=mysql_real_escape_string($_GET['category']);

//Incident_id
$incident_id = intval(mysql_real_escape_string(($_GET['inc_id'])));
$q="
	INSERT INTO disaster_category
	(name)
	VALUES
	('".$category."');	
	";
	$result=$global['db']->Execute($q);
if ($result==false)
{ 
	daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "insert new category 1");
}
//getting list of categories associated with this disaster --- pre-selected values
$names=array();
$q= " 
SELECT *
FROM scenario 
WHERE `incident_id` = ".$incident_id.";
";
$categories = $global['db']->Execute($q);
if($categories === false) 
{
	daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "generate category list");
}
while($type_id = $categories->FetchRow()){
	//get the name of the disaster category from ids
	$q="
		SELECT *
		FROM disaster_category
		WHERE `category_id` =".$type_id['disaster_type'].";
	";
	$type_name = $global['db']->Execute($q);
	if($type_name === false) 
	{ 
		daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "generate category list2");
	}
	$name=$type_name->FetchRow();
	$names[]=$name['name'];
}
//Creating main category_select list
$q="
SELECT *
FROM disaster_category
ORDER BY name;
";
$categories=$global['db']->Execute($q);
$cats='';
$cats.="<select data-placeholder=\"Select from Existing Categories\" class=\"chzn-select-no-results\" name=\"rez_select\" multiple  id= \"chzn-select-no-results\" style=\"width: 300px\";";
if($categories === false) 
{ 
	daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "generate category list3"); 
}
while($name=$categories->FetchRow())
{
	if(in_array($name['name'],$names))
	{
		$cats.='<option selected value="'.$name['category_id'].' ">' . $name['name'] . '</option>';
	}
	else
	{
		$cats.='<option value="'.$name['category_id'].'">' . $name['name'] . '</option>';
	}
}
$cats.='</select>';
echo $cats;

//Error Logging Function
function daoErrorLog($file, $line, $method, $class, $function, $errorMessage, $other) {

	global $global;
	$db = $global['db'];
	$q = "
		INSERT INTO dao_error_log (
				file,
				line,
				method,
				class,
				function,
				error_message,
				other )
		VALUES (
				'".mysql_real_escape_string((string)$file)."',
				'".mysql_real_escape_string((string)$line)."',
				'".mysql_real_escape_string((string)$method)."',
				'".mysql_real_escape_string((string)$class)."',
				'".mysql_real_escape_string((string)$function)."',
				'".mysql_real_escape_string((string)$errorMessage)."',
				'".mysql_real_escape_string((string)$other)."' );
	";
	$result = $db->Execute($q);
}

?>

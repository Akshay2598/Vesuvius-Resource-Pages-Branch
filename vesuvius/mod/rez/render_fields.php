<?php
/**
 * @author       Mayank Jain<mayankkrjain@gmail.com>
 * @about        Developed for the Sahana Foundation (GSOC 13)
 * @link         http://sahanafoundation.org
 * @license      http://www.gnu.org/licenses/lgpl-2.1.html GNU Lesser General Public License (LGPL)
 * @lastModified 2013.1907
 */

global $global;
$global['confroot']   =   realpath(dirname(dirname(dirname(__FILE__))));
$global['approot']  =   realpath(dirname(dirname(dirname(__FILE__)))).'/www/../';

require($global['approot'].'conf/sahana.conf');
require($global['confroot'].'/inc/handler_db.inc');
$actual =intval(mysql_real_escape_string(($_GET['actual'])));
$type =(mysql_real_escape_string(($_GET['type'])));
$id = intval(mysql_real_escape_string(($_GET['id'])));
if ($type == 'suggest_page')
{
	//Revert back to original page data
	if ($actual == 1)
	{
		$q="
			select *
			from rez_pages
			where rez_page_id='".$id."';
		";
		$result = $global['db']->Execute($q);
		if($result === false) {
			daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez edit get_disaster2");
		}
		$row=$result->FetchRow();
		echo"<titles>".$row['rez_menu_title']."</titles><textarea>".$row['rez_content']."</textarea>";

	}
	//Show the content of the selected page template
	elseif ($actual == 2)
	{
		$q="
			select *
			from rez_pages
			where rez_page_id='".$id."';
		";
		$result = $global['db']->Execute($q);
		if($result === false) {
			daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez edit get_disaster2");
		}
		$row=$result->FetchRow();
		echo"<titles>".$row['rez_menu_title']."</titles><textarea>".$row['rez_content']."</textarea>";
	}
}
elseif ($type == 'suggest_template'){
	$template_id = intval(mysql_real_escape_string(($_GET['id'])));
	$q="
		select *
		from rez_page_template
		where rez_template_id='".$template_id."';
	";
	$result = $global['db']->Execute($q);
	if($result === false) {
		daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez edit get_disaster2");
	}
	$row=$result->FetchRow();
	$content=htmlentities($row['rez_template_content']);
	echo html_entity_decode("<titles>".$row['rez_menu_title']."</titles><textarea>".$content."</textarea>");
}
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

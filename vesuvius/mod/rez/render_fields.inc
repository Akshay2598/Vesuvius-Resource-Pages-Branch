<?php
/**
 * @author       Mayank Jain<mayankkrjain@gmail.com>
 * @about        Developed for the Sahana Foundation (GSOC 13)
 * @link         http://sahanafoundation.org
 * @license      http://www.gnu.org/licenses/lgpl-2.1.html GNU Lesser General Public License (LGPL)
 * @lastModified 2013.13.09
 */

function render_fields($actual,$type,$id){
	global $global;
	$q="
		select *
		from incident
		where shortname ='".mysql_real_escape_string($_GET['shortname'])."';
	";
	$result = $global['db']->Execute($q);
	if($result === false) {
		daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez edit get_event");
	}
	$event_row = $result->FetchRow();
	$event_name=$event_row['name'];

	if ($type == 'suggest_page'){
		//Revert back to original page data
		if ($actual == 1)		{
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
			$content=json_encode($row['rez_content']);
			$title = ($row['rez_menu_title']);
			return array($title,$content);

		}
		//Show the content of the selected page template
		elseif ($actual == 2)		{
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
			$content=json_encode($row['rez_content']);
			$title = $row['rez_menu_title'];
			return array($title,$content);
		}
	}
	elseif ($type == 'suggest_template'){
		$template_id = $id;
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
		$content=json_encode($row['rez_template_content']);
		$title = $event_name . ' : ' . ($row['rez_menu_title']);
		return array($title,$content);
	}
}
function show_template_list($template_id){
	//This function genrates a list of child-pages for a given template
	global $global;
	$q="
		SELECT *
		FROM rez_pages
		WHERE parent_template_id = ".$template_id.";
	";
	$result = $global['db']->Execute($q);
	if($result === false) {
		daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez show_event list");
	}
	$select="<select data-placeholder=\"Select from Existing Pages\" class=\"chzn-select-no-results\" name=\"page_change_select\" multiple  id= \"chzn-select-no-results\" style=\"width: 800px\";";
	$select.='<option value=""></option>';
	while($rows = $result->FetchRow()){
			$select.='<option value="'.$rows['rez_page_id'].'">'.$rows['rez_menu_title'].'</option>';
	}
	$select.='</select>';
	return $select;

}
function show_event_list($page_id){
	global $global;
	$q="
		select *
		from incident
		where shortname ='".mysql_real_escape_string($_GET['shortname'])."';
	";
	$result = $global['db']->Execute($q);
	if($result === false) {
		daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez edit get_event");
	}
	$event_row = $result->FetchRow();
	$current_event_name=$event_row['name'];
	$current_event_id=$event_row['incident_id'];
	$names=array();
	$q="
		SELECT *
		FROM disaster_category;
	";
	$result = $global['db']->Execute($q);
	if($result === false) {
		daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez show_event list");
	}

	while($rows = $result->FetchRow()){
		$names[$rows['category_id']] = $rows['name'];
	}

	$events = array();
	$q="
		SELECT *
		FROM incident;
	";
	$result = $global['db']->Execute($q);
	if($result === false) {
		daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez show_event list");
	}
	while($rows = $result->FetchRow()){
		$events[$rows['incident_id']] = $rows['name'];
	}

	$select="<select data-placeholder=\"Select from Existing Events\" class=\"chzn-select-no-results\" name=\"event_select\" multiple  id= \"chzn-select-no-results\" style=\"width: 800px\";";
	$select.='<option value=""></option>';
	$q="
		SELECT *
		FROM rez_incident
		WHERE page_id =".$page_id.";
	";
	$page_event=array();
	$result = $global['db']->Execute($q);
	while($rows =$result->FetchRow()){
		$page_event[]=$rows['incident_id'];
	}
	$q="
		SELECT *
		FROM scenario
		ORDER BY disaster_type;
	";

	$prev ='';
	$shown=array();
	$result = $global['db']->Execute($q);
	while($rows = $result->FetchRow()){
		if ($rows['disaster_type'] !== $prev)			{
			if ($prev !== '')				{
				$select.='</optgroup>';
			}
			$select.='<optgroup label ="'.$names[$rows['disaster_type']].'">';
			$prev=$rows['disaster_type'];
		}
		if (in_array($rows['incident_id'],$page_event)){
			$select.='<option selected value="'.$rows['incident_id'].'">'.$events[$rows['incident_id']].'</option>';
			$page_event = array_diff($page_event, array($rows['incident_id']));
			$shown[]=$rows['incident_id'];
		}
		elseif (($rows['incident_id'] == $current_event_id) and !(in_array($rows['incident_id'],$shown)))
			{
				$select.='<option selected value="'.$current_event_id.'">'.$current_event_name.'</option>';
				$shown[]=$current_event_id;
			}
		else {
			$select.='<option value="'.$rows['incident_id'].'">'.$events[$rows['incident_id']].'</option>';
		}
	}
	$select.='</select>';
	return $select;


}

?>

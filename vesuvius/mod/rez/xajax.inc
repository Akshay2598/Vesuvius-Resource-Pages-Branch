<?
/**
 * @name         Resources
 * @version      16
 * @package      rez
 * @author       Greg Miernicki <g@miernicki.com> <gregory.miernicki@nih.gov>
 * @about        Developed in whole or part by the U.S. National Library of Medicine
 * @link         https://pl.nlm.nih.gov/about
 * @license	 http://www.gnu.org/licenses/lgpl-2.1.html GNU Lesser General Public License (LGPL)
 * @lastModified 2012.0213
 */

global $global;
require_once($global['approot'].'/mod/lpf/lib_lpf.inc');
require_once($global['approot'].'/mod/rez/handler_locale.inc');
require_once($global['approot'].'/mod/rez/render_fields.inc');

// create an array as it does not exist previosuly and will make warnings
$global['xajax_functions'] = array();

// publicly register accessible xajax funtions
array_push($global['xajax_functions'],'rez_append_log');
array_push($global['xajax_functions'],'rez_prepend_log');
array_push($global['xajax_functions'],'rez_show_message');
array_push($global['xajax_functions'],'rez_render_fields');
array_push($global['xajax_functions'],'rez_show_pages');
array_push($global['xajax_functions'],'rez_show_templates');
array_push($global['xajax_functions'],'rez_show_help');
array_push($global['xajax_functions'],'rez_perform_edit');
array_push($global['xajax_functions'],'rez_get_translation');
array_push($global['xajax_functions'],'rez_perform_delete');
array_push($global['xajax_functions'],'rez_perform_translate');
array_push($global['xajax_functions'],'rez_perform_insert');
array_push($global['xajax_functions'],'rez_perform_save');
array_push($global['xajax_functions'],'rez_template_save');
array_push($global['xajax_functions'],'rez_perform_move_up');
array_push($global['xajax_functions'],'rez_perform_move_down');
array_push($global['xajax_functions'],'rez_perform_change_visibility');
array_push($global['xajax_functions'],'rez_perform_change_locale');

//Initialize locale array
global $global;
//@TODO: This should be built on a query.
$global['rez_locales'] = array(
		'en_US' => 'English US',
		'fr' => 'French',
		'es' => 'Spanish' );
// adds a message to the log div @ tail
function rez_append_log($message = "no message specified?") {
	global $global;
	$global['xajax_res']->addAppend('rezLog','innerHTML',$message);
	//---- scroll the log to the bottom
	$global['xajax_res']->addScript("setTimeout('e = document.getElementById(\'rezLog\'); e.scrollTop = e.scrollHeight+1000;', 500);");
	return $global['xajax_res']->getXML();
}

function rez_get_translation($value)
{
	global $global;
	return $global['xajax_res']->getXML();
}

//Pulls up the value from selected template/page suugestion into edit fields.
function rez_render_fields($id,$type,$actual){

	global $global;
	list($title,$content)=render_fields($actual,$type,$id);
	$global['xajax_res']->addAssign('pageContent','innerHTML',$content);
	$global['xajax_res']->addScript('window.rezPageSaved = false;nicEditors.findEditor(\'pageContent\').setContent('.$content.');');
	$global['xajax_res']->addScript("document.getElementById('menuTitle').value='".$title."';");
	return $global['xajax_res']->getXML();
}

// adds a message to the log div @ head
function rez_prepend_log($message = "no message specified?") {
	global $global;
	$global['xajax_res']->addPrepend('rezLog','innerHTML',$message);
	return $global['xajax_res']->getXML();
}


// shows a status message before another ajax function is executed
function rez_show_message($message = "no message specified?") {
	global $global;
	$global['xajax_res']->addAssign('rezMain','innerHTML','<div class="loadingMessage"><center><blink>'.$message.'</blink></center></div>');
	return $global['xajax_res']->getXML();
}


// reloads the mainArea div with information of what pages we have
function rez_show_pages($internal = FALSE) {
	global $conf;
	global $global;
	$htmlLog = "";
	$htmlMain = "";
	$relevant_pages=array();
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
	$event_id=$event_row['incident_id'];


	// NORMAL PAGES
	$htmlMain .= "<table class=\"mainTable\"><tr>"
		."<td style=\"padding-left: 8px;\" ><b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"._t("ResourcePages-Table-Header|Menu/Page Title")."</b></td>"
		."<td style=\"text-align: center;\" ><b>"._t("ResourcePages-Table-Header|Visibility")."</b></td>"
		."<td style=\"text-align: center;\" ><b>"._t("ResourcePages-Table-Header|Locale")."</b></td>"
		."<td style=\"text-align: center;\" ><b>"._t("ResourcePages-Table-Header|Edit")."</b></td>"
		."<td style=\"text-align: center;\" ><b>"._t("ResourcePages-Table-Header|Delete")."</b></td>"
		."<td style=\"text-align: center;\" ><b>"._t("ResourcePages-Table-Header|Translate")."</b></td>"
		."<td style=\"text-align: center;\"><b>"._t("ResourcePages-Table-Header|Move")."</b></td></tr>";

	$currentlocale = shn_rez_getlocale();
	$q = "
		select rez_page_id, rez_locale, rez_menu_title, rez_menu_order, rez_visibility, rez_timestamp
		from rez_pages
		where rez_page_id > -1 and rez_menu_title != 'new page'
		order by rez_menu_order asc;
	";
	$result = $global['db']->Execute($q);
	if($result === false) {
		daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez show pages");
	}

	$pages = array();
	$count = 0;

	// find out how many pages we have so we can later build our pages table
	while($row = $result->FetchRow() ){
		$pages[$count] = $row;
		$count++;
	}
	$rez_locales = $global['rez_locales'];
	// build pages Table
	$i = 1;
	$eo = 0;
	while($i <= $count) {
		$row = $pages[$i-1];
		$page_id    = $row['rez_page_id'];
		$page_locale= $row['rez_locale'];
		$menu_title = $row['rez_menu_title'];
		$menu_order = $row['rez_menu_order'];
		$visibility = $row['rez_visibility'];
		$timestamp  = $row['rez_timestamp'];
		if ( $eo == 0 ) {
			$evenOddClass = "mainRowEven";
		} else {
			$evenOddClass = "mainRowOdd";
		}
		$int = ($visibility == 'INTERNAL') ? "<b>INTERNAL</b> " : "";
		$htmlMain .= "<tr>"
			."<td style=\"padding-left: 8px;\" class=\"".$evenOddClass."\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"#\" onclick=\"rez_perform_edit('resource',".$page_id.",'".$page_locale."');\">".$int.$menu_title."</a></td>"
			."<td style=\"text-align: center;\" class=\"".$evenOddClass."\"><b><select name=\"visibility\" onchange=\"javascript: rez_perform_change_visibility('page',".$page_id.", this.value);\"><option value=\"Public\" ".(($visibility == 'Public')?'SELECTED':'').">"._t("ResourcePages-Visibility-Select|Public")."<option value=\"Hidden\" ".(($visibility == 'Hidden')?'SELECTED':'').">"._t("ResourcePages-Visibility-Select|Hidden")."<option value=\"INTERNAL\" ".(($visibility == 'INTERNAL')?'SELECTED':'').">"._t("ResourcePages-Visibility-Select|INTERNAL")."</select></b></td>";

		$htmlMain .= "<td style=\"padding-left: 8px; text-align: center;\" class=\"".$evenOddClass."\">".$rez_locales[$page_locale]."</td>";
		$htmlMain .= "<td style=\"text-align: center;\" class=\"".$evenOddClass."\"><input class=\"styleTehButton\" type=\"button\" onclick=\"rez_perform_edit('resource',".$page_id.",'".$page_locale."');\" value=\""._t("ResourcePages-Button|Edit")."\"></td>";

		$htmlMain .= "<td style=\"text-align: center;\" class=\"".$evenOddClass."\"><input class=\"styleTehButton\" type=\"button\" onclick=\"rez_append_log('Deleting Page ".$page_id." ...<br>'); setTimeout('rez_perform_delete(\'page\',".$page_id.",\'".$page_locale."\',confirm(\'"._t("ResourcePages-Delete-Confirm|Are you sure you want to delete this page?")."\'));', 100);\" value=\""._t("ResourcePages-Button|Delete")."\"></td>";
		$htmlMain .= "<td style=\"text-align: center;\" class=\"".$evenOddClass."\"><input class=\"styleTehButton\" type=\"button\" onclick=\"rez_perform_translate('page',".$page_id.",'".$page_locale."');\" value=\""._t("ResourcePages-Button|Translate")."\"></td>";

		// if its the first and only item, no move up/down controls
		if (($i == 1) && ($count == 1)) {
			$htmlMain .= "<td style=\"text-align: center;\" class=\"".$evenOddClass."\">-</td>";

			// if its the first item and there are more, show down control
		} elseif (($i == 1) && ($count > 1)) {
			$htmlMain .= "<td style=\"text-align: center;\" class=\"".$evenOddClass."\"><input class=\"styleTehButton\" type=\"button\" onclick=\"rez_perform_move_down(".$page_id.");\" value=\"⇣\"></td>";

			// if its the last item, only show up control
		} elseif ($i == $count) {
			$htmlMain .= "<td style=\"text-align: center;\" class=\"".$evenOddClass."\"><input class=\"styleTehButton\" type=\"button\" onclick=\"rez_perform_move_up(".$page_id.");\" value=\"⇡\"></td>";

			// its a normal item, show up and down controls
		} else {
			$htmlMain .= "<td style=\"text-align: center;\" class=\"".$evenOddClass."\"><input class=\"styleTehButton\" type=\"button\" onclick=\"rez_perform_move_up(".$page_id.");\" value=\"⇡\"> <input class=\"styleTehButton\" type=\"button\" onclick=\"rez_perform_move_down(".$page_id.");\" value=\"⇣\"></td>";
		}
		$htmlMain .= "</tr>";
		$i++;
		$eo++;
		$eo %= 2;
	}
	if ($count == 0) {
		$htmlMain .= "<tr><td colspan=7 class=\"mainRowEven\" style=\"text-align: center;\">"._t("ResourcePages-Message|No Pages have been created.")."</td></tr>";
	}
	$htmlMain .= "</table>";
	// END NORMAL PAGES





	// SYSTEM PAGES
	$htmlMain .= "<br><h2>"._t("ResourcePages-Heading|System Pages")."</h2>"
		."<table class=\"mainTable\"><tr><td style=\"padding-left: 8px; width: 50px; text-align: center;\" ><b>"._t("ResourcePages-Table-Header|Page #")."</b></td>"
		."<td style=\"padding-left: 8px;\" ><b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"._t("ResourcePages-Table-Header|Menu/Page Title")."</b></td>"
		."<td style=\"text-align: center;\" ><b>"._t("ResourcePages-Table-Header|Edit")."</b></td></tr>";

	$q = "
		select rez_page_id, rez_menu_title, rez_menu_order, rez_visibility, rez_timestamp
		from rez_pages
		where rez_page_id < 0
		order by rez_page_id desc;
	";
	$result = $global['db']->Execute($q);
	if($result === false) {
		daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez system pages");
	}

	$pages = array();
	$count = 0;

	// find out how many pages we have so we can later build our pages table
	while($row = $result->FetchRow() ){
		$pages[$count] = $row;
		$count++;
	}

	// build system pages Table
	$burl = "<img src=\"theme/".$conf['theme']."/img/"; // base URL for images, (it chooses the /www/theme/THEME_NAME/img/ folder)
	$i = 1;
	$eo = 0;
	while($i <= $count) {
		$row = $pages[$i-1];
		$page_id    = $row['rez_page_id'];
		$menu_title = $row['rez_menu_title'];
		$menu_order = $row['rez_menu_order'];
		$visibility = $row['rez_visibility'];
		$timestamp  = $row['rez_timestamp'];
		if ( $eo == 0 ) {
			$evenOddClass = "mainRowEven";
		} else {
			$evenOddClass = "mainRowOdd";
		}
		$htmlMain .= "<tr><td style=\"padding-left: 8px; text-align: center;\" class=\"".$evenOddClass."\">".abs($page_id)."</td><td style=\"padding-left: 8px;\" class=\"".$evenOddClass."\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"#\" onclick=\"rez_perform_edit('system',".$page_id.",'');\">".$menu_title."</a></td>";

		$htmlMain .= "<td style=\"text-align: center;\" class=\"".$evenOddClass."\"><input class=\"styleTehButton\" type=\"button\" onclick=\"rez_perform_edit('system',".$page_id.",'');\" value=\""._t("ResourcePages-Button|Edit")."\"></td>";

		$i++;
		$eo++;
		$eo %= 2;
	}
	if ($count == 0) {
		$htmlMain .= "<tr><td colspan=7 class=\"mainRowEven\" style=\"text-align: center;\">"._t("ResourcePages-Message|No Pages have been created.")."</td></tr>";
	}
	$htmlMain .= "</table>";
	// END SYSTEM PAGES





	$pageControlHtml = '<input class="styleTehButton" type="button" onclick="javascript: rez_append_log(\''._t("ResourcePages-Message|Creating a new page...").'<br>\'); rez_perform_edit(\'newresource\',\'\',\'\');" value="'._t("ResourcePages-Button|Create new Resource Page").'">&nbsp;&nbsp;&nbsp;&nbsp;<input class="styleTehButton" type="button" onclick="javascript: rez_append_log(\''._t("ResourcePages-Message|Creating a new System page...").'<br>\'); rez_perform_edit(\'newsystem\',\'\',\'\');" value="'._t("ResourcePages-Button|Create new System Page").'">&nbsp;&nbsp;&nbsp;&nbsp;<input class="styleTehButton" type="button" onclick="javascript: rez_append_log(\''._t("ResourcePages-Message|Showing Templates...").'<br>\'); rez_show_templates();" value="'._t("ResourcePages-Button|Manage Templates").'">&nbsp;&nbsp;&nbsp;&nbsp;<input class="styleTehButton" type="button" onclick="javascript: rez_append_log(\'Showing Help...<br>\'); rez_show_help();" value="Help">';

	//---- internally, no ajax, just pass back data
	if ( $internal ) {
		return $htmlMain;
	} else {
		$global['xajax_res']->addAssign('rezControl','style.opacity','1.0');
		$global['xajax_res']->addAssign('rezControl','innerHTML',$pageControlHtml);

		$global['xajax_res']->addAssign('rezMain','style.opacity','1.0');
		$global['xajax_res']->addAssign('rezMain','innerHTML',$htmlMain);
		$count-=1;
		$htmlLog .= "Found ".$count ." Resource page(s).<br>";
		$global['xajax_res']->addAppend('rezLog','innerHTML',$htmlLog);
		$global['xajax_res']->addScript("setTimeout('e = document.getElementById(\'rezLog\'); e.scrollTop = e.scrollHeight+1000;', 500);");
		return $global['xajax_res']->getXML();
	}
}



// save a change in visibility for a certain page
function rez_perform_change_visibility($type,$page_id, $value) {
	global $global;
	// update page's visibility
	if ($type !== 'template'){
		$q = "
			UPDATE rez_pages
			set rez_visibility = '".$value."'
			where `rez_page_id` = ".$page_id." ;
		";}
	else{
		$q = "
			UPDATE rez_page_template
			set rez_template_visibility = '".$value."'
			where `rez_template_id` = ".$page_id." ;
		";

	}
	$result = $global['db']->Execute($q);
	if($result === false) {
		daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez update visibility");
	}

	$global['xajax_res']->addAppend('rezLog','innerHTML',_t("ResourcePages-Changed-Message|Changed visibility of Page/Template").' <b>#'.$page_id.'</b> '._t('ResourcePages-Changed-Message|to').' <b>'.$value.'</b>.<br>');
	$global['xajax_res']->addScript("setTimeout('e = document.getElementById(\'rezLog\'); e.scrollTop = e.scrollHeight+1000;', 500);");
	return $global['xajax_res']->getXML();
}

function rez_show_templates() {
	global $conf;
	global $global;
	$htmlLog = "";
	$htmlMain = "";

	$htmlMain .= "<table class=\"mainTable\"><tr><td style=\"padding-left: 8px; width: 50px; text-align: center;\" ><b>"._t("ResourcePages-Table-Header|Template #")."</b></td>"
		."<td style=\"padding-left: 8px;\" ><b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"._t("ResourcePages-Table-Header|Template Title")."</b></td>"
		."<td style=\"text-align: center;\" ><b>"._t("ResourcePages-Table-Header|Visibility")."</b></td>"
		."<td style=\"text-align: center;\" ><b>"._t("ResourcePages-Table-Header|Disaster Category")."</b></td>"
		."<td style=\"text-align: center;\"><b>"._t("ResourcePages-Table-Header|Locale")."</b></td>"
		."<td style=\"text-align: center;\" ><b>"._t("ResourcePages-Table-Header|Edit")."</b></td>"
		."<td style=\"text-align: center;\" ><b>"._t("ResourcePages-Table-Header|Delete")."</b></td>"
		."<td style=\"text-align: center;\" ><b>"._t("ResourcePages-Table-Header|Translate")."</b></td></tr>";

	$currentlocale = shn_rez_getlocale();
	$q="
		SELECT *
		FROM disaster_category
		ORDER BY name;
	";
	$result = $global['db']->Execute($q);
	if($result === false) {
		daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "getting disaster categories");
	}
	$categories =array(); //contains mapping between disaster_category_id v/s name
	$ordered =array();  //contains disaster_category_id in sorted order according to names.
	$rez_disaster_categories=array(); //contains mapping between templates and category. 
	while($row = $result->FetchRow() ){
		$categories[$row['category_id']] = $row['name'];
		$ordered[]=$row['category_id'];
	}
	$templates = array(); // Holds the template details
	foreach ($ordered as $value){
		$q="
			SELECT *
			FROM rez_template_scenario
			WHERE disaster_type =".$value."
			ORDER BY template_id;
		";
		$result = $global['db']->Execute($q);
		if($result === false) {
			daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "getting disaster categories");
		}
		while($row = $result->FetchRow()){
			$q2 = "
				select *
				from rez_page_template
				WHERE rez_template_id = ".$row['template_id']." and rez_menu_title != 'New Template'; 
			";
			$result2 = $global['db']->Execute($q2);
			if($result2 === false) {
				daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "getting disaster categories");
			}
			$row2 =$result2->FetchRow();
			$templates[$row2['rez_template_id']]=$row2;
			if ($rez_disaster_categories[$row2['rez_template_id']] !== NULL){
				$rez_disaster_categories[$row2['rez_template_id']].=','.$categories[$value];
			}
			else{
				$rez_disaster_categories[$row2['rez_template_id']]=$categories[$value];
			}

		}

	}
	$rez_locales = $global['rez_locales'];
	// build templates Table
	$i = 1;
	$eo = 0;
	$count= sizeof($templates);
	foreach($templates as $row) {
		$template_id    = $row['rez_template_id'];
		$category=$rez_disaster_categories[$row['rez_template_id']];
		$template_locale= $row['rez_template_locale'];
		$menu_title = $row['rez_menu_title'];
		$visibility = $row['rez_template_visibility'];
		$timestamp  = $row['rez_template_timestamp'];
		if ( $eo == 0 ) {
			$evenOddClass = "mainRowEven";
		} else {
			$evenOddClass = "mainRowOdd";
		}
		$int = ($visibility == 'INTERNAL') ? "<b>INTERNAL</b> " : "";
		$htmlMain .= "<tr><td style=\"padding-left: 8px; text-align: center;\" class=\"".$evenOddClass."\">".$i."</td>"
			."<td style=\"padding-left: 8px;\" class=\"".$evenOddClass."\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"#\" onclick=\"rez_perform_edit('template',".$template_id.",'".$template_locale."');\">".$int.$menu_title."</a></td>"
			."<td style=\"text-align: center;\" class=\"".$evenOddClass."\"><b><select name=\"visibility\" onchange=\"javascript: rez_perform_change_visibility('template',".$template_id.", this.value);\"><option value=\"Public\" ".(($visibility == 'Public')?'SELECTED':'').">"._t("ResourcePages-Visibility-Select|Public")."<option value=\"Hidden\" ".(($visibility == 'Hidden')?'SELECTED':'').">"._t("ResourcePages-Visibility-Select|Hidden")."<option value=\"INTERNAL\" ".(($visibility == 'INTERNAL')?'SELECTED':'').">"._t("ResourcePages-Visibility-Select|INTERNAL")."</select></b></td>";

		$htmlMain .= "<td style=\"padding-left: 8px; text-align: center;\" class=\"".$evenOddClass."\">".$category."</td>";
		$htmlMain .= "<td style=\"padding-left: 8px; text-align: center;\" class=\"".$evenOddClass."\">".$rez_locales[$template_locale]."</td>";
		$htmlMain .= "<td style=\"text-align: center;\" class=\"".$evenOddClass."\"><input class=\"styleTehButton\" type=\"button\" onclick=\"rez_perform_edit('template',".$template_id.",'".$template_locale."');\" value=\""._t("ResourcePages-Button|Edit")."\"></td>";

		$htmlMain .= "<td style=\"text-align: center;\" class=\"".$evenOddClass."\"><input class=\"styleTehButton\" type=\"button\" onclick=\"rez_append_log('Deleting Template ".$template_id." ...<br>'); setTimeout('rez_perform_delete(\'template\',".$template_id.",\'".$template_locale."\',confirm(\'"._t("ResourcePages-Delete-Confirm|Are you sure you want to delete this template?")."\'));', 100);\" value=\""._t("ResourcePages-Button|Delete")."\"></td>";
		$htmlMain .= "<td style=\"text-align: center;\" class=\"".$evenOddClass."\"><input class=\"styleTehButton\" type=\"button\" onclick=\"rez_perform_translate('template',".$template_id.",'".$template_locale."');\" value=\""._t("ResourcePages-Button|Translate")."\"></td>";

		$htmlMain .= "</tr>";
		$i++;
		$eo++;
		$eo %= 2;
	}
	if ($count == 0) {
		$htmlMain .= "<tr><td colspan=7 class=\"mainRowEven\" style=\"text-align: center;\">"._t("ResourcePages-Message|No Templates  have been created.")."</td></tr>";
	}
	$htmlMain .= "</table>";
	//
	$pageControlHtml = '<input class="styleTehButton" type="button" onclick="javascript: rez_append_log(\''._t("ResourcePages-Message|Creating a new Template...").'<br>\'); rez_perform_edit(\'newtemplate\',\'\',\'\');" value="'._t("ResourcePages-Button|Create new Template").'">&nbsp;&nbsp;&nbsp;&nbsp;<input class="styleTehButton" type="button" onclick="javascript: rez_append_log(\'Showing Help...<br>\'); rez_show_help();" value="Help">';

	//---- internally, no ajax, just pass back data
	if ( $internal ) {
		return $htmlMain;
	} else {
		$global['xajax_res']->addAssign('rezControl','style.opacity','1.0');
		$global['xajax_res']->addAssign('rezControl','innerHTML',$pageControlHtml);

		$global['xajax_res']->addAssign('rezMain','style.opacity','1.0');
		$global['xajax_res']->addAssign('rezMain','innerHTML',$htmlMain);

		$htmlLog .= "Found ".$count." Template(s).<br>";
		$global['xajax_res']->addAppend('rezLog','innerHTML',$htmlLog);
		$global['xajax_res']->addScript("setTimeout('e = document.getElementById(\'rezLog\'); e.scrollTop = e.scrollHeight+1000;', 500);");
		return $global['xajax_res']->getXML();
	}
}


// move page_id down on page in the menu order
function rez_perform_move_down($page_id) {
	global $global;

	// find the menu_order for $page_id
	$q = "
		SELECT rez_menu_order
		from rez_pages
		where rez_page_id = '".$page_id."';
	";
	$result = $global['db']->Execute($q);
	if($result === false) {
		daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez move down 1");
	}
	$row = $result->FetchRow();
	$pageMenuOrder = $row['rez_menu_order'];

	// find the menu_order for the page below $page_id we are going to swap it with
	$q = "
		SELECT *
		from rez_pages
		where rez_menu_order > '".$pageMenuOrder."'
		order by rez_menu_order asc
		limit 1;
	";
	$result = $global['db']->Execute($q);
	if($result === false) {
		daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez move down 2");
	}
	$row2 = $result->FetchRow();
	$nextMenuOrder = $row2['rez_menu_order'];
	$nextPageID = $row2['rez_page_id'];

	// update page_id with new menu order
	$q = "
		UPDATE rez_pages
		set rez_menu_order = '".$nextMenuOrder."'
		where `rez_page_id` = ".$page_id." ;
	";
	$result = $global['db']->Execute($q);
	if($result === false) {
		daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez move down 3");
	}

	// update next page with page_id's menu order
	$q = "
		UPDATE rez_pages
		set rez_menu_order = '".$pageMenuOrder."'
		where `rez_page_id` = ".$nextPageID." ;
	";
	$result = $global['db']->Execute($q);
	if($result === false) {
		daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez move down 4");
	}

	$global['xajax_res']->addAppend('rezLog','innerHTML',_t("ResourcePages-Moved-Message|Moved page #").$page_id._t("ResourcePages-Moved-Message|down.").'<br>');
	$global['xajax_res']->addScript("setTimeout('e = document.getElementById(\'rezLog\'); e.scrollTop = e.scrollHeight+1000;', 500);");
	$global['xajax_res']->addAssign('rezMain','innerHTML',rez_show_pages(TRUE));
	return $global['xajax_res']->getXML();
}



// move page_id up on page in the menu order
function rez_perform_move_up($page_id) {
	global $global;

	// find the menu_order for $page_id
	$q = "
		SELECT rez_menu_order
		from rez_pages
		where rez_page_id = '".$page_id."';
	";
	$result = $global['db']->Execute($q);
	if($result === false) {
		daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez move up 1");
	}
	$row = $result->FetchRow();
	$pageMenuOrder = $row['rez_menu_order'];

	// find the menu_order for the page above $page_id we are going to swap it with
	$q = "
		SELECT *
		from rez_pages
		where rez_menu_order < '".$pageMenuOrder."'
		order by rez_menu_order desc
		limit 1;
	";
	$result = $global['db']->Execute($q);
	if($result === false) {
		daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez move up 2");
	}
	$row2 = $result->FetchRow();
	$previousMenuOrder = $row2['rez_menu_order'];
	$previousPageID = $row2['rez_page_id'];

	// update page_id with new menu order
	$q = "
		UPDATE rez_pages
		set rez_menu_order = '".$previousMenuOrder."'
		where `rez_page_id` = ".$page_id." ;
	";
	$result = $global['db']->Execute($q);
	if($result === false) {
		daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez move up 3");
	}

	// update previous page with page_id's menu order
	$q = "
		UPDATE rez_pages
		set rez_menu_order = '".$pageMenuOrder."'
		where `rez_page_id` = ".$previousPageID." ;
	";
	$res = $global['db']->Execute($q);
	if($result === false) {
		daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez move up 4");
	}

	$global['xajax_res']->addAppend('rezLog','innerHTML',_t("ResourcePages-Moved-Message|Moved page #").$page_id._t("ResourcePages-Moved-Message|up.").'<br>');
	$global['xajax_res']->addScript("setTimeout('e = document.getElementById(\'rezLog\'); e.scrollTop = e.scrollHeight+1000;', 500);");
	$global['xajax_res']->addAssign('rezMain','innerHTML',rez_show_pages(TRUE));
	return $global['xajax_res']->getXML();
}



// create a new page/template with default content and return its new id
function rez_perform_new($type) {
	global $global;
	if ($type =='newsystem')
	{
		$q = "
			SELECT *
			FROM rez_pages
			WHERE rez_page_id < 0
			ORDER BY rez_page_id ASC;
		";
		$result = $global['db']->Execute($q);
		if($result === false) {
			daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez new 1");
		}
		$row = $result->FetchRow();
		$mID = $row['rez_page_id'] -1;

		$q2 = "
			SELECT *
			FROM rez_pages
			WHERE rez_page_id =".$mID."; 
		";
		$q = "
			INSERT INTO rez_pages (rez_page_id,rez_locale,rez_menu_title,rez_content, rez_timestamp, rez_visibility)
			VALUES (".$mID.",'en_US','new system page','new system page content','".date('Y-m-d H:i:s')."', 'Hidden');
		";
		$result = $global['db']->Execute($q);
		if($result === false) {
			daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez new 2");
		}
		return array($mID,'system');

	}
	else if ($type =='newtemplate')
	{
		$q = "
			SELECT *
			FROM rez_page_template
			ORDER BY rez_template_id DESC;
		";
		$result = $global['db']->Execute($q);
		if($result === false) {
			daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez new 2");
		}
		$row = $result->FetchRow();
		$mID = $row['rez_template_id']+1;

		$q = "
			INSERT INTO rez_page_template (rez_template_id,rez_template_locale,rez_menu_title,rez_template_content, rez_template_timestamp, rez_template_visibility,created_by)
			VALUES (".$mID.",'en_US','New Template','New Template content','".date('Y-m-d H:i:s')."', 'Hidden',1);
		";
		$result = $global['db']->Execute($q);
		if($result === false) {
			daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez new 2");
		}

		return array($mID,'template');

	}
	else
	{
		$q = "
			SELECT *
			FROM rez_pages
			WHERE rez_page_id > 0
			ORDER BY rez_menu_order DESC
			LIMIT 1;
		";
		$result = $global['db']->Execute($q);
		if($result === false) {
			daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez new 3");
		}
		$row = $result->FetchRow();
		$mID = isset($row['rez_menu_order']) ? $row['rez_menu_order'] : 0;
		$mID++;
		$q = "
			SELECT *
			FROM rez_pages
			ORDER BY rez_page_id DESC;
		";
		$result = $global['db']->Execute($q);
		if($result === false) {
			daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez new 3");
		}
		$row = $result->FetchRow();
		$page_id =$row['rez_page_id'] + 1;
		$q = "
			INSERT INTO rez_pages (rez_page_id,rez_locale,rez_menu_title, rez_menu_order, rez_content, rez_timestamp, rez_visibility)
			VALUES ('".$page_id."','en_US','new page', '".$mID."','new page content', '".date('Y-m-d H:i:s')."', 'Hidden');
		";
		$result = $global['db']->Execute($q);
		if($result === false) {
			daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez new 3");
		}


		return array($page_id,'resource');

	}

}



// saves the edit's page content/attributes to the database
function rez_perform_save($old_locale,$page_id, $r) {
	global $global;
	//if locale is not set .. switch to default value - en_US
	if (!$r['locale']) 
	{
		$r['locale'] = 'en_US';
	}
	if ($old_locale !== 'translated'){
		$q = "select rez_page_id from rez_pages
			where rez_page_id='{$page_id}'
			and rez_locale='{$old_locale}'";
		$result = $global['db']->Execute($q);
		if($result === false) {
			daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez save");
		}
		$row = $result->FetchRow();
		if ( isset($row['rez_page_id']) ) {
			if (isset($r['Templates'])){
				// update table data
				$q = "
					update rez_pages
					set rez_menu_title = '".mysql_escape_string($r['menuTitle'])."', rez_content = '".mysql_escape_string($r['pageContent'])."', rez_timestamp = '".date('Y-m-d H:i:s')."',rez_locale='".$r['locale']."',parent_template_id='".$r['Templates']."' where `rez_page_id` = ".$page_id." and `rez_locale` = '".$old_locale."' ;
					";
			}
			elseif( isset($r['Pages'])){
				$q = "
					update rez_pages
					set rez_menu_title = '".mysql_escape_string($r['menuTitle'])."', rez_content = '".mysql_escape_string($r['pageContent'])."', rez_timestamp = '".date('Y-m-d H:i:s')."',rez_locale='".$r['locale']."' , parent_page_id='".$r['Pages']."' where `rez_page_id` = ".$page_id." and `rez_locale` = '".$old_locale."' ;
					";
			}
			else{
				$q = "
					update rez_pages
					set rez_menu_title = '".mysql_escape_string($r['menuTitle'])."', rez_content = '".mysql_escape_string($r['pageContent'])."', rez_timestamp = '".date('Y-m-d H:i:s')."',rez_locale='".$r['locale']."' where `rez_page_id` = ".$page_id." and `rez_locale` = '".$old_locale."' ;
					";
			}
			$result = $global['db']->Execute($q);
				if($result === false) {
					daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez save");
				}
			if ($page_id >0){
				$q= "
					delete FROM
					rez_incident
					WHERE `page_id` ='".$page_id."';
				";
				$result = $global['db']->Execute($q);
				if($result === false) {
					daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez page:_disaster save");
				}
				$tempvar=$r['event_select'];
				$temp=explode(",",$tempvar);
				foreach ($temp as $value){
					$q="
						INSERT INTO rez_incident (page_id,incident_id,timestamp)
						VALUES ('".$page_id."','".mysql_escape_string($value)."','".date('Y-m-d H:i:s')."');
					";
					$result = $global['db']->Execute($q);
						if($result === false) {
							daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez page_incident save");
						}
				}

			}
		}
	}
	else {
		//Translation creates new page
		$q = "
			SELECT *
			FROM rez_pages
			ORDER BY rez_menu_order DESC
			LIMIT 1;
			";
			$result = $global['db']->Execute($q);
			if($result === false) {
				daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez new 1");
			}
		$row = $result->FetchRow();
			$mID = isset($row['rez_menu_order']) ? $row['rez_menu_order'] : 0;
			$mID++;
		$q = "
			INSERT INTO rez_pages (rez_page_id,rez_locale,rez_menu_title, rez_menu_order, rez_content, rez_timestamp, rez_visibility)
			VALUES ('{$page_id}','{$r['locale']}','{$r['menuTitle']}', '{$mID}','{$r['pageContent']}', '".date('Y-m-d H:i:s')."', 'Public');
		";
		$result = $global['db']->Execute($q);
			if($result === false) {
				daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez new 2");
			}
		$global['xajax_res']->addAppend('rezLog','innerHTML',_t("ResourcePages-Saved-Message|Page #").$page_id.$r['menuTitle']." <span style=\"color: green;\">"._t("ResourcePages-Saved-Message|Saved")."</span>.<b><br>");
	}


	// update log to reflect the save
	$global['xajax_res']->addAppend('rezLog','innerHTML',_t("ResourcePages-Saved-Message|Page #").$page_id." <span style=\"color: green;\">"._t("ResourcePages-Saved-Message|Saved")."</span>.<b><br>");
	$global['xajax_res']->addScript("setTimeout('e = document.getElementById(\'rezLog\'); e.scrollTop = e.scrollHeight+1000;', 500);");
	return $global['xajax_res']->getXML();
}
function rez_template_save($old_locale,$template_id, $r) {
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
	//if locale is not set .. switch to default value - en_US
	if (!$r['locale']) 
	{
		$r['locale'] = 'en_US';
	}
	if ($old_locale !== 'translated'){
		$q = "select rez_template_id from rez_page_template
			where rez_template_id='{$template_id}'
			and rez_template_locale='{$old_locale}'";
		$result = $global['db']->Execute($q);
		if($result === false) {
			daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez template save");
		}
		$row = $result->FetchRow();
		if ( isset($row['rez_template_id']) ) {
			// update table data
			$q = "
				update rez_page_template
				set rez_menu_title = '".mysql_escape_string($r['menuTitle'])."', rez_template_content = '".mysql_escape_string($r['pageContent'])."', rez_template_timestamp = '".date('Y-m-d H:i:s')."',rez_template_locale='".$r['locale']."' where `rez_template_id` = ".$template_id." and `rez_template_locale` = '".$old_locale."' ;
				";
				$result = $global['db']->Execute($q);
				if($result === false) {
					daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez template save");
				}
			$q= "
				delete FROM
				rez_template_scenario
				WHERE `template_id` ='".$template_id."';
			";
			$result = $global['db']->Execute($q);
			if($result === false) {
				daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez template_disaster save");
			}
			$tempvar=$r['disaster_select'];
			$temp=explode(",",$tempvar);
			foreach ($temp as $value){
				$q="
					INSERT INTO rez_template_scenario (template_id,disaster_type)
					VALUES ('".$template_id."','".mysql_escape_string($value)."');
				";
				$result = $global['db']->Execute($q);
				if($result === false) {
					daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez template_disaster save");
				}
			}

			if (isset($r['page_change_select'])){
				$title = $event_name . ' : ' . mysql_escape_string($r['menuTitle']);
				$q="
					update rez_pages
					set rez_menu_title = '".$title."', rez_content = '".mysql_escape_string($r['pageContent'])."', rez_timestamp = '".date('Y-m-d H:i:s')."',rez_locale='".$r['locale']."' where `rez_page_id` = ".$r['page_change_select']."";
				$result = $global['db']->Execute($q);
				if($result === false) {
					daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez template_disaster save 2");
				}

			}
		}
	}
	else {

		//Translation creates new page
		$q = "
			INSERT INTO rez_page_template (rez_template_id,rez_template_locale,rez_menu_title,rez_template_content, rez_template_timestamp, rez_template_visibility,created_by)
			VALUES (".$template_id.",'".$r['locale']."','".$r['menuTitle']."','".$r['pageContent']."','".date('Y-m-d H:i:s')."', 'Hidden',1);
		";

		$result = $global['db']->Execute($q);
		if($result === false) {
			daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez new 2");
		}
	}


	// update log to reflect the save
	$global['xajax_res']->addAppend('rezLog','innerHTML',_t("ResourcePages-Saved-Message|Template #").$template_id." <span style=\"color: green;\">"._t("ResourcePages-Saved-Message|Saved")."</span>.<b><br>");
	$global['xajax_res']->addScript("setTimeout('e = document.getElementById(\'rezLog\'); e.scrollTop = e.scrollHeight+1000;', 500);");
	return $global['xajax_res']->getXML();
}



// presents a gui to edit a page and its corresponding fields
function rez_perform_edit($rez_type,$page_id, $locale) {
	global $global;
	$type ='resource';
	// if locale is not set .. Switch to default value - en_Us
	if (!$locale){
		$locale='en_US';
	}
	//This implies that the current page is a System Page
	if($page_id < 0 or $rez_type == 'system'){
		$type='system';
	}
	if ($rez_type == "template"){
		$type = 'template';
	}
	if (strpos($rez_type,'new') !== false){
		list($page_id,$type) = rez_perform_new($rez_type);
	}
	$old_locale=$locale;
	if ($type !=='template'){
		// get page info
		$q = "
			select *
			from rez_pages
			where rez_page_id = '".$page_id."';
		";
		$result = $global['db']->Execute($q);
		if($result === false) {
			daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez edit 1");
		}
		$row = $result->FetchRow();
		$content = $row['rez_content'];
		$title= $row['rez_menu_title'];
		$locale=$row['rez_locale'];
		$timestamp=$row['rez_timestamp'];

		if((int)$row['rez_page_id'] < 0) {
			$sistim = "SYSTEM";
		} else {
			$sistim = "";
		}
		$pageIdText = $sistim." ". abs($row['rez_page_id']);
	}
	else{

		$q = "
			select *
			from rez_page_template
			where rez_template_id = '".$page_id."'
			and rez_template_locale = '".$locale."';
		";
		$result = $global['db']->Execute($q);
		if($result === false) {
			daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez edit 1");
		}
		$row = $result->FetchRow();
		$pageIdText = $row['rez_template_id'];
		$content = $row['rez_template_content'];
		$title= $row['rez_menu_title'];
		$locale=$row['rez_template_locale'];
		$timestamp=$row['rez_template_timestamp'];
		$names=array();
		$q= " 
			SELECT *
			FROM rez_template_scenario 
			WHERE `template_id` = ".$row['rez_template_id'].";
		";
		$categories = $global['db']->Execute($q);
		if($categories === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "generate categry 1"); }
		{
			while($type_id = $categories->FetchRow()){
				//get the name of the disaster category from ids
				$q="
					SELECT *
					FROM disaster_category
					WHERE `category_id` =".$type_id['disaster_type'].";
				";
				$type_name = $global['db']->Execute($q);
				if($type_name === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "generate category 2"); }
				$name=$type_name->FetchRow();
				$names[]=$name['name'];
			}

		}
		$q="
			SELECT *
			FROM disaster_category
			ORDER BY name;
		";
		$categories=$global['db']->Execute($q);
		$cats='';
		$cats.="<select data-placeholder=\"Select from Existing Categories\" class=\"chzn-select-no-results\" name=\"disaster_select\" multiple  id= \"chzn-select-no-results\" style=\"width: 800px\";";
		if($categories === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "generate categry 3"); }
		$cats.='<option value=""></option>'; // for showing placeholder we need to have a blank option.
		while($name=$categories->FetchRow())
		{
			if ($name['name'] == "Default")
			{
				continue;
			}
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

	}
	if ($type === 'resource'){
		//getting incident_id
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
		//creating template_select
		//getting incident->disaster mapping
		$template_select='<select name="Templates" id="suggest_template" style="display:none;" onclick="rez_render_fields(document.getElementById(\'suggest_template\').value,\'suggest_template\',2)">';
		$q="
			select *
			from scenario
			where incident_id ='".$event_row['incident_id']."';
		";
		$result = $global['db']->Execute($q);
		if($result === false) {
			daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez edit get_disaster1");
		}
		$categories=array();	
		while($category = $result->FetchRow()){
			//getting disaster name
			$q="
				select *
				from disaster_category
				where category_id ='".$category['disaster_type']."';
			";
			$result2 = $global['db']->Execute($q);
			if($result2 === false) {
				daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez edit get_disaster2");
			}
			$cat_row = $result2->FetchRow();
			$categories[$category['disaster_type']] = $cat_row['name'];
			$categories[0]='Default';
		}
		foreach ($categories as $key => $val){
			//getting templates associated with disaster types
			$q="
				select *
				from rez_template_scenario
				where disaster_type ='".$key."';
			";
			$result2 = $global['db']->Execute($q);
			if($result2 === false) {
				daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez edit get_disaster3");
			}
			$template_select.='<optgroup label ="'.$val.'">';
			while($templates = $result2->FetchRow())
			{
				$q="
					select *
					from rez_page_template
					where rez_template_id ='".$templates['template_id']."';
				";
				$result3 = $global['db']->Execute($q);
				if($result3 === false) {
					daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez edit get_template");
				}
				$template_name=$result3->FetchRow();
				$template_select.='<option value="'.$template_name['rez_template_id'].'">'.$template_name['rez_menu_title'].'</option>';
			}	
			$template_select.='</optgroup>';

		}
		$template_select.='</select>';
		//creating page_select
		$page_select='<select name="Pages" id="suggest_page" style="display:none;" onclick="rez_render_fields(document.getElementById(\'suggest_page\').value,\'suggest_page\',2)">';
		$q="
			select *
			from rez_pages
			where rez_page_id > 0 and rez_menu_title != 'new page';
		";
		$result2 = $global['db']->Execute($q);
		if($result2 === false) {
			daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez edit get_disaster3");
		}
		while($pages = $result2->FetchRow()){
			$page_select.='<option value="'.$pages['rez_page_id'].'">'.$pages['rez_menu_title'].'</option>';
		}	
		$page_select.='</select>';
		$event_list=show_event_list($pageIdText);
		$editMainHtml =
			"<form id=\"editPageForm\" name=\"editPageForm\">
			<table class=\"mainTable\">
			<tr><td class=\"mainRowOdd\" style=\"text-align: center; text-weight: bold;\">"._t("ResourcePages-Table-Header|Page #")."</td><td class=\"mainRowOdd\"> &nbsp; <b>".$pageIdText."</b></td></tr>
			<tr><td class=\"mainRowEven\" style=\"text-align: center; text-weight: bold;\">"._t("ResourcePages-Table-Header|Use Exisiting ")."</td><td class=\"mainRowEven\" colspan=7><input name=\"inherit\" type=\"radio\" id=\"template\" value=\"template\" onclick=\"suggest('template',".$page_id.")\">Templates &nbsp;".$template_select." &nbsp; <input name=\"inherit\" type=\"radio\" id=\"page\" value=\"page\" onclick=\"suggest('page',".$page_id.")\">Pages &nbsp;".$page_select." &nbsp;<input name=\"inherit\" type=\"radio\" id=\"inherit\" onclick=\"suggest('none',".$page_id.")\" checked value=\"none\">None &nbsp;<b></b></td></tr>
			<tr><td class=\"mainRowOdd\" style=\"text-align: center; text-weight: bold;\">"._t("ResourcePages-Table-Header|Menu/Page Title")."</td><td class=\"mainRowOdd\"><input name=\"menuTitle\" type=\"text\" id=\"menuTitle\" value=\"".$row['rez_menu_title']."\" onchange=\"window.rezPageSaved = false;\" size=\"64\" /></td></tr>
			<tr><td class=\"mainRowEven\" style=\"text-align: center; text-weight: bold;\">"._t("ResourcePages-Table-Header|Locale")."</td><td class=\"mainRowEven\"><input name=\"locale\" type=\"text\" id=\"locale\" value=\"".$row['rez_locale']."\" onchange=\"window.rezPageSaved = false;\" size=\"64\" /></td></tr>
			<tr><td class=\"mainRowOdd\" style=\"text-align: center; text-weight: bold;\">"._t("ResourcePages-Table-Header|Events")."</td><td class=\"mainRowOdd\">".$event_list."</td></tr>
			<tr><td class=\"mainRowEven\" style=\"text-align: center; text-weight: bold;\">"._t("ResourcePages-Table-Header|Last Modified")."</td><td class=\"mainRowEven\"> &nbsp; ".$row['rez_timestamp']."</td></tr>
			</table>
			<div class=\"wysiwyg\"><textarea name=\"pageContent\" onchange=\"window.rezPageSaved = false;\" id=\"pageContent\" style=\"height: 300px; width: 100%;\">".$row['rez_content']."</textarea></div>
			</form>";
	}
	else{
		$editMainHtml =
			"<form id=\"editPageForm\" name=\"editPageForm\">
			<table class=\"mainTable\">
			<tr><td class=\"mainRowOdd\" style=\"text-align: center; text-weight: bold;\">"._t("ResourcePages-Table-Header|Page/Template #")."</td><td class=\"mainRowOdd\"> &nbsp; <b>".$pageIdText."</b></td></tr>
			<tr><td class=\"mainRowEven\" style=\"text-align: center; text-weight: bold;\">"._t("ResourcePages-Table-Header|Title")."</td><td class=\"mainRowEven\"><input name=\"menuTitle\" type=\"text\" id=\"menuTitle\" value=\"".$title."\" onchange=\"window.rezPageSaved = false;\" size=\"64\" /></td></tr>
			<tr><td class=\"mainRowOdd\" style=\"text-align: center; text-weight: bold;\">"._t("ResourcePages-Table-Header|Locale")."</td><td class=\"mainRowOdd\"><input name=\"locale\" type=\"text\" id=\"locale\" value=\"".$locale."\" onchange=\"window.rezPageSaved = false;\" size=\"64\" /></td></tr>
			<tr><td class=\"mainRowEven\" style=\"text-align: center; text-weight: bold;\">"._t("ResourcePages-Table-Header|Last Modified")."</td><td class=\"mainRowEven\"> &nbsp; ".$timestamp."</td></tr>
			";
		if($type ==='template')
		{
			$template_list=show_template_list($pageIdText);
			$editMainHtml.=
				"<tr><td class=\"mainRowOdd\" style=\"text-align: center; text-weight: bold;\">"._t("ResourcePages-Table-Header|Disaster Category")."</td><td class=\"mainRowOdd\"> &nbsp; ".$cats."</td></tr>
				<tr><td class=\"mainRowEven\" style=\"text-align: center; text-weight: bold;\">"._t("ResourcePages-Table-Header|Make changes to the following pages")."</td><td class=\"mainRowEven\"> &nbsp; ".$template_list."</td></tr>
				";
		}

		$editMainHtml.="	
			</table>
			<div class=\"wysiwyg\"><textarea name=\"pageContent\" onchange=\"window.rezPageSaved = false;\" id=\"pageContent\" style=\"height: 300px; width: 100%;\">".$content."</textarea></div>
			</form>";
	}

	if ($type === 'template')
	{
		$editControlHtml = '
			<input class="styleTehButton" type="button" onclick="javascript: window.rezPageSaved = true; if(nicEditors.editors.length>0){nicEditors.findEditor(\'pageContent\').saveContent();} rez_template_save(\''.$old_locale.'\','.$page_id.', xajax.getFormValues(\'editPageForm\'));" value="'._t("ResourcePages-EditForm-Button|Save and Publish Changes").'">';
	}
	else 
	{
		$editControlHtml='<input class="styleTehButton" type="button" onclick="javascript: window.rezPageSaved = true; if(nicEditors.editors.length>0){nicEditors.findEditor(\'pageContent\').saveContent();} rez_perform_save(\''.$old_locale.'\','.$page_id.', xajax.getFormValues(\'editPageForm\'));" value="'._t("ResourcePages-EditForm-Button|Save and Publish Changes").'">';
	}
	$editControlHtml.='<input class="styleTehButton" type="button" onclick="checkSaved(\''.$type.'\')" value="Close Editor">
		&nbsp; &nbsp; &nbsp; <span class="toggle">View editor as:
		<input id="wysiwyg" class="styleTehButton" type="button" onclick="javascript: document.getElementById(\'wysiwyg\').disabled = true; document.getElementById(\'htmlcode\').disabled = false; area2 = new nicEditor({fullPanel : true}).panelInstance(\'pageContent\');" value="WYSIWYG">
		<input id="htmlcode" class="styleTehButton" type="button" onclick="javascript: area2.removeInstance(\'pageContent\'); document.getElementById(\'wysiwyg\').disabled = false; document.getElementById(\'htmlcode\').disabled = true; nicEditors.editors = []; " value="HTML"></span>';

	$global['xajax_res']->addAssign('rezMain','style.opacity','1.0');
	$global['xajax_res']->addAssign('rezMain','innerHTML',$editMainHtml);

	$global['xajax_res']->addAssign('rezControl','style.opacity','1.0');
	$global['xajax_res']->addAssign('rezControl','innerHTML',$editControlHtml);

	$global['xajax_res']->addScript('area2 = new nicEditor({fullPanel : true}).panelInstance(\'pageContent\'); document.getElementById(\'wysiwyg\').disabled = true;');
	$global['xajax_res']->addScript("chosen();");
	return $global['xajax_res']->getXML();
}



// delete a page
function rez_perform_delete($type,$page_id, $locale, $confirm) {
	global $global;
	if($confirm == "true")    // Implies delete this page
	{
		if(!$locale)    // if locale is not set, then set it to default -> en_US
		{

			$locale='en_US';
		}
		if($type === 'template')   //Deleting a template
		{
			$q = "
				delete from rez_page_template
				where rez_template_id = '".$page_id."'
				and rez_template_locale = '".$locale."';
			";
			//Delete the mapping b/w template and disaster_category
			$q2 ="
				delete from rez_template_scenario
				where template_id = '".$page_id."';
			";
		}
		else
		{
			//---- delete the page from db table
			$q = "
				delete from rez_pages
				where rez_page_id = '".$page_id."'
				and rez_locale = '".$locale."';
			";

			// ---- Delete the page's mapping with the incident
			$q2 ="
				delete from rez_incident
				where page_id = '".$page_id."';
			";


		}
		$result = $global['db']->Execute($q);
		if($result === false) {
			daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez delete 1");
		}
		$result = $global['db']->Execute($q2);
		if($result === false) {
			daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez delete 1");
		}

		if($type ==='template')
		{//Display the template's table 
			rez_show_templates();
		}

		$global['xajax_res']->addAppend('rezLog','innerHTML',_t("ResourcePages-Deleted-Message|Page/Template <b>#").$page_id."</b>"._t("ResourcePages-Deleted-Message| deleted.")."<br>");
	} else {
		$global['xajax_res']->addAppend('rezLog','innerHTML',_t("ResourcePages-Message|Delete operation aborted.")."<br>");
	}
	if($type !== 'template'){
		//---- update UI
		$global['xajax_res']->addAssign('rezControl','style.opacity','1.0');
		$global['xajax_res']->addAssign('rezMain','innerHTML',rez_show_pages(TRUE));
		$global['xajax_res']->addAssign('rezMain','style.opacity','1.0');
		$global['xajax_res']->addScript("setTimeout('e = document.getElementById(\'rezLog\'); e.scrollTop = e.scrollHeight+1000;', 500);");
	}
	return $global['xajax_res']->getXML();
}

/**
 * Translate a single page
 * @param string $page_id
 */
function rez_perform_translate($type,$page_id, $locale) {
	global $global;
	if($type !== 'template')
	{
		// get page info
		$q = "
			select *
			from rez_pages
			where rez_page_id = '".$page_id."'
			and rez_locale = '".$locale."';
		";
		$result = $global['db']->Execute($q);
		if($result === false) {
			daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez translate 1");
		}
		$original_row = $result->FetchRow();
		$original_title= $original_row['rez_menu_title'];
		$original_locale=$original_row['rez_locale'];
		$original_content=$original_row['rez_content'];
		$original_time=$original_row['rez_timestamp'];

		if ( isset($_SESSION['rez_locale']) ) {
			$rez_locale = $_SESSION['rez_locale'];
		}
		else $rez_locale = shn_rez_getlocale();
		$q = "
			select *
			from rez_pages
			where rez_page_id = '".$page_id."'
			and rez_locale = '".$rez_locale."';
		";
		$result = $global['db']->Execute($q);
		if($result === false) {
			daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez translate 1");
		}
		$row = $result->FetchRow();
		$new_content='';
		$new_title= $row['rez_menu_title'];
		$new_content=$row['rez_content'];
		$new_time=$row['rez_timestamp'];
		if (($new_content === '') or ($new_content === NULL))
		{
			$new_content ='Blank Content';
		}
	}
	else
	{
		$q = "
			select *
			from rez_page_template
			where rez_template_id = '".$page_id."'
			and rez_template_locale = '".$locale."';
		";
		$result = $global['db']->Execute($q);
		if($result === false) {
			daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez translate 1");
		}
		$original_row = $result->FetchRow();
		$original_title= $original_row['rez_menu_title'];
		$original_locale=$original_row['rez_template_locale'];
		$original_content=$original_row['rez_template_content'];
		$original_time=$original_row['rez_template_timestamp'];

		if ( isset($_SESSION['rez_locale']) ) {
			$rez_locale = $_SESSION['rez_locale'];
		}
		else $rez_locale = shn_rez_getlocale();
		$q = "
			select *
			from rez_page_template
			where rez_template_id = '".$page_id."'
			and rez_template_locale = '".$rez_locale."';
		";
		$result = $global['db']->Execute($q);
		if($result === false) {
			daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rez translate 1");
		}
		$row = $result->FetchRow();
		$new_title= $row['rez_menu_title'];
		$new_content=$row['rez_template_content'];
		if (($new_content === '') or ($new_content === NULL))
		{
			$new_content ='Blank Content';
		}

		$new_time=$row['rez_template_timestamp'];
	}


	$pageIdText = $page_id;
	$editMainHtml =
		"<form id=\"editPageForm\" name=\"editPageForm\">
		<table class=\"mainTable\">
		<tr><td class=\"mainRowOdd\" style=\"text-align: center; text-weight: bold;\">"._t("ResourcePages-Table-Header|Page #")."</td><td class=\"mainRowOdd\"> &nbsp; <b>".$pageIdText."</b></td>
		<td class=\"mainRowOdd\" style=\"text-align: center; text-weight: bold;\">"._t("ResourcePages-Table-Header|Page #")."</td><td class=\"mainRowOdd\"> &nbsp; <b>".$pageIdText."</b></td></tr>
		<tr><td class=\"mainRowEven\" style=\"text-align: center; text-weight: bold;\">"._t("ResourcePages-Table-Header|Menu/Page Title")."</td><td class=\"mainRowEven\"><input name=\"old_menuTitle\" type=\"text\" id=\"menuTitle\" value=\"".$original_title."\" onchange=\"window.rezPageSaved = false;\" size=\"64\" /></td>
		<td class=\"mainRowEven\" style=\"text-align: center; text-weight: bold;\">"._t("ResourcePages-Table-Header|Menu/Page Title")."</td><td class=\"mainRowEven\"><input name=\"menuTitle\" type=\"text\" id=\"menuTitle\" value=\"".$new_title."\" onchange=\"window.rezPageSaved = false;\" size=\"64\" /></td></tr>
		<tr><td class=\"mainRowOdd\" style=\"text-align: center; text-weight: bold;\">"._t("ResourcePages-Table-Header|Locale")."</td><td class=\"mainRowOdd\"><input name=\"old_slocale\" type=\"text\" id=\"locale\" value=\"".$original_locale."\" onchange=\"window.rezPageSaved = false;\" size=\"64\" /></td>
		<td class=\"mainRowOdd\" style=\"text-align: center; text-weight: bold;\">"._t("ResourcePages-Table-Header|Locale")."</td><td class=\"mainRowOdd\"><input name=\"locale\" type=\"text\" id=\"locale\" value=\"".$rez_locale."\" onchange=\"window.rezPageSaved = false;\" size=\"64\" /></td></tr>
		<tr><td class=\"mainRowEven\" style=\"text-align: center; text-weight: bold;\">"._t("ResourcePages-Table-Header|Last Modified")."</td><td class=\"mainRowEven\"> &nbsp; ".$original_timestamp."</td>
		<td class=\"mainRowEven\" style=\"text-align: center; text-weight: bold;\">"._t("ResourcePages-Table-Header|Last Modified")."</td><td class=\"mainRowEven\"> &nbsp; ".$new_time."</td></tr>
		</table>
		<div class=\"wysiwyg\"><table width=\"100%\" border=\"0\">"
		."<tr><td width=\"50%\"><div id=\"original\" onmouseup=\"checkSelection()\" style=\"height: 100%; width: 100%;\">".$original_content."</div></td>"
		."<td width=\"50%\" style=\"vertical-align:top;\"><textarea name=\"pageContent\" onchange=\"window.rezPageSaved = false;\" id=\"pageContent\" style=\"height: 300px; width: 90%;\">".$new_content."</textarea></td></tr>"
		."</table></div>
		</form>";
	if ($type !== 'template'){
		$pageControlHtml = '
			<input class="styleTehButton" type="button" onclick="javascript:  window.rezPageSaved = true; if(nicEditors.editors.length>0){nicEditors.findEditor(\'pageContent\').saveContent();} rez_perform_save(\'translated\','.$page_id.', xajax.getFormValues(\'editPageForm\'));" value="'._t("ResourcePages-EditForm-Button|Save and Publish Changes").'"> 
			<input class="styleTehButton" type="button" onclick="checkSaved(\'template\')" value="Close Editor">';}
	else{
		$pageControlHtml = '
			<input class="styleTehButton" type="button" onclick="javascript:  window.rezPageSaved = true; if(nicEditors.editors.length>0){nicEditors.findEditor(\'pageContent\').saveContent();} rez_template_save(\'translated\','.$page_id.', xajax.getFormValues(\'editPageForm\'));" value="'._t("ResourcePages-EditForm-Button|Save and Publish Changes").'">
			<input class="styleTehButton" type="button" onclick="checkSaved(\'template\')" value="Close Editor">';}
	$pageControlHtml .= '
		<input class="styleTehButton" type="button" onclick=\'insertAtCursor()\' value="Translate">
		&nbsp; &nbsp; &nbsp; <span class="toggle">View editor as:
		<input id="wysiwyg" class="styleTehButton" type="button" onclick="javascript: document.getElementById(\'original\').innerHTML = unescape(\''.rawurlencode($original_row['rez_content']).'\'); document.getElementById(\'wysiwyg\').disabled = true; document.getElementById(\'htmlcode\').disabled = false; area2 = new nicEditor({fullPanel : true}).panelInstance(\'pageContent\');" value="WYSIWYG">
		<input id="htmlcode" class="styleTehButton" type="button" '
		.'onclick="javascript: var s = document.getElementById(\'original\').innerHTML;document.getElementById(\'original\').innerHTML = \'<textarea style=width:90%;height:300px disabled=disabled>\'+s+\'</textarea>\';'
		.'area2.removeInstance(\'pageContent\'); document.getElementById(\'wysiwyg\').disabled = false; document.getElementById(\'htmlcode\').disabled = true; nicEditors.editors = []; " value="HTML"></span>';
	$pageControlHtml .= "<select name=\"locale\" onchange=\"javascript: rez_perform_change_locale('".$type."',this.value, '".$pageIdText."','".$locale."'); \">";
	//@TODO: textarea values: disabled="yes" height="300px" width="100%"
	foreach ( $global['rez_locales'] as $key=>$value ) {
		if ($key == $rez_locale) {
			$pageControlHtml .= "<option value=\"".$key."\" selected=\"selected\">".$value."</option>";
		}
		else $pageControlHtml .= "<option value=\"".$key."\">".$value."</option>";
	}
	$pageControlHtml .= "</select>";

	$global['xajax_res']->addAssign('rezMain','style.opacity','1.0');
		$global['xajax_res']->addAssign('rezMain','innerHTML',$editMainHtml);
	$global['xajax_res']->addAssign('rezControl','style.opacity','1.0');
		$global['xajax_res']->addAssign('rezControl','innerHTML',$pageControlHtml);
	$global['xajax_res']->addScript('area2 = new nicEditor({fullPanel : true}).panelInstance(\'pageContent\'); document.getElementById(\'wysiwyg\').disabled = true;');

	return $global['xajax_res']->getXML();
}

// presents a gui to edit a page and its corresponding fields
function rez_show_help() {
	global $global;
	// help content
	$editMainHtml = "
		<h2>Creating a New Page</h2>
		<ul>
		<li>Clicking the <b>Create new Resource Page</b> button or<b> Create new System Page</b> button will create a new page with default sample data.</li>
		<li>Once the new page is created, you are immediately redirected to begin editing it.</li>
		</ul>
		<br>
		<h2>Editing a Page</h2>
		<ul>
		<li>Any changes made to the page are not saved until you click the <b>Save and Publish Changes</b> button.</li>
		<li>If you click <b>Close Editor</b> before saving , changes are not saved and your edits are lost.</li>
		<li><b>Use Existing</b> shows the suggestions available for creating a page. They are Page Templates, Existing Resource Pages or None of these (new content).</br>This option is available only for Resource Pages and not for System Page.</li>
		<li><b>Menu/Page Title</b> refers to the title of the Page.</li>
		<li><b>Locale</b> refers to the locale of the Page. </li>
		</ul>
		<br>
		<h2>User Interface Definitions</h2>
		<br>
		<ul>
		<img alt=\"IE6,7,8 dont support inline images, please upgrade to IE9 or Google Chrome, Firefox, Safari\" class=\"inlineHelp\" src=\"data:image/png;base64,".base64_encode(file_get_contents($global['approot'].'/mod/rez/help1.png'))."\">
		<br><br>
		<li><b>Page#</b> shows the page number</li>
		<li><b>Menu/Page Title</b> is the title of the Resource Page.</li>
		<li><b>Visibility</b> indicates whether the page is a <b>PUBLIC</b>,<b>INTERNAL</b> or <b>HIDDEN</b>.</li>
		<li><b>Locale</b> indicates the locale of the Page.</li>
		<li><b>Edit</b> is the link you click on to edit this particular Page.</li>
		<li><b>Delete</b> is the link you click on to delete this particular Page.</li>
		<li><b>Translate</b> is the link you click on to translate this particular Page.</li>
		<li><b>Move</b> allows you to change the display order of Pages</li>
		</ul>
		<br>
		<br>
		<ul>
		<img alt=\"IE6,7,8 dont support inline images, please upgrade to IE9 or Google Chrome, Firefox, Safari\" class=\"inlineHelp\" src=\"data:image/png;base64,".base64_encode(file_get_contents($global['approot'].'/mod/rez/help2.png'))."\">
		<br>
		<br>
		<br>
		<li><b>Event ID#</b> is the internal event number used by Sahana is is automatically assigned.</li>
		<li><b>Description</b>: is a paragraph of text describing the event in more detail.</li>
		<li><b>Date</b>: the date on which the event occurred.</li>
		<li><b>Street Address</b>: is used to lookup a location when editing or creating a new event.</li>
		<li><b>Latitude / Longitude</b>: non-editable fields which are updated as the map is updated.</li>
		</ul>
		<br>
		<br>
		<ul>
		<li><b>Log</b>: the log displays notifications as you are using the Event Manager to let you understand what is going on. Example: when started it shows how many events are presnet or when saving or canceling an edit, the status of this operation appears in the log so that a user can see when this operation is complete.

		</ul>
		<br>
		<br>
		";


	$editControlHtml = '<input class="controlButton" type="button" onclick="javascript: rez_append_log(\'Closing help...<br>\'); setTimeout(\'rez_show_pages();\', 250);" value="Close Help">';

	$global['xajax_res']->addAssign('rezMain','style.opacity','1.0');
	$global['xajax_res']->addAssign('rezMain','innerHTML',$editMainHtml);

	$global['xajax_res']->addAssign('rezControl','style.opacity','1.0');
	$global['xajax_res']->addAssign('rezControl','innerHTML',$editControlHtml);

	return $global['xajax_res']->getXML();
}
/**
 *
 * @param string $locale
 * @author Ramindu Deshapriya
 */
function rez_perform_change_locale($type,$locale, $pageId, $oldLocale) {
	global $global;
	$_SESSION['rez_locale'] = $locale;
	rez_perform_translate($type,$pageId, $oldLocale);
	$global['xajax_res']->addAssign('rezControl','style.opacity','1.0');
		$global['xajax_res']->addAssign('rezMain','style.opacity','1.0');
		$global['xajax_res']->addScript("setTimeout('e = document.getElementById(\'rezLog\'); e.scrollTop = e.scrollHeight+1000;', 500);");
		
		return $global['xajax_res']->getXML();
}



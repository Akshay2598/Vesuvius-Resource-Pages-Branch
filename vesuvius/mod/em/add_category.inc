<?php
/**
 * @author       Mayank Jain<mayankkrjain@gmail.com>
 * @about        Developed for the Sahana Foundation (GSOC 13). 
 * @link         http://sahanafoundation.org
 * @license      http://www.gnu.org/licenses/lgpl-2.1.html GNU Lesser General Public License (LGPL)
 * @lastModified 2013.13.09
 */

function add_category($category,$incident_id){
	//This function creates a multiple select dropdown of disaster categories. 
	//If $category variable has a value 'main_list' , it means simply create a multiple select dropdown.
	//Else add $category to the disaster_category table in the db. Then return a new updated disaster dropdown.
	global $global;
	if ($category !== 'main_list'){
		//No addition of category
		$q="
			INSERT INTO disaster_category
			(name)
			VALUES
			('".$category."');	
		";
		$result=$global['db']->Execute($q);
		if ($result==false){ 
			daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "insert new disaster category");
		}}
	//getting list of categories associated with this disaster --- pre-selected values
	$names=array();
	$q= " 
		SELECT *
		FROM scenario 
		WHERE `incident_id` = ".$incident_id.";
	";
	$categories = $global['db']->Execute($q);
	if($categories === false) {
		daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "generate category list");
	}
	while($category = $categories->FetchRow()){
		//get the name of the disaster category from ids
		$q="
			SELECT *
			FROM disaster_category
			WHERE `category_id` =".$category['disaster_type'].";
		";
		$category_name = $global['db']->Execute($q);
		if($category_name === false) { 
			daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "generate category list2");
		}
		$name=$category_name->FetchRow();
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
	if($categories === false) { 
		daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "generate category list3"); 
	}
	$cats.='<option value=""></option>'; // for showing data-placeholder we need to have a blank option.
	while($name=$categories->FetchRow()){
		if ($name['name'] == "Default")//No need to show the Default category. If no category is selected it will be automatically assigned.
		{		
			continue;
		}
		if(in_array($name['name'],$names)){
			$cats.='<option selected value="'.$name['category_id'].' ">' . $name['name'] . '</option>';
		}
		else	{
			$cats.='<option value="'.$name['category_id'].'">' . $name['name'] . '</option>';
		}
	}
	$cats.='</select>';
	return $cats;
}

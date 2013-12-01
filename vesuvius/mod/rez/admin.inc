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


include_once ($global['approot'].'/mod/rez/xajax.inc');
shn_xajax_registerFunction('rez_append_log');
shn_xajax_registerFunction('rez_prepend_log');
shn_xajax_registerFunction('rez_show_message');
shn_xajax_registerFunction('rez_render_fields');
shn_xajax_registerFunction('rez_show_pages');
shn_xajax_registerFunction('rez_show_templates');
shn_xajax_registerFunction('rez_show_help');
shn_xajax_registerFunction('rez_perform_edit');
shn_xajax_registerFunction('rez_perform_delete');
shn_xajax_registerFunction('rez_perform_insert');
shn_xajax_registerFunction('rez_get_translation');
shn_xajax_registerFunction('rez_perform_save');
shn_xajax_registerFunction('rez_template_save');
shn_xajax_registerFunction('rez_perform_move_up');
shn_xajax_registerFunction('rez_perform_move_down');
shn_xajax_registerFunction('rez_perform_change_visibility');
shn_xajax_registerFunction('rez_perform_change_locale');
shn_xajax_registerFunction('rez_perform_translate');


/** setup admin GUI for modRez and then let xajax do all the work */
function shn_rez_adm_default() {
	shn_xajax_printJavascript();
	?>
		<div id="rezLog" class="logArea">Loading Pages...<br></div>
		<div id="rezControl" class="controlArea">&nbsp;</div>
		<div id="rezMain" class="mainArea">&nbsp;</div>
		<script src="res/js/jquery-1.6.4.min.js" type="text/javascript"></script>

		<script src="res/js/chosen.jquery.js" type="text/javascript"></script>
		<script type="text/javascript" src="res/js/jquery-ui-autocomplete.min.js" ></script>
		<script type="text/javascript" src="res/js/jquery-ui-datepicker.min.js"></script>
		<script>
		rez_show_message("Loading Pages...");
	setTimeout("rez_show_pages(0,<? echo isset($_GET['template'])?1:0?>);", 1500);
	window.rezPageSaved = true;

	function checkSaved(type) {
		if(!window.rezPageSaved) {
			var answer = confirm('Are you sure you want to close this document? It has not been saved.');
			if(answer) {
				rez_append_log('Closing editor...<br>');
				area2.removeInstance('pageContent');
				if (type == 'template'){
					setTimeout('rez_show_templates();', 500);}
				else {
					setTimeout('rez_show_pages();', 500);}
				window.rezPageSaved = true;
			}
		} else {
			rez_append_log('Closing editor...<br>');
			area2.removeInstance('pageContent');
			if (type == 'template'){
				setTimeout('rez_show_templates();', 500);}
			else {
				setTimeout('rez_show_pages();', 500);}

			window.rezPageSaved = true;
		}
	}
	var translation_input='';
	function insertAtCursor(){
		var editor = nicEditors.findEditor('pageContent');
		var range = editor.getRng(); 
		var editorField = editor.selElm();
		editorField.nodeValue = editorField.nodeValue.substring(0, range.startOffset) +
			translation_input +
			editorField.nodeValue.substring(range.endOffset, editorField.nodeValue.length);
	}
	function checkSelection(){
		translation_input=window.getSelection();
		translation_input=translation_input.toString();
	}
	function toggle(one,two){
		if(one.style.display =='none'){
			one.style.display= 'inline';
			two.style.display='none';
		}
	}
	function chosen() {
		$(".chzn-select-no-results").chosen({no_results_text: "No matching categories for : "});
	}
	function suggest(val,id)	{
		var one = document.getElementById('suggest_page'); 
		var two = document.getElementById('suggest_template');
		if (val == 'page'){
			toggle(one,two);
		}
		else if (val == 'template'){
			toggle(two,one) 
		}
		else{
			one.style.display='none';
			two.style.display='none';
			rez_render_fields(id,'suggest_page',1);
		}
	}

	</script>
		<script type="text/javascript" src="res/nicedit/nicEdit.js"></script><?
}

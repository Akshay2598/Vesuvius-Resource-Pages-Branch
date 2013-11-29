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
shn_xajax_registerFunction('rez_show_pages');
shn_xajax_registerFunction('rez_show_help');
shn_xajax_registerFunction('rez_perform_edit');
shn_xajax_registerFunction('rez_perform_delete');
shn_xajax_registerFunction('rez_perform_insert');
shn_xajax_registerFunction('rez_perform_save');
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
		<script>
		rez_show_message("Loading Pages...");
	setTimeout("rez_show_pages();", 1500);
	window.rezPageSaved = true;

	function checkSaved(id) {
		if(!window.rezPageSaved) {
			var answer = confirm('Are you sure you want to close this document? It has not been saved.');
			if(answer) {
				rez_append_log('Closing editor...<br>');
				area2.removeInstance('pageContent');
				setTimeout('rez_show_pages();', 500);
				window.rezPageSaved = true;
			}
		} else {
			rez_append_log('Closing editor...<br>');
			area2.removeInstance('pageContent');
			setTimeout(function(){rez_perform_delete(id,'',true);},200)
			setTimeout('rez_show_pages();', 500);
			window.rezPageSaved = true;
		}
	}
	function toggle(one,two){
		if(one.style.display =='none'){
			one.style.display= 'inline';
			two.style.display='none';
		}
	}
	function getXMLHTTP() {
		var x = false;
		try {
			x = new XMLHttpRequest();
		}catch(e) {
			try {
				x = new ActiveXObject("Microsoft.XMLHTTP");
			}catch(ex) {
				try {
					req = new ActiveXObject("Msxml2.XMLHTTP");
				}
				catch(e1) {
					x = false;
				}
			}
		}
		return x;
	}	
	function render_fields(id)	{
		var value = document.getElementById(id).value;
		var strURL="../../mod/rez/render_fields.php?type="+id+"&id="+value+"&actual=2";
		var req = getXMLHTTP();
		if (req){
			req.onreadystatechange = function(){
				if (req.readyState == 4){
					
					// only if "OK"
					if (req.status == 200){
						var div = window.content.document.createElement('div');
						div.innerHTML = req.responseText;
						var title = ((div.getElementsByTagName('titles'))[0].innerHTML);
						var content = (((div.getElementsByTagName('textarea'))[0].innerHTML));
						div.innerHTML = content;
						var content = div.firstChild.nodeValue;	
						document.getElementById('menuTitle').value=title;
						document.getElementById('pageContent').value=content;
						nicEditors.findEditor('pageContent').setContent(content);
					} else {
						alert("There was a problem while using XMLHTTP:\n" + req.statusText);
					}
				}
			}
			req.open("GET", strURL, true);
			req.send(null);
		}
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
			var value = document.getElementById('menuTitle').value;
			one.style.display='none';
			two.style.display='none';
			var strURL="../../mod/rez/render_fields.php?type=suggest_page&id="+id+"&actual="+1;
			var req = getXMLHTTP();
			if (req){
				req.onreadystatechange = function(){
					if (req.readyState == 4){
						if (req.status == 200)	{
							var div = window.content.document.createElement('div');
							div.innerHTML = req.responseText;
							var title = ((div.getElementsByTagName('titles'))[0].innerHTML);
							document.getElementById('menuTitle').value=title;
							var content = (((div.getElementsByTagName('textarea'))[0].innerHTML));
							div.innerHTML = content;
							var content = div.firstChild.nodeValue;	
							document.getElementById('menuTitle').value=content;
							nicEditors.findEditor('pageContent').setContent(content);
						} else {
							alert("There was a problem while using XMLHTTP:\n" + req.statusText);
						}
					}
				}
				req.open("GET", strURL, true);
				req.send(null);
			}
		}
	}

 
	</script>
		<script type="text/javascript" src="res/nicedit/nicEdit.js"></script><?
}

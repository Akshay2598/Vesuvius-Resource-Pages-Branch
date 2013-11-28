<?php
/**
 * @name         Diagnose Vesuvius
 * @version      1.0
 * @author       Akshay Kalose <AkshayProductions@gmail.com>
 * @about        Developed in whole or part by the U.S. National Library of Medicine
 * @link         https://pl.nlm.nih.gov/about
 * @link         http://sahanafoundation.org
 * @license	 http://www.gnu.org/licenses/lgpl-2.1.html GNU Lesser General Public License (LGPL)
 * @lastModified 2013.1123
 */

$global['approot']  = realpath(dirname(__FILE__)).'/../';

require($global['approot'].'conf/sahana.conf');

// include the base libraries
require_once($global['approot'].'inc/lib_config.inc');
require_once($global['approot'].'inc/lib_modules.inc');
require_once($global['approot'].'inc/lib_errors.inc');

// include the main libraries the system depends on
require_once($global['approot'].'inc/handler_db.inc');
require_once($global['approot'].'inc/lib_security/lib_crypt.inc');
require_once($global['approot'].'inc/handler_session.inc');
//require_once($global['approot'].'inc/lib_security/handler_openid.inc'); // replacing openID lib soon....
require_once($global['approot'].'inc/lib_locale/handler_locale.inc');
require_once($global['approot'].'inc/lib_exception.inc');
require_once($global['approot'].'inc/lib_user_pref.inc');

// Check if required modules are installed for Vesuvius to function
shn_diagnose_checkRequiredModules();


function shn_diagnose_checkRequiredModules()
{
	global $global;
	global $conf;
	$results = array();
	// Checks:
	// PHP version compatibility
	// PHP CURL
	// Successful DB connection (username, password, host)
	// MySQL time zone tables and if they are populated
	// MySQL DB stored procedures - "PLSearch", "PLSearch2"

	// Check for correct PHP Version
	if ( version_compare(PHP_VERSION, '5.0.0', '<=') ) {
		$results[] = ('Please update your PHP version('.PHP_VERSION.'). It is not supported by Vesuvius.');
	}

	// Check if PHP CURL is working 
	if (!function_exists('curl_version')) {
		$results[] = ('PHP CURL is required for Vesuvius to function properly.');
	}

	// START MYSQL CHECKS
	// New connection to check time tables
 	$timetableconnection = NewADOConnection($conf['db_engine']);

	// Get Databse Port
	$_host_port = $conf['db_host'].(isset($conf['db_port']) ? ':'.$conf['db_port']:'');
	
	// Try to connect to mysql database
	if(!$timetableconnection->Connect($_host_port, $conf['db_user'], $conf['db_pass'], 'mysql')) {
		$results[] = ('Could not establish connection to mysql database.');
	}
	
	// Set array for all time tables to check
	$timezonetables = array(
	'time_zone',
	'time_zone_leap_second',
	'time_zone_name',
	'time_zone_transition',
	'time_zone_transition_type',
	);

	// Check each time table and see if they exist.
	foreach ($timezonetables as $key => $value) {
		$column = $timetableconnection->GetCol('SELECT 1 FROM '.$value);
		if ($column === false) {
			$results[] = $value.' table does not exist in the mysql database.';
		}
	}

	// Checks if time tables are populated and properly working. Both return string(2007-03-11 1:00:00) if found else false.
	if ($timetableconnection->GetOne("SELECT CONVERT_TZ('2007-03-11 2:00:00','US/Eastern','US/Central')") !== $timetableconnection->GetOne("SELECT CONVERT_TZ('2007-03-11 3:00:00','US/Eastern','US/Central')")) {
		$results[] = "Time tables are not populated properly.";
	}

	// Check is connection to database can be made.
	if(!$global['db']->Connect($_host_port, $conf['db_user'], $conf['db_pass'], $conf['db_name'])) {
		$results[] = ('Could not establish connection to database.');
	}


	// Set array for all routines to check
	$routines = array(
		'delete_reported_person',
		'PLSearch',
		'PLSearch2',
		'PLSearch_Count',
	);

	// Check each routine/procedure to see if they exist.
	foreach ($routines as $key => $value) {
		if ($global['db']->GetRow("SELECT routine_definition FROM information_schema.routines WHERE routine_schema = '{$conf['db_name']}' AND routine_name = '{$value}'") === array()) {
			$results[] = 'The routine/procedure '.$value.' does not exist.';
		}
	}

	// Report all errors as html list.
	echo "<ol>";
	foreach ($results as $key => $value) {
		echo "<li>{$value}</li>";
	}
	echo "</ol>";
	die();
}
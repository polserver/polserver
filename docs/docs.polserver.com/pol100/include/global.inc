<?php

// **** CONFIGURATION **********************************************************
// NOTE:
// An online website can define its own $header to prevent default one from being included
// also $official, $analytics and other site-specific variables can be defined there

// Will be set to true when official site is detected
$official = false;

//$analytics = '
//     <script src="http://www.google-analytics.com/urchin.js" type="text/javascript">
//     </script>
//     <script type="text/javascript">
//     _uacct = "UA-2869696-3";
//     urchinTracker();
//     </script>'

define('OFFICIAL_HEADER_PATH', '/home/polserve/include/_header.php');

// Will be set to true when building the offline doc
$offline = false;

// **** CONFIG END *************************************************************



// Sets a default error handler, transforms any warning into a critical error
set_error_handler('XML_errorHandler');

function XML_errorHandler ($errno, $errstr, $errfile, $errline, $errcontext)
{
	global $offline;
	if( $offline ) {
		@ob_end_clean();
	}
	die("ERROR $errno in \"$errfile\" at line $errline: \"$errstr\"\n");
}

// Sets the default timezone (needed on official website)
if (function_exists('date_default_timezone_set')) {
	if( $envtz = getenv("TZ") )
		@date_default_timezone_set($envtz);
}

// PHP-BB global stuff
if( isset($request) )
	$request->enable_super_globals();

// The official header does not like to be put into a function, so what i am
// doing instead is caching the output now and using it later
if( file_exists(OFFICIAL_HEADER_PATH) ) {
	ob_start();
	include OFFICIAL_HEADER_PATH;
	$header = ob_get_contents();
	ob_end_clean();
	$official = true;
}

// **** FUNCTIONS **************************************************************

/**
 * Gets an HTTP GET parameter (use this instead of $_GET)
 *
 * @param $name The name of the GET parameter to retrieve
 * @param $default The default value to return if not found (default: null)
 * @return The retrieved parameter or default value
 */
function httpget($name, $default='')
{
	global $official;

	if ($official)
		return request_var($name, $default);

	if (isset($_GET[$name]))
		return $_GET[$name];

	return $default;
}

/**
* Outputs the page header, from doctype to body
*
* @param $title string: The page title
*/
function siteheader($title)
{
	global $offline, $official, $header;

	if( isset($header) )
		echo $header;
	else
		require 'header.inc';
}

/**
* Outputsthe page footer
*/
function sitefooter()
{
	global $offline, $official;

	require 'footer.inc';
}


/**
* Compile and output an xlst document
*
* @param $docname string: The .xslt file name
* @param $tplname string: The .xml template file name
*/
function xlstdocument($docname, $tplname)
{
	global $offline;

	$xsltproc = new XsltProcessor();
	$xsl = new DomDocument;
	$xsl->load($docname);
	$xsltproc->importStylesheet($xsl);
	$xml_doc = new DomDocument;
	$xml_doc->load($tplname);
	$xsltproc->setParameter('', 'offline', $offline);

	if ($html = $xsltproc->transformToXML($xml_doc))
		echo $html;
}

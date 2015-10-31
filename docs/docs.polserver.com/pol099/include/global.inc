<?php

// **** CONFIGURATION **********************************************************
// NOTE:
// An online website can define its own $header to prevent default one from being included
// also $official, $analytics and other site-specific variables can be defined there

// Set this to true on the official website
$official = false;

//$analytics = '
//     <script src="http://www.google-analytics.com/urchin.js" type="text/javascript">
//     </script>
//     <script type="text/javascript">
//     _uacct = "UA-2869696-3";
//     urchinTracker();
//     </script>'

define('OFFICIAL_HEADER_PATH', '/home/polteam/include/_header.php');
if( file_exists(OFFICIAL_HEADER_PATH) )
	$header = OFFICIAL_HEADER_PATH;

// Will be set to true when building the offline doc
$offline = false;

// **** CONFIG END *************************************************************



// Sets a default error handler, transforms any warning into a critical error
set_error_handler('XML_errorHandler');

function XML_errorHandler ($errno, $errstr, $errfile, $errline, $errcontext)
{
	global $offline;
	if ($offline) {
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



// **** FUNCTIONS **************************************************************

/**
* Outputs the page header, from doctype to body
*
* @param $title string: The page title
*/
function siteheader($title)
{
	global $offline, $official, $header;

	if( isset($header) ) {
		echo $header;
		return;
	}

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

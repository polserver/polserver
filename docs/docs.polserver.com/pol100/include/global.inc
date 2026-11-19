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



// Sets a default error handler: when building the offline doc, transforms any
// warning into a critical error. Online, errors are left to PHP (or to phpBB's
// handler, which replaces this one once the official header is loaded), so
// server paths are not shown to visitors.
set_error_handler('XML_errorHandler');

function XML_errorHandler ($errno, $errstr, $errfile, $errline)
{
	global $offline;
	if( !$offline || !(error_reporting() & $errno) ) {
		return false;
	}
	@ob_end_clean();
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
// (a page that outputs no header, like searchindex.php, defines OFFICIAL_HEADER_SKIP)
if( !defined('OFFICIAL_HEADER_SKIP') && file_exists(OFFICIAL_HEADER_PATH) ) {
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
* @param $params array: Extra stylesheet parameters (name => value)
*/
function xlstdocument($docname, $tplname, $params = array())
{
	global $offline;

	$xsltproc = new XsltProcessor();
	$xsl = new DomDocument;
	$xsl->load($docname);
	$xsltproc->importStylesheet($xsl);
	$xml_doc = new DomDocument;
	$xml_doc->load($tplname);
	$xsltproc->setParameter('', 'offline', $offline);
	foreach ($params as $name => $value)
		$xsltproc->setParameter('', $name, $value);

	if ($html = $xsltproc->transformToXML($xml_doc))
		echo $html;
}

/**
* Returns the class names of the object reference as "|Name|Name|...|", the form
* escript.xslt takes them in to link "<Class> Reference" parameter types.
* (The stylesheet does not read objref.xml itself: its DTD pulls in an external one.)
*/
function objrefclasses()
{
	$names = array();
	$xml = simplexml_load_file('objref.xml');
	if ($xml !== false)
		foreach ($xml->class as $class)
			$names[] = (string)$class['name'];
	return '|' . implode('|', $names) . '|';
}

/**
* Returns the first sentence of a text, with whitespace collapsed
*/
function firstsentence($text)
{
	$text = trim(preg_replace('/\s+/', ' ', (string)$text));
	$parts = preg_split('/(?<=\.)\s/', $text, 2);
	return $parts[0];
}

/**
* Builds the search index used by docs.js: every module and every function,
* as a script that sets window.POL_DOCS_INDEX. Each entry is
* [name, module file, module nice name, first sentence of the description];
* modules have an empty name.
*
* @return string: The script source
*/
function searchindexscript()
{
	$index = array();
	foreach (simplexml_load_file('modules.xml') as $em) {
		$file = (string)$em['name'];
		$nice = (string)$em['nice'];
		$xml = simplexml_load_file($file.'.xml');
		if ($xml === false)
			continue;
		$index[] = array('', $file, $nice, firstsentence($xml->fileheader->filedesc));
		foreach ($xml->function as $function)
			$index[] = array((string)$function['name'], $file, $nice, firstsentence($function->explain[0]));
	}
	return 'window.POL_DOCS_INDEX = '
		. json_encode($index, JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE | JSON_HEX_TAG)
		. ";\n";
}

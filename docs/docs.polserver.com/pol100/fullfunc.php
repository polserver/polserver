<?php
	require_once 'include/global.inc';

	$xmlfile = httpget('xmlfile');

	// Only the modules listed in modules.xml, never a path or URL
	$modules = array();
	foreach (simplexml_load_file('modules.xml') as $em)
		$modules[] = (string)$em['name'];

	siteheader('POL Scripting Reference');
	if (in_array($xmlfile, $modules, true) && file_exists($xmlfile.'.xml'))
		xlstdocument('escript.xslt', $xmlfile.'.xml', array('xmlfile' => $xmlfile, 'classes' => objrefclasses()));
	else
		echo "<p style='text-align: center; color: #F00; padding:2em;'>Error: unknown module name.</p>";

	sitefooter();

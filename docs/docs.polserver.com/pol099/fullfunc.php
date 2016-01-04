<?php
	require_once 'include/global.inc';
	
	// This is likely needed in the other PHP scripts and could become a function get_request_var(var_name, default_value)
	$defaultxml = 'attributesem';	
	if ($official) {
		$xmlfile = request_var('xmlfile', $defaultxml);
	}
	else {
		if (isset($_GET['xmlfile']))
			$xmlfile = $_GET['xmlfile'];
		else
			$xmlfile = $defaultxml;
	}
	
	siteheader('POL Scripting Reference');
	if ($xmlfile && file_exists($xmlfile.'.xml')) // A better error processing here wouldn't hurt
		xlstdocument('escript.xslt', $xmlfile.'.xml');
	else
		echo "<p style='text-align: center; color: #F00; padding:2em;'>Error: unknown module name.</p>";

	sitefooter();

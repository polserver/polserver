<?php
	require_once 'include/global.inc';

	$defaultxml = httpget('xmlfile');

	siteheader('POL Scripting Reference');
	if ($xmlfile && file_exists($xmlfile.'.xml')) // A better error processing here wouldn't hurt
		xlstdocument('escript.xslt', $xmlfile.'.xml');
	else
		echo "<p style='text-align: center; color: #F00; padding:2em;'>Error: unknown module name.</p>";

	sitefooter();

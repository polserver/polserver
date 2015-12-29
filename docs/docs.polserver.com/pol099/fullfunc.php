<?php
	require_once 'include/global.inc';

	siteheader('POL Scripting Reference');
	xlstdocument('escript.xslt', $_GET['xmlfile'].'.xml');
	sitefooter();

<?php
	require_once 'include/global.inc';

	siteheader('Latest Core Changes');
	xlstdocument('corechanges.xslt', 'corechanges.xml');
	sitefooter();

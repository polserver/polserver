<?php
	require_once 'include/global.inc';

	siteheader('Character Privileges');
	xlstdocument('privileges.xslt', 'privileges.xml');
	sitefooter();

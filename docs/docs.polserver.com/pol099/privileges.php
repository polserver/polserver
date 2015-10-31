<?php
	require 'include/global.inc';

	siteheader('Character Privileges');
	xlstdocument('privileges.xslt', 'privileges.xml');
	sitefooter();

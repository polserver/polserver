<?php
	require 'include/global.inc';

	siteheader('Latest Core Changes');
	xlstdocument('corechanges.xslt', 'corechanges.xml');
	sitefooter();

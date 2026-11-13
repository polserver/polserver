<?php
	// The search index for docs.js, generated from the module XML files.
	// (The offline docs get the same content as searchindex.js from buildoffline.php.)
	define('OFFICIAL_HEADER_SKIP', true); // no page header is needed here
	require_once 'include/global.inc';

	header('Content-Type: application/javascript; charset=utf-8');
	header('Cache-Control: public, max-age=3600');
	echo searchindexscript();

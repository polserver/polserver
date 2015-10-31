<?php
    include_once 'include/global.inc';
    if (!$offline) {
      $g_Page = "poldoc";
      if( isset($header) && $header )
        include_once $header;
    }
    
    // PHP-BB global stuff
	global $request;
	if( isset($request) )
		$request->enable_super_globals();
	//
    

  	$funcname =  $_GET['funcname'];
  	$xmlfile = $_GET['xmlfile'];

    /* add the header */
    if( $offline || ! isset($header) ) {
      siteheader($funcname.' Reference');
    }
    $xsltproc = new XsltProcessor();
    $xsl = new DomDocument;
    $xsl->load('singlefunc.xslt');
    $xsltproc->importStylesheet($xsl);
    $xml_doc = new DomDocument;
    $xml_doc->load($xmlfile);
    $xsltproc->setParameter('', 'funcname', $funcname);
    $xsltproc->setParameter('', 'xmlfile', $xmlfile);
    
    if ($html = $xsltproc->transformToXML($xml_doc)) {
       echo $html;
    } 

    /* add the footer */
    sitefooter();

<?php
	require 'include/global.inc';
	$funcname =  $_GET['funcname'];
	$xmlfile = $_GET['xmlfile'];
	siteheader($funcname.' Reference');
/*
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
*/
xlstdocument('singlefunc.xslt', $xmlfile);
	sitefooter();

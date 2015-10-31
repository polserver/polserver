<?php
  include_once( "include/global.inc" );
  if (!$offline) {
    $g_Page = "home";
    if( isset($header) && $header )
      include_once $header;
  }
  /* add the header */
  if( $offline || ! isset($header) ) {
    siteheader('Latest Core Changes');
  }
  
  $xsltproc = new XsltProcessor();
  $xsl = new DomDocument;
  $xsl->load('corechanges.xslt');
  $xsltproc->importStylesheet($xsl);
  $xml_doc = new DomDocument;
  $xml_doc->load('corechanges.xml');
  $xsltproc->setParameter('', 'offline', $offline);
  
  if ($html = $xsltproc->transformToXML($xml_doc)) {
     echo $html;
  } 
  
  /* add the footer */
  sitefooter();

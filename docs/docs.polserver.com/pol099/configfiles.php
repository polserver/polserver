<?php
  include_once( "include/global.inc" );
  if (!$offline) {
    $g_Page = "home";
    if( isset($header) && $header )
        include_once $header;
  }
  
  /* add the header */
  if ($offline) {
    siteheader('Config File Reference');
  }
  
  $xsltproc = new XsltProcessor();
  $xsl = new DomDocument;
  $xsl->load('configfiles.xslt');
  $xsltproc->importStylesheet($xsl);
  $xml_doc = new DomDocument;
  $xml_doc->load('configfiles.xml');
  $xsltproc->setParameter('', 'offline', $offline);
  
  if ($html = $xsltproc->transformToXML($xml_doc)) {
     echo $html;
  } 
  
  if (!$offline) {
    echo ('
    <script src="http://www.google-analytics.com/urchin.js" type="text/javascript">
    </script>
    <script type="text/javascript">
    _uacct = "UA-2869696-3";
    urchinTracker();
    </script>');
  }
  
  /* add the footer */
  sitefooter($offline);
?>


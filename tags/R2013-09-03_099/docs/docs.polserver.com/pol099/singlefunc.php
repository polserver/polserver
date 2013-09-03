<?php
    include_once 'include/global.inc';
    if (!$offline) {
      $g_Page = "poldoc";
      include_once '/home/polteam/include/_header.php';
    }
    

  	$funcname =  $_GET['funcname'];
  	$xmlfile = $_GET['xmlfile'];

    /* add the header */
    if ($offline) {
      siteheader($funcname.' Reference');
    }
    $xsltproc = new XsltProcessor();
    $xsl = new DomDocument;
    $xsl->load($webroot.'singlefunc.xslt');
    $xsltproc->importStylesheet($xsl);
    $xml_doc = new DomDocument;
    $xml_doc->load($webroot.$xmlfile);
    $xsltproc->setParameter('', 'funcname', $funcname);
    $xsltproc->setParameter('', 'xmlfile', $xmlfile);
    
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


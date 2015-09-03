<?php
    $g_Page = "home";
    include( "include/global.inc" );
    //include_once ("include/archive.inc");
    if( isset($header) && $header )
      include_once $header;

    /* add the header */
    mainpageheader();

    $xsltproc = new XsltProcessor();
    $xsl = new DomDocument;
    $xsl->load($webroot.'front_em.xslt');
    $xsltproc->importStylesheet($xsl);
    $xml_doc = new DomDocument;
    $xml_doc->load($webroot.'modules.xml');
    $xsltproc->setParameter('', 'offline', $offline);
    
    if ($html = $xsltproc->transformToXML($xml_doc)) {
       echo $html;
    }
?>

<div id="main">
	<div class="container">
	  <div class="doc-col_1">
    <div class="doc-mainbox-small">
      <center><h2>POL Core Documentation:</h2></center>
      <ul>
      <li><a href="objref.php">POL Object Class Reference</a>
      <li><a href="configfiles.php">Configuration Files</a>
      <li><a href="scripttypes.php">Script Types</a>
      <li><a href="events.php">System Events</a>
      <li><a href="builtintextcmds.php">Built-In Text Commands</a>
      <li><a href="privileges.php">Character Privileges</a>
      <li><a href="attack.php">Combat Pseudocode</a>
      </ul>
    </div>
  </div>
  <div class="doc-col_12">
    <div class="doc-mainbox-small">
      <center><h2>Tutorials and Guides:</h2></center>
      <ul>
      <li><a href="corechanges.php">Latest Core Changes</a>
      <li><a href="guides.php?guidefile=escriptguide">Racalac's eScript Guide</a>
      <li><a href="guides.php?guidefile=performance">ToGu's eScript Performance Guide</a>
      <li><a href="guides.php?guidefile=gumps">Lystramon's Gump Tutorial</a>
      <li><a href="guides.php?guidefile=gumpcmdlist">Turley's Gump-Command-List</a>
      </ul>
    </div>
  </div>
  </div>
</div>
	<script src="http://www.google-analytics.com/urchin.js" type="text/javascript">
	</script>
	<script type="text/javascript">
		_uacct = "UA-2869696-3";
		urchinTracker();
	</script>
<?php
   
   /* add the footer */
   sitefooter($offline);
?>



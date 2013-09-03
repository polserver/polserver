<?php
function ob_file_callback($buffer)
{
  global $ob_file;
  fwrite($ob_file,$buffer);
}

    include_once("/home/polteam/include/CreateZipFile.inc");
    include( "include/global.inc" );
    
    $offline = 1;
    if (!is_dir('offline')) {
      mkdir('offline');
    }
    else { /* cleanup */
      if($dh = opendir('offline')) {
        while (($file = readdir($dh)) !== false) {
          if(is_file('offline/'. $file)) {
             unlink('offline/'. $file);
          }
        }
      }
      closedir($dh);
    }
    
    $ob_file = fopen('offline/index.html','w');

    ob_start('ob_file_callback');

    /* add the header */
    siteheader('POL Scripting Reference');
    mainpageheader($offline);
    
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
      <li><a href="objref.html">POL Object Class Reference</a>
      <li><a href="configfiles.html">Configuration Files</a>
      <li><a href="scripttypes.html">Script Types</a>
      <li><a href="events.html">System Events</a>
      <li><a href="builtintextcmds.html">Built-In Text Commands</a>
      <li><a href="privileges.html">Character Privileges</a>
      <li><a href="attack.html">Combat Pseudocode</a>
      </ul>
  </div>
  </div>
  <div class="doc-col_12">
    <div class="doc-mainbox-small">
      <center><h2>Tutorials and Guides:</h2></center>
        <ul>
        <li><a href="corechanges.html">Latest Core Changes</a>  
        <li><a href="escriptguide.html">Racalac's eScript Guide</a>
        <li><a href="performance.html">ToGu's eScript Performance Guide</a>
        <li><a href="gumps.html">Lystramon's Gump Tutorial</a>
        <li><a href="gumpcmdlist.html">Turley's Gump-Command-List</a>
        </ul>
      </div>
    </div>
  </div>
</div>
<?php
   /* add the footer */
   sitefooter($offline);
   ob_end_flush();
   
   /* generate Core Documentation */
   $files = array("objref","configfiles","scripttypes","events","builtintextcmds","privileges","attack");
   foreach ($files as $f)
   {
     $ob_file = fopen('offline/'.$f.'.html','w');
   
     ob_start('ob_file_callback');
     include $f.'.php';
     ob_end_flush();
   }
   
   /* generate em Modules */
   $xsltproc = new XsltProcessor();
   $xsl = new DomDocument;
   $xsl->load($webroot.'/escript.xslt');
   $xsltproc->importStylesheet($xsl);
   $xsltproc->setParameter('', 'offline', $offline);
   
   $xml = simplexml_load_file($webroot.'/modules.xml');
   foreach ($xml as $em)
   {
      $name=(string)$em['name'];
      $nicename=(string)$em['nice'];
      $ob_file = fopen('offline/'.$name.'.html','w');
      ob_start('ob_file_callback');
      siteheader('POL Scripting Reference '.$nicename.'.em');
      $xml_doc = new DomDocument;
      $xml_doc->load($webroot.'/'.$name.'.xml');
     
	    if ($html = $xsltproc->transformToXML($xml_doc)) {
        echo $html;
      }
      sitefooter();
      ob_end_flush();
   }
   
   /* generate Guides */
   $files = array("escriptguide","performance","gumps","gumpcmdlist");
   $type ="";
   global $type;
   foreach ($files as $f)
   {
     $type=$f;
     $ob_file = fopen('offline/'.$f.'.html','w');
   
     ob_start('ob_file_callback');
     include 'guides.php';
     ob_end_flush();
   }
   $ob_file = fopen('offline/corechanges.html','w');
   ob_start('ob_file_callback');
   include 'corechanges.php';
   ob_end_flush();

   
   /* copy files */
   $styledir ='/home/.orlo/polteam/polserver.com/';
   copy($styledir.'style.css','offline/style.css');
   copy('picture_gump_finish_1.jpg','offline/picture_gump_finish_1.jpg');
   if (!is_dir('offline/images')) {
     mkdir('offline/images');
   }
   else { /* cleanup */
     if($dh = opendir('offline/images')) {
       while (($file = readdir($dh)) !== false) {
         if(is_file('offline/images/'. $file)) {
            unlink('offline/images/'. $file);
         }
       }
       closedir($dh);
     }
   }
   copy($styledir.'images/miniheader_bg.jpg','offline/images/miniheader_bg.jpg');
   copy($styledir.'images/sectionheader_bg.jpg','offline/images/sectionheader_bg.jpg');
   copy($styledir.'images/sectionheader_gold_bg.jpg','offline/images/sectionheader_gold_bg.jpg');
   copy($styledir.'images/sectionheader_small_bg.jpg','offline/images/sectionheader_small_bg.jpg');
   copy($styledir.'images/sectionheader_small_gold_bg.jpg','offline/images/sectionheader_small_gold_bg.jpg');
   
   /* Save archivetime */
   //date_default_timezone_set('America/Los_Angeles');
   $ob_file = fopen('include/archive.inc','w');
   ob_start('ob_file_callback');
   $d=date('Y-m-d');
   echo ("<?php\n");
   echo ('$archivetime="'.$d.'";');   
   echo ("\n?>");
   ob_end_flush();

   /* Zip the offline doc */   
   if (!is_dir('archives')) {
     mkdir('archives');
     copy('include/index.html','archives/index.html');
   }
   $zip = new createZip;
   $zip -> addDirectory("images/");
   
   if ($dh = opendir('offline')) {
     while (($file = readdir($dh)) !== false) {
       if(is_file('offline/'. $file)) {
         $fileContents = file_get_contents('offline/'.$file);  
         $zip -> addFile($fileContents, $file);  
       }
     }
     closedir($dh);
   }
   if ($dh = opendir('offline/images')) {
     while (($file = readdir($dh)) !== false) {
       if(is_file('offline/images/'. $file)) {
         $fileContents = file_get_contents('offline/images/'.$file);  
         $zip -> addFile($fileContents, 'images/'.$file);  
       }
     }
     closedir($dh);
   }
   $fileName = 'archives/pol-docs-099-'.$d.'.zip';
   $fd = fopen ($fileName, "wb");
   $out = fwrite ($fd, $zip -> getZippedfile());
   fclose ($fd);

   echo "Finished";
?>



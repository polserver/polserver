<?php
	require_once 'include/global.inc';
	$offline = 1;

	if (!is_dir('offline')) {
		mkdir('offline');
	} else { /* cleanup */
		if($dh = opendir('offline')) {
			while (($file = readdir($dh)) !== false) {
				if(is_file('offline/'. $file)) {
					unlink('offline/'. $file);
				}
			}
		}
		closedir($dh);
	}

	// This callback will take care of writing the offline doc file when the
	// output buffer is flushed
	function ob_file_callback($buffer)
	{
		global $ob_file;
		fwrite($ob_file, $buffer);
	}

	// generate Core Documentation
	$files = array('index',"objref","configfiles","scripttypes","events","builtintextcmds","privileges","attack");
	foreach ($files as $f)
	{
		$ob_file = fopen('offline/'.$f.'.html','w');

		ob_start('ob_file_callback');
		require $f.'.php';
		ob_end_flush();
	}

   /* generate em Modules */
   $xsltproc = new XsltProcessor();
   $xsl = new DomDocument;
   $xsl->load('escript.xslt');
   $xsltproc->importStylesheet($xsl);
   $xsltproc->setParameter('', 'offline', $offline);

   $xml = simplexml_load_file('modules.xml');
   foreach ($xml as $em)
   {
      $name=(string)$em['name'];
      $nicename=(string)$em['nice'];
      $ob_file = fopen('offline/'.$name.'.html','w');
      ob_start('ob_file_callback');
      siteheader('POL Scripting Reference '.$nicename.'.em');
      $xml_doc = new DomDocument;
      $xml_doc->load($name.'.xml');

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
   copy('style.css','offline/style.css');
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
   copy('images/miniheader_bg.jpg','offline/images/miniheader_bg.jpg');
   copy('images/sectionheader_bg.jpg','offline/images/sectionheader_bg.jpg');
   copy('images/sectionheader_gold_bg.jpg','offline/images/sectionheader_gold_bg.jpg');
   copy('images/sectionheader_small_bg.jpg','offline/images/sectionheader_small_bg.jpg');
   copy('images/sectionheader_small_gold_bg.jpg','offline/images/sectionheader_small_gold_bg.jpg');

   /* Save archivetime */
   date_default_timezone_set('America/Los_Angeles');
   $d=date('Y-m-d');

   /* Zip the offline doc */   
   if (!is_dir('archives')) {
     mkdir('archives');
     copy('include/index.html','archives/index.html');
   }

   class POLZip extends ZipArchive {
     public function addDirRecursive($dirname, $base=null) {
       if( $base )
         if( ! $this->addEmptyDir($base) )
           throw new Exception("ERROR while adding \"$base\" to zip archive");

       foreach( scandir($dirname) as $file ) {
         $relpath = $base ? "$base/$file" : $file;

         if( $file == '.' || $file == '..' )
           continue;
         elseif( is_file("$dirname/$file") ) {
           echo "Compressing $file...\n";
           if( ! $this->addFile("$dirname/$file", $relpath) )
             throw new Exception("ERROR while adding \"$dirname/$file\" to zip archive");
         } elseif( is_dir($file) )
           $this->addDirRecursive($file, $relpath);
       }
     }
   }

   $zip = new POLZip();
   $zip->open("archives/pol-docs-099-$d.zip", ZIPARCHIVE::CREATE | ZIPARCHIVE::OVERWRITE);
   $zip->addDirRecursive('offline');
   $zip->close();

   echo "Finished\n";

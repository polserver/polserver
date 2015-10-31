<?php
    include_once( "include/global.inc" );
    $guidefile="";
    if (!$offline) {
      $g_Page = "home";
      if( isset($header) && $header )
        include_once $header;

      // PHP-BB global stuff
			global $request;
			$request->enable_super_globals();
			//
	
      $guidefile = $_GET['guidefile'];
    }
    else {
     $guidefile=$type;
    }

    /* add the header */
    if( $offline || ! isset($header) ) {
      if ($guidefile=='escriptguide') {
        siteheader("Racalac's EScript Guide");
      }
      else if ($guidefile=='performance') {
        siteheader("POL Performance Guide");
      }
      else if ($guidefile=='gumps') {
        siteheader("Lystramon's Gump Tutorial");
      }
      else if ($guidefile=='gumpcmdlist') {
        siteheader("Turley's Gump-Command-List");
      }
    }
    if ($offline) {
      echo('<div id="main">
      <div class="container">');
    }
echo('
<div class="doc-mainbox">');
if ($offline) {
echo("<div class='doc-home'><a href='index.html'>Home</a></div>");
}
else {
echo("<div class='doc-home'><a href='index.php'>Home</a></div>");
}
echo('<br/>');
    
    include('include/'.$guidefile.'.inc');

echo('
</div>');
if ($offline) {
      echo('</div></div>');
    }

   /* add the footer */
   sitefooter();

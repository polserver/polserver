<?php
    include_once( "include/global.inc" );
    $guidefile="";
    if (!$offline) {
      $g_Page = "home";
      include_once '/home/polteam/include/_header.php';
      $guidefile = $_GET['guidefile'];
    }
    else {
     $guidefile=$type;
    }

    /* add the header */
    if ($offline) {
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
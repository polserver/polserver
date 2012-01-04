<?php

$webroot = '/home/polteam/docs.polserver.com/pol099/';
$offline = 0;
set_error_handler('XML_errorHandler');
date_default_timezone_set(@date_default_timezone_get());


function XML_errorHandler ($errno, $errstr, $errfile, $errline,$errcontext)
{
  global $offline;
  if ($offline) {
    ob_end_clean();
  }
  die(sprintf("%s %s %s %s", $errstr, $errfile, $errline,$errcontext));
}

function siteheader($title)
{
?>
<!DOCTYPE html PUBLIC "-//W3C//XHTML 1.0 Transitional" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>
	<head>
		<title><?php echo $title ?></title>
		<meta name="keyword" content="POL, EScript, documentation, Ultima Online, UO, packets, client, ultima online client, emulator" />
		<meta name="description" content="Penultima Online Documentation website." />
		<meta name="Author" content="Racalac, Rykard" />
		<meta name="Generator" content="POL Doc-o-matic" />
		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
		<meta http-equiv="Content-Style-Type" content="text/css" />
		<link href="style.css" rel="stylesheet" type="text/css"  media="all"  />
	</head>
	<script language="JavaScript1.2" type="text/javascript">
function ExpandContract(s) {
  var bulletE = document.getElementById("bullet_"+s);
  var spanE = document.getElementById("span_"+s);
  var Expanded = bulletE.innerHTML;

  if(Expanded == '[+]') {
    spanE.style.display = 'block';
    bulletE.innerHTML = '[-]';
  }
  else { 
    spanE.style.display = 'none'; 
    bulletE.innerHTML = '[+]';
  }
}
function initMenus() {
	if (!document.getElementsByTagName) return;
	
	var aMenus = document.getElementsByTagName("LI");
	for (var i = 0; i < aMenus.length; i++) {
		var mclass = aMenus[i].className;
		if (mclass.indexOf("treenode") > -1) {
			var submenu = aMenus[i].childNodes;
			for (var j = 0; j < submenu.length; j++) {
				if (submenu[j].tagName == "A") {
				  if (submenu[j].innerHTML == "[+] ") {
					
					submenu[j].onclick = function() {
						var node = this.nextSibling;
											
						while (1) {
							if (node != null) {
								if (node.tagName == "UL") {
									var d = (node.style.display == "none")
									node.style.display = (d) ? "block" : "none";
									this.className = (d) ? "treeopen" : "treeclosed";
									this.innerHTML = (d) ? "[-] " : "[+] ";
									return false;
								}
								node = node.nextSibling;
							} else {
								return false;
							}
						}
						return false;
					}
					
					submenu[j].className = (mclass.indexOf("open") > -1) ? "treeopen" : "treeclosed";
					submenu[j].innerHTML = (mclass.indexOf("open") > -1) ? "[-] " : "[+] ";
				}
				}
				
				if (submenu[j].tagName == "UL")
					submenu[j].style.display = (mclass.indexOf("open") > -1) ? "block" : "none";
			}
		}
	}
}

window.onload = initMenus;
</script>
<body>

<?php
}

function mainpageheader($offline=0)
{
   echo('
    <div class="container">
      <div id="doc-mini-header">
    <h1>
      POL Documentation Site
    </h1>
    <p>
    Note everything on these pages is considered to be IN PROGRESS. Data may and will change, and not all data has been verified (though the
         best effort was made to produce correct documentation). If you are viewing the downloadable local version of these docs, note the online
	      version may contain more up-to-date information.<p>
        I hope you find these documents useful!');
    
    if (!$offline)
    {
    global $archivetime;
      echo('
    <hr>
    Downloadable Version (099): <a href="http://docs.polserver.com/pol099/archives/pol-docs-099-'.$archivetime.'.zip">099 Offline Documentation ('.$archivetime.')</a>
    <hr>');
    }
  else
    {
    echo('
    <hr>
    The Official POL Documentaion site is found at <a href="http://docs.polserver.com">http://docs.polserver.com</a>
    <hr>');
    }
    echo('
    </div>
    </div>
    ');

}

/* web page footer function */
function sitefooter($offline=0)
{
   if (!$offline) {
     echo ('</div></div>');
   }
   echo ('
<div id="footer">
	<div class="container">
    <p >
      If you <i>know</i> if any information is incorrect on these pages, mail your corrections to polteam@polserver.com
    </p>
    <p >
      Copyright &copy;2003-2011 David Carpman and POL Development Team, all rights reserved. DO NOT REPRODUCE, MIRROR, ALTER, SPINDLE, MUTILATE, OR SIT ON.
    </p>
    </div>
    </div>
    </body>
    </html>
   ');
}

?>

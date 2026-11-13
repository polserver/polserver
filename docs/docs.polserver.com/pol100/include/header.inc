<?php
	// Header for the offline docs, and for viewing these pages anywhere but on the official
	// site (there, the official site header is used instead - see global.inc).
	// Like the official header, it leaves <div id="main"><div class="container"> open;
	// footer.inc closes them.
	global $offline;
	$ext = $offline ? 'html' : 'php';
	// The stylesheet is the one of www.polserver.com: buildoffline.php copies it next to
	// the offline pages; everywhere else it is loaded from the site.
	$stylesheet = $offline ? 'style.css' : 'https://www.polserver.com/style.css';
?>
<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="utf-8" />
	<meta name="viewport" content="width=device-width, initial-scale=1" />
	<title><?php echo htmlspecialchars($title) ?></title>
	<meta name="keyword" content="POL, EScript, documentation, Ultima Online, UO, packets, client, ultima online client, emulator" />
	<meta name="description" content="Penultima Online Documentation website." />
	<meta name="Author" content="Racalac, Rykard" />
	<meta name="Generator" content="POL Doc-o-matic" />
	<link href="<?php echo $stylesheet ?>" rel="stylesheet" type="text/css" media="all" />

	<script type="text/javascript">
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
	</script>
</head>
<body class="docs">

<div id="mini-header">
	<div class="container">
		<div id="mini-nav">
			<nav class="subnav" aria-label="Documentation sections">
				<a href="index.<?=$ext?>">Modules</a>
				<a href="objref.<?=$ext?>">Object Reference</a>
				<a href="configfiles.<?=$ext?>">Config Files</a>
				<a href="scripttypes.<?=$ext?>">Script Types</a>
				<a href="events.<?=$ext?>">Events</a>
				<a href="corechanges.<?=$ext?>">Core Changes</a>
				<a href="index.<?=$ext?>#guides">Guides</a>
			</nav>
		</div>
		<div id="search-box">
			<form action="index.<?=$ext?>" method="get">
				<input type="search" name="q" id="q" placeholder="Search the docs" aria-label="Search the docs" />
			</form>
		</div>
	</div>
</div>

<div id="main">
	<div class="container">

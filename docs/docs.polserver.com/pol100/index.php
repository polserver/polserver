<?php
	require_once 'include/global.inc';

	siteheader('POL Documentation Site');
	if (!$offline && $official) {
		$files = scandir('archives', SCANDIR_SORT_DESCENDING);
		foreach ($files as $file) {
			if (substr($file,0,strlen("pol-docs"))=="pol-docs") {
				$archivefile = $file;
				break;
			} 
		}
		$parts = explode("-",pathinfo($archivefile)["filename"],4);
		$archivetime = end($parts);
	}

?>
	<div class="container">
		<div id="doc-mini-header">
			<h1>
				POL Documentation Site
			</h1>
			<p>
				Note everything on these pages is considered to be IN PROGRESS.
				Data may and will change, and not all data has been verified (though the
				best effort was made to produce correct documentation).
				If you are viewing the downloadable local version of these docs,
				note the online
				version may contain more up-to-date information.
			<p>
			I hope you find these documents useful!
<?php if( ! $offline && $official ) { ?>
				<hr>
				Downloadable Version (100):
				<a href="https://docs.polserver.com/pol100/archives/<?=$archivefile?>">
					100 Offline Documentation <?=$archivetime?>
				</a>
				<hr>
<?php } else { ?>
				<hr>
				The Official POL Documentaion site is found at
				<a href="https://docs.polserver.com">https://docs.polserver.com</a>
				<hr>
<?php } ?>
		</div>
	</div>

<?php
	xlstdocument('front_em.xslt', 'modules.xml');

	$ext = $offline ? 'html' : 'php';
	function guideUrl($name) {
		global $offline;
		return $offline ? "$name.html" : "guides.php?guidefile=$name";
	}
?>

	<div id="main">
		<div class="container">
			<div class="doc-col_1">
				<div class="doc-mainbox-small">
					<center><h2>POL Core Documentation:</h2></center>
					<ul>
					<li><a href="objref.<?=$ext?>">POL Object Class Reference</a>
					<li><a href="configfiles.<?=$ext?>">Configuration Files</a>
					<li><a href="scripttypes.<?=$ext?>">Script Types</a>
					<li><a href="events.<?=$ext?>">System Events</a>
					<li><a href="builtintextcmds.<?=$ext?>">Built-In Text Commands</a>
					<li><a href="privileges.<?=$ext?>">Character Privileges</a>
					<li><a href="attack.<?=$ext?>">Combat Pseudocode</a>
					</ul>
				</div>
			</div>
		<div class="doc-col_12">
			<div class="doc-mainbox-small">
				<center><h2>Tutorials and Guides:</h2></center>
				<ul>
				<li><a href="corechanges.<?=$ext?>">Latest Core Changes</a>
				<li><a href="<?=guideUrl('escriptguide')?>">Racalac's eScript Guide</a>
				<li><a href="<?=guideUrl('performance')?>">ToGu's eScript Performance Guide</a>
				<li><a href="<?=guideUrl('gumps')?>">Lystramon's Gump Tutorial</a>
				<li><a href="<?=guideUrl('gumpcmdlist')?>">Turley's Gump-Command-List</a>
				</ul>
			</div>
		</div>
	</div>

<?php
   sitefooter();

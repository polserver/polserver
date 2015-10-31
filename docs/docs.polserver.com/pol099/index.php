<?php
	require_once 'include/global.inc';

	siteheader('POL Documentation Site');

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
				Downloadable Version (099):
				<a href="http://docs.polserver.com/pol099/archives/pol-docs-099-<?=$archivetime?>.zip">
					099 Offline Documentation <?=$archivetime?>
				</a>
				<hr>
<?php } else { ?>
				<hr>
				The Official POL Documentaion site is found at
				<a href="http://docs.polserver.com">http://docs.polserver.com</a>
				<hr>
<?php } ?>
		</div>
	</div>

<?php
	xlstdocument('front_em.xslt', 'modules.xml');
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

<?php
   sitefooter();

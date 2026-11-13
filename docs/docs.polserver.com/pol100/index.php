<?php
	require_once 'include/global.inc';

	siteheader('POL Documentation Site');
	$archivefile = '';
	if (!$offline && $official) {
		$files = is_dir('archives') ? (scandir('archives', SCANDIR_SORT_DESCENDING) ?: array()) : array();
		foreach ($files as $file) {
			if (substr($file,0,strlen("pol-docs"))=="pol-docs") {
				$archivefile = $file;
				break;
			}
		}
		if ($archivefile !== '') {
			$parts = explode("-",pathinfo($archivefile)["filename"],4);
			$archivetime = end($parts);
		}
	}

	$ext = $offline ? 'html' : 'php';
	function guideUrl($name) {
		global $offline;
		return $offline ? "$name.html" : "guides.php?guidefile=$name";
	}
?>
	<div class="doc-head">
		<h1>POL Documentation Site</h1>
		<p>
			Note everything on these pages is considered to be IN PROGRESS.
			Data may and will change, and not all data has been verified (though the
			best effort was made to produce correct documentation).
			If you are viewing the downloadable local version of these docs,
			note the online version may contain more up-to-date information.
		</p>
		<div class="meta">
<?php if( ! $offline && $official ) { ?>
<?php if( $archivefile !== '' ) { ?>
			Downloadable version (100):
			<a href="https://docs.polserver.com/pol100/archives/<?=$archivefile?>">100 Offline Documentation <?=$archivetime?></a>
<?php } ?>
<?php } else { ?>
			The official POL documentation site is found at
			<a href="https://docs.polserver.com">https://docs.polserver.com</a>
<?php } ?>
		</div>
	</div>

	<div class="mainbox">
		<h1>POL Core Documentation</h1>
		<div class="tiles">
			<a class="tile" href="objref.<?=$ext?>"><b>Object Class Reference</b><span>Members and methods of every object type</span></a>
			<a class="tile" href="configfiles.<?=$ext?>"><b>Configuration Files</b><span>Every .cfg file and its settings</span></a>
			<a class="tile" href="scripttypes.<?=$ext?>"><b>Script Types</b><span>The scripts the core runs, and when</span></a>
			<a class="tile" href="events.<?=$ext?>"><b>System Events</b><span>Events the core sends to scripts</span></a>
			<a class="tile" href="builtintextcmds.<?=$ext?>"><b>Built-In Text Commands</b><span>Commands handled by the core itself</span></a>
			<a class="tile" href="privileges.<?=$ext?>"><b>Character Privileges</b><span>Privilege names and what they allow</span></a>
			<a class="tile" href="attack.<?=$ext?>"><b>Combat Pseudocode</b><span>How the core resolves an attack</span></a>
			<a class="tile" href="corechanges.<?=$ext?>"><b>Latest Core Changes</b><span>What changed in recent core versions</span></a>
		</div>
	</div>

	<div class="mainbox" id="guides">
		<h1>Tutorials and Guides</h1>
		<div class="guides">
			<div class="guide-group">
				<h2>eScript</h2>
				<a class="tile" href="<?=guideUrl('escriptguide')?>"><b>eScript Guide</b><span>Learning the language from the start</span><i>by Racalac</i></a>
				<a class="tile" href="<?=guideUrl('performance')?>"><b>eScript Performance Guide</b><span>Writing faster scripts</span><i>by ToGu</i></a>
			</div>
			<div class="guide-group">
				<h2>Gumps</h2>
				<a class="tile" href="<?=guideUrl('gumps')?>"><b>Gump Tutorial</b><span>Building a gump step by step</span><i>by Lystramon</i></a>
				<a class="tile" href="<?=guideUrl('gumpcmdlist')?>"><b>Gump Command List</b><span>Every gump command in one place</span><i>by Turley</i></a>
			</div>
		</div>
	</div>

	<div class="mainbox-gold">
		<h1>eScript Modules</h1>
		<div class="mod-tools">
			<input type="search" id="mod-filter" placeholder="Find a function in any module" aria-label="Find a function in any module" autocomplete="off">
			<button type="button" id="mod-toggle">Collapse all</button>
			<span class="status" id="mod-status" aria-live="polite"></span>
		</div>
<?php
	xlstdocument('front_em.xslt', 'modules.xml');
?>
	</div>

<?php
   sitefooter();

<?php
	require_once 'include/global.inc';

	$guidefile = $offline ? $type : httpget('guidefile');

	switch( $guidefile ) {
	case 'escriptguide':
		$title = 'Racalac\'s EScript Guide';
		break;
	case 'performance':
		$title = 'POL Performance Guide';
		break;
	case 'gumps':
		$title = 'Lystramon\'s Gump Tutorial';
		break;
	case 'gumpcmdlist':
		$title = 'Turley\'s Gump-Command-List';
		break;
	default: // $guidefile is used in a require below: only the guides above
		http_response_code(404);
		exit('Unknown guide.');
	}
	siteheader($title);

	// A guide marks its table of contents with <!-- toc --> ... <!-- /toc -->; it is
	// shown beside the text (and as a folding menu on narrow screens).
	ob_start();
	require 'include/'.$guidefile.'.inc';
	$guide = ob_get_clean();
	$toc = '';
	$tocstart = strpos($guide, '<!-- toc -->');
	$tocend = strpos($guide, '<!-- /toc -->');
	if ($tocstart !== false && $tocend !== false && $tocend > $tocstart) {
		$toc = substr($guide, $tocstart, $tocend - $tocstart);
		$guide = substr($guide, 0, $tocstart) . substr($guide, $tocend + strlen('<!-- /toc -->'));
	}
?>

	<div class="doc-crumbs">
		<a href="index.<?=$offline ? 'html' : 'php'?>">Documentation</a> &#8250;
		<a href="index.<?=$offline ? 'html' : 'php'?>#guides">Guides</a> &#8250;
		<?=htmlspecialchars($title)?>
	</div>

<?php if ($toc !== '') { ?>
	<div class="doc-layout doc-guide-layout">
		<div class="doc-side">
			<details class="smallbox doc-menu" open="open">
				<summary><h1>Contents</h1></summary>
				<?=$toc?>
			</details>
		</div>
		<div class="doc-main">
			<div class="doc-mainbox doc-guide">
				<?=$guide?>
			</div>
		</div>
	</div>
<?php } else { ?>
	<div class="doc-mainbox doc-guide">
		<?=$guide?>
	</div>
<?php } ?>

<?php
	sitefooter();

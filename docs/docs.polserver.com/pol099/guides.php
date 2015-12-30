<?php
	require_once 'include/global.inc';

	$guidefile = $offline ? $type : ($official?request_var('guidefile',''):$_GET['guidefile']);

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
	}
	siteheader($title);

	if( $offline ) {
?>
	<div id="main"><div class="container">
<?php } ?>

	<div class="doc-mainbox">

<?php if( $offline ) { ?>
		<div class='doc-home'><a href='index.html'>Home</a></div>
<?php } else { ?>
		<div class='doc-home'><a href='index.php'>Home</a></div>
<?php } ?>
			<br/>
			<?php require 'include/'.$guidefile.'.inc'; ?>
		</div>

<?php if( $offline ) { ?>
	</div></div>
<?php
	}

	sitefooter();

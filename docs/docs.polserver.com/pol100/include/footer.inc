<?php global $offline; ?>
	</div>
</div>

<div id="footer">
	<div class="container">
		<p>
			If you <i>know</i> if any information is incorrect on these pages, mail your corrections to polteam@polserver.com
		</p>
		<p>
			Copyright &copy;2003-<?php echo date('Y'); ?> David Carpman and POL Development Team, all rights reserved. DO NOT REPRODUCE, MIRROR, ALTER, SPINDLE, MUTILATE, OR SIT ON.
		</p>
	</div>
</div>

<script src="searchindex.<?php echo $offline ? 'js' : 'php' ?>" defer></script>
<script src="docs.js" defer<?php echo $offline ? ' data-offline="1"' : '' ?>></script>

<?php
	if( isset($analytics) ) {
		echo $analytics;
	}
?>

</body>
</html>

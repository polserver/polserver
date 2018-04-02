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

</head>
<body>

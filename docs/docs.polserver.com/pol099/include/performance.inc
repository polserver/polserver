<h1>POL Performance Guide</h1>
<p>
	This guide assumes you already know how eScript works, so you'll only get an explanation for the profiling commands.<br />
	You should at least know everything in
<?php if ($offline) {echo("<a href='escriptguide.html'>");}else{echo("<a href='guides.php?guidefile=escriptguide'>");}
	?>Racalac's eScript guide</a> before reading this.
</p>
<br/>
<h2>Table of Contents:</h2>
<ul>
	<li><a href="#runaway">Runaway scripts</a></li>
	<li><a href="#polcore">polcore()</a></li>
	<li><a href="#scriptopts">Runtime script options</a></li>
</ul>
<hr />
<br/>
<a name="runaway"></a><h2>Runaway scripts</h2>
<p>
	Apart from "doesn't work", a runaway script is usually the first runtime problem you have to deal with.<br />
	I have made an example text command that goes runaway for high parameters, click <a name="runawaysquareref" href="#runawaysquare">here</a> to view it.<br />
	When computing 10^2 it uses more than 1000 cycles, for 100^2 it's above 100k cycles<br />
	Let's say you typed <tt>.runawaysquare 1000</tt> and get the following in your console:
</p>

<div class="doc-guide-pre">
<pre>
[01/12 21:47:36 script.log] Runaway script[3034]: scripts/textcmd/test/runawaysquare.ecl (160000 cycles)
[01/12 21:47:36 script.log]  25: :=
[01/12 21:47:36 script.log]  26: #
[01/12 21:47:37 script.log]  27: local #4
[01/12 21:47:37 script.log]  28: local #2
[01/12 21:47:37 script.log]  29: <
[01/12 21:47:37 script.log] >30: if false goto 42
[01/12 21:47:37 script.log]  31: local #5
[01/12 21:47:38 script.log]  32: 1L
[01/12 21:47:38 script.log]  33: +
[01/12 21:47:38 script.log]  34: local5 :=
[01/12 21:47:38 script.log]  35: local #4
</pre></div>

<p>
	A very nice way to stop the script is to use the test-level command killpid, you type .killpid process_id, which in this case is 3034. (<tt>.killpid 3034</tt>)<br />
	If you've never done this before, trying it to confirm that the script indeed stops might be a good idea, and looking at the killpid source wouldn't hurt either :)
</p>
<p>
	If your scripts go runaway like this, it means something is wrong. If you have to perform very long loops, you should add some sleeps. Let me repeat, there is never a good reason to let scripts go runaway. Ever. And increasing the runaway treshold is just ignoring the problem, not fixing it ;)
</p>

<p>
	Now let's say the runaway is caused by a bug, but you have no idea where it's looping. Having compiled with the -i really helps here, as you can often immediately see where the problem is in the console window:
</p>
<div class="doc-guide-pre">
<pre>
[01/12 21:56:37 script.log] Runaway script[3037]: scripts/textcmd/test/runawaysquare.ecl (440000 cycles)
[01/12 21:56:37 script.log]  31: :=
[01/12 21:56:37 script.log]  32: #
[01/12 21:56:37 script.log]  33: local #4
[01/12 21:56:37 script.log]  34: local #2
[01/12 21:56:37 script.log]  35: <
[01/12 21:56:37 script.log] >36: if false goto 49
[01/12 21:56:38 script.log]  37: [sum := sum + 1;]
[01/12 21:56:38 script.log]  38: local #5
[01/12 21:56:38 script.log]  39: 1L
[01/12 21:56:38 script.log]  40: +
[01/12 21:56:38 script.log]  41: local5 :=
</pre></div>

<p>
	(This is the same place as last time, but compiled with -i and you see a source code line listed as instruction 37 (as you can see this increases length of a script, so you shouldn't use the -i script in a completed and debugged script!)<br />
	You're not always lucky enough to get a source code line in the runaway dump though, and then you have to find the corresponding code line manually:<br />
	The first thing you do is to generate a source listing (<tt>ecompile -l runawaysquare.src</tt>). The complete listing for runawaysquare.lst file you get is available <a name="runawaysquarelistingref" href="#runawaysquarelisting">here</a>.
</p>

<p>
	I find the following around the "30: if false goto 49" line:
</p>
<div class="doc-guide-pre">
<pre>
for (b := 0; b < i; b := b + 1)
23: local #4
24: 0L
25: :=
26: #
27: local #4
28: local #2
29: <
30: if false goto 42
sum := sum + 1;
31: local #5
32: 1L
33: +
34: local5 :=
35: local #4
36: local #4
37: 1L
38: +
39: :=
40: #
41: goto27
</pre></div>

<p>
	Here's the same information as with the -i switch, and you can move up and down the ecl file as you please :)<br />
	The lines with numbers in front are the same as the ones you see on the console (or the ScriptExRef PC if you need it from a script)<br />
	The ones without numbers in front are lines from your source code. All your lines may not be including, for instance there's no "endfor" in the above list, but you should get enough to figure out where it happens.
</p>

<hr />
<br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br />

<a name="polcore"></a><h2>polcore()</h2>
<br />
<p>
	This section is about the polcore() commands, most of them are only listed with name + a short description, but they're all used in <a href="_tmp_fix">this example code</a>.<br />
</p>

<p>
	<b>
		polcore().uptime<br />
	</b>
  Returns the system uptime in seconds. Only really useful for checking how long the system takes to start up really, since ReadGameClock should be used to measure intervals
</p>

<p>
	<b>
		polcore().sysload<br />
		polcore().sysload_severity<br />
	</b>
		Returns the sysload as a percentage (0-100) or as severity (raw number, 0->). Good to check every couple of minutes, so you can tell if something's very wrong. The percentage value is more useful since actual load is hard to tell from the raw value.
</p>

<p>
	<b>
		polcore().bytes_sent<br />
		polcore().bytes_received<br />
	</b>
		Returns the number of bytes sent/received. Good to know if you have enough bandwidth (store the old value and display the difference every minute). Can also let you know if someone's trying to flood you, though it doesn't tell you who.
</p>

<p>
	<b>
		polcore().itemcount<br />
		polcore().mobilecount<br />
	</b>
		Returns total number of items/mobiles. Could check that items dont increase too fast, and that the number of mobiles stays apporoximately the same.
</p>

<p>
	<b>
		polcore().instr_per_min<br />
		polcore().events_per_min<br />
		polcore().combat_operations_per_min<br />
		polcore().skill_checks_per_min<br />
	</b>
		Returns the total number of instructions/events/combat operations/skill checks on the shard during the last minute. This can vary quite a lot, but doesn't hurt to check
</p>

<p>
	<b>
		polcore().script_profiles<br />
		polcore().log_profile(clear)<br />
		polcore().clear_script_profile_counters()<br />
	</b>
		Now we get to the interesting part :)<br />
		clear_script_profile_counters reset the counters, and log_profile also clears after dumping if clear is set. To see what the pol.log dump from .log_profile looks like, click <a name="logprofileref" href="#logprofile">here</a><br />
		These methods tell you how much the script that have been run since the last clear (or server start), log_profile writes it to pol.log while script_profiles returns an array with the following information for each script:<br />
		<table>
			<tr>
				<td>name:</td>
				<td>The name of the script :)</td>
			</tr>
			<tr>
				<td>instr:</td>
				<td>Total instructions the script has run</td>
			</tr>
			<tr>
				<td>invocations:</td>
				<td>The number of times the script has been called</td>
			</tr>
			<tr>
				<td>instr_per_invoc:</td>
				<td>Total instructions divided by number of invocations :)</td>
			</tr>
			<tr>
				<td>instr_percent:</td>
				<td>How much the script has run, as a percentage of all cycles run on shard</td>
			</tr>
		</table>

		Example:<br />
<div class="doc-guide-pre">
<pre>
program view_script_profiles(who)
	SendSysMessage(who, "Scripts that run a lot:");
	var vscript_profiles := polcore().script_profiles;
	foreach script in vscript_profiles
		if (script.instr_percent > 5)
			SendSysMessage(who, script.name + " has run " + script.instr +
				" (" + script.instr_percent + "%) instructions, " +
				script.instr_per_invok + " at a time");
		end if
	endforeach
endprogram
</pre></div>

	
</p>

<p>
	<b>
		polcore().running_scripts<br />
		polcore().all_scripts<br />
	</b>
		Returns the scripts that are currently in effect. running_scripts only returns the ones that aren't sleeping or waiting for events.<br />
		The return value is an array of ScriptExRef's, which can be used like normal structs to get information:<br />
		<table>
			<tr>
				<td>pid:</td>
				<td>Process id of the script</td>
			</tr>
			<tr>
				<td>name:</td>
				<td>Name of the script</td>
			</tr>
			<tr>
				<td>instr_cycles:</td>
				<td>Number of instructions the script has run so far</td>
			</tr>
			<tr>
				<td>consec_cycles:</td>
				<td>Number of cycles run since last time the script called sleep</td>
			</tr>
			<tr>
				<td>PC:</td>
				<td>Prog Counter, the current instruction the script is at.</td>
			</tr>
			<tr>
				<td>call_depth:</td>
				<td>Call depth, the number of function calls currently nested</td>
			</tr>
			<tr>
				<td>num_globals:</td>
				<td>Number of globals the script has used</td>
			</tr>
		</table>

		Example:<br />
<div class="doc-guide-pre">
<pre>
program view_script_status(who)
	SendSysMessage(who, "Loaded scripts:");
	var vall_script := polcore().all_scripts;
	foreach script in vall_scripts
		SendSysMessage(who, scripts.pid + ": " + script.name + " has run " +
			script.instr_cycles + " (" + script.consec_cycles + ", " +
			script.call_depth + ", " + script.num_globals + ")");
	endforeach
endprogram
</pre></div>

</p>

<br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br />

<hr />
<a name="scriptopts"></a><h2>Runtime script options</h2>
		set_priority( priority )<br />
		This sets a scripts priority (the number of instructions it runs at a time). A higher number means your script gets run faster, on behalf of all the other scripts. The default number is 1, and the function returns the previous priority.

		set_script_option( optnum, optval )<br />

		This sets the various script options:
		<table>
			<tr>
				<td>SCRIPTOPT_NO_INTERRUPT:</td>
				<td>Makes the script run until it sleeps (like calling set_priority('infinite'). In effect this halts!! the shard until your script is done</td>
			</tr>
			<tr>
				<td>SCRIPTOPT_DEBUG:</td>
				<td>Prints any debug info included. Not used much.</td>
			</tr>
			<tr>
				<td>SCRIPTOPT_NO_RUNAWAY:</td>
				<td>Doesn't warn about runaway for this script. Only use this if you're sure what you're doing!</td>
			</tr>
			<tr>
				<td>SCRIPTOPT_CAN_ACCESS_OFFLINE_MOBILES:</td>
				<td>Whether or not the script has access to offline mobs</td>
			</tr>
		</table>
		Example:<br />
<div class="doc-guide-pre">
<pre>
program lag_scripts(who)
	SendSysMessage(who, "That was not a good idea...");
	var i,i2;
	set_priority(200);
	set_script_option(SCRIPTOPT_NO_RUNAWAY);
	for (i := 0; i < 100000; i := i + 1)
		for (i2 := 0; i2 < 100; i2 := i2 + 1)
			i2 := 122 + i2 + 12 * 12 - 22 + (5 * 5) - 25;
		endfor
	endfor
	SendSysMessage(who, "Phew...");
endprogram
</pre></div>

		set_critical( critical )<br />

		This is the same as calling set_script_option for the SCRIPTOPT_NO_INTERRUPT flag. <b>Don't use this unless you have to, it freezes the shard</b>!
		What it does is prevent the rest of the scripts from running at all until you turn critical off. The best use for this is when something has to be done without *anything* happening while its running.
		For instance this could be when you creating and modifying an object and it all has to happen at once so nothing can happen on the shard between creation and when you're done setting properties (remember to set_critical(0)!).
		Another example would be if you are using a global prop and you have to be sure it doesn't change. These situations usually can, and should, be avoided.

<br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br />
--- various code ---
<br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br />
<a name="runawaysquare">runawaysquare.src</a> <a href="#runawaysquareref">(Back)</a>
<br/><div class="doc-guide-pre"><pre>
use uo;
// this is *not* the right way to compute squares ;)
program runawaysquare(who, text)
	var i := CInt(text);
	var a,b,sum := 0;
	for (a := 0; a < i; a := a + 1)
		for (b := 0; b < i; b := b + 1)
			sum := sum + 1;
		endfor
	endfor
	SendSysMessage(who, i + "^2 = " + sum);
endprogram
</pre></div>
<a href="#runawaysquareref">(Back)</a>
<br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br />
<br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br />
<a name="runawaysquarelisting">runawaysquare.lst</a> <a href="#runawaysquarelistingref">(Back)</a>
<br/><div class="doc-guide-pre"><pre>
program runawaysquare(who, text)
use uo;
0: get arg 'who'
1: get arg 'text'
var i := CInt(text);
2: decl local #2
3: local #1
4: Func(0,0): left
5: :=
6: #
var a,b,sum := 0;
7: decl local #3
8: #
9: decl local #4
10: #
11: decl local #5
12: 0L
13: :=
14: #
for (a := 0; a < i; a := a + 1)
15: local #3
16: 0L
17: :=
18: #
19: local #3
20: local #2
21: <
22: if false goto 49
for (b := 0; b < i; b := b + 1)
23: local #4
24: 0L
25: :=
26: #
27: local #4
28: local #2
29: <
30: if false goto 42
sum := sum + 1;
31: local #5
32: 1L
33: +
34: local5 :=
35: local #4
36: local #4
37: 1L
38: +
39: :=
40: #
41: goto27
42: local #3
43: local #3
44: 1L
45: +
46: :=
47: #
48: goto19
SendSysMessage(who, i + "^2 = " + sum);
49: local #0
50: local #2
51: "^2 = "
52: +
53: local #5
54: +
55: 3L
56: 1000L
57: Func(2,0): Accessible
58: #
59: leave block(6)
60: progend
</pre></div>
<a href="#runawaysquarelistingref">(Back)</a>
<br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br />
<a name="logprofile">Example of logged script profile</a> <a href="#logfileref">(Back)</a>
<br/><div class="doc-guide-pre"><pre>
Script passes:    89469002
Script                                       cycles  invoc    cyc/invoc      %
scripts/misc/chrdeath.ecl                     59049    114          517      0
scripts/misc/death.ecl                        81944    340          241      0
scripts/misc/help.ecl                          1743      3          581      0
pkg/foundations/hooks/attributeCore.ecl       40015     32         1250      0
pkg/items/doors/closeDoor.ecl                 11313     73          154      0
pkg/items/doors/door.ecl                      67139    159          422      0
pkg/items/doors/openDoor.ecl                  10614     73          145      0
pkg/skills/alchemy/yellowPotion.ecl             702      8           87      0
pkg/skills/anatomy/anatomy.ecl                25626    144          177      0
pkg/skills/armslore/armsLore.ecl                165      1          165      0
pkg/skills/evalint/evalInt.ecl                17486    101          173      0
pkg/skills/healing/healing.ecl               949169    939         1010      3
pkg/skills/hiding/hiding.ecl                  24496    233          105      0
pkg/skills/meditation/medCheck.ecl          2653781     32        82930     10
pkg/skills/meditation/meditation.ecl            645      3          215      0
pkg/skills/spells/archProt.ecl                 1009      1         1009      0
pkg/skills/spells/bladeSpirit.ecl              9916      9         1101      0
pkg/skills/spells/bless.ecl                   22968     26          883      0
pkg/skills/spells/cure.ecl                     1116      3          372      0
pkg/skills/spells/eBolt.ecl                    1691      2          845      0
pkg/skills/spells/explosion.ecl               41158     79          520      0
pkg/skills/spells/fireball.ecl                63570    116          548      0
pkg/skills/spells/fstrike.ecl                 18623     84          221      0
pkg/skills/spells/gheal.ecl                  190957    962          198      0
pkg/skills/spells/harm.ecl                    73102    134          545      0
pkg/skills/spells/lightning.ecl               65582    127          516      0
pkg/skills/spells/paralyze.ecl                33259     61          545      0
pkg/skills/spells/poison.ecl                  68405    119          574      0
pkg/skills/spells/poisonDamage.ecl            89594    164          546      0
pkg/skills/spells/poisonField.ecl               487      1          487      0
pkg/skills/spells/protection.ecl               6929      8          866      0
pkg/skills/spells/reflect.ecl                 22964     91          252      0
pkg/skills/stealth/stealth.ecl                  249      3           83      0
pkg/skills/taming/taming.ecl                    121      1          121      0
pkg/skills/tinkering/tinkerTraps.ecl            346      2          173      0
pkg/systems/combat/bladeSpiritHit.ecl          6898     89           77      0
pkg/systems/combat/destroy.ecl                16518    285           57      0
pkg/systems/combat/earthElemental.ecl         14566    136          107      0
pkg/systems/combat/equip.ecl                 119729   1787           67      0
pkg/systems/combat/itemControl.ecl             7185    330           21      0
pkg/systems/combat/maceHit.ecl               154075   1022          150      0
pkg/systems/combat/mainHitScript.ecl         929549   3911          237      3
pkg/systems/combat/mobPoisonHit.ecl           19983    247           80      0
pkg/systems/combat/reactiveArmorHit.ecl      361106   5920           60      1
pkg/systems/combat/unequip.ecl                75054   1787           42      0
pkg/systems/combat/wrestlingHitScript.ecl    443890   2009          220      1
pkg/tools/motd/logon.ecl                        224     32            7      0
pkg/tools/motd/textcmd/player/motd.ecl         1024     32           32      0
scripts/misc/unmod.ecl                        10567     87          121      0
scripts/ai/bladeSpirit.ecl                    34475      5         6895      0
scripts/ai/dumbKillPCs.ecl                   839025    403         2081      3
scripts/ai/firebreather.ecl                   92077     57         1615      0
scripts/ai/immobile.ecl                     1123980     14        80284      4
scripts/ai/immobileSpell.ecl                 475435      3       158478      1
scripts/ai/killPCs.ecl                      5785841   1489         3885     22
scripts/ai/packFirebreather.ecl              103719     15         6914      0
scripts/ai/spellFire.ecl                     259221      2       129610      0
scripts/ai/spellKillPCs.ecl                 7041261    348        20233     26
scripts/ai/undead.ecl                       1317271     45        29272      5
scripts/boneControl.ecl                        1026    114            9      0
scripts/console/online.ecl                      230      2          115      0
scripts/control/canInsert.ecl                 17845    541           32      0
scripts/control/canRemove.ecl                 27882    992           28      0
scripts/control/corpseControl.ecl             20844    454           45      0
scripts/control/onInsert.ecl                  16042    553           29      0
scripts/control/onRemove.ecl                  19844    992           20      0
scripts/control/trashControl.ecl                378      2          189      0
scripts/control/trashInsert.ecl                 416     32           13      0
scripts/control/trashRemove.ecl                   9      1            9      0
scripts/items/bladed.ecl                        306      1          306      0
scripts/items/key.ecl                           808      6          134      0
scripts/misc/dblclickself.ecl                   225     15           15      0
scripts/misc/logoff.ecl                        8178     37          221      0
scripts/misc/logofftest.ecl                  120111     37         3246      0
scripts/misc/logon.ecl                        48058     32         1501      0
scripts/NPCKeeper.ecl                        648469   2376          272      2
</pre></div>
<a href="#logprofileref">(Back)</a>
<p style="text-align: right; font-size: xx-small;">
	The above guide was written by ToGu. No animals were hurt during the making of this document.
</p>
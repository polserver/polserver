<h1>Gumps : The original way</h1>
<br>
<h3>Topics:</h3>
<p>
<a href="#commands" style="letter-spacing:2pt;"><b>.Commands and generals</b></a><br>
<a href="#creation" style="letter-spacing:2pt;"><b>.Creating a picture-gump</b></a><br>
<a href="#interactive" style="letter-spacing:2pt;"><b>.Interactive gumps</b></a><br>
</p>
<br>
<h3>HowTo's:</h3>
<a href="#translucent_gump" style="letter-spacing:2pt;"><b>.How to make a gump translucent</b></a><br>
<a href="#textentry" style="letter-spacing:2pt;"><b>.How to create and use textentries</b></a><br>
<br>
<hr>
<br>
<a word="#commands" name="commands"><b>Commands and generals</b></a>
<p>
Let's start with a little list of the available commands. Don't wonder about their outlook. Gumps are a client-feature and have (nearly) nothing in common with eScript.
<br>
All parameters are colored <font color="#FF0000">red</font>.
<ul>
<li>NoMove</li>
<li>NoDispose</li>
<li>NoClose</li>
<li>Page <font color="#FF0000">[#]</font></li>
<li>GumpPic <font color="#FF0000">[x] [y] [id]</font></li>
<li>TilePic <font color="#FF0000">[x [y] [id]</font></li>
<li>ResizePic <font color="#FF0000">[x] [y] [gump-id] [width] [height]</font></li>
<li>Text <font color="#FF0000">[x] [y] [color] [text-id]</font></li>
<li>TextEntry <font color="#FF0000">[x] [y] [width] [height] [color] [return-value] [default-text-id]</font></li>
<li>Button <font color="#FF0000">[x] [y] [released-id] [pressed-id] [quit] [page-id] [return-value]</font></li>
<li>Group <font color="#FF0000">[groupnumber]</font></li>
<li>Radio <font color="#FF0000">[x] [y] [released-id] [pressed-id] [status] [return-value]</font></li>
<li>CheckBox <font color="#FF0000">[x] [y] [released-id] [pressed-id] [status] [return-value]</font></li>
<li>HtmlGump <font color="#FF0000">[x] [y] [width] [height] [text-id] [background] [scrollbar]</font></li>
<li>CheckerTrans <font color="#FF0000">[x] [y] [width] [height]</font></li>
</ul>

Those are the commands any gump consists of and which we will use during this tutorial (and you hopefully further on). Most commands are self-explanatory but since this is a tutorial I will introduce first the parameters and then all commands to you.
<br><br>
<b>[#]</b> : # represents a number, such as 0, 1, 2, 3, ... ,9. You mustn't use one number twice. Means: 1 and 11 would be ok, but 1 twice is wrong.<br>
<b>[x]</b> : x specifies the x-coordinate of the upper left corner of the object you want to position<br>
<b>[y]</b> : y specifies the y-coordinate of the upper left corner of the object you want to position<br>
<b>[id]</b> : id represents one of three possible ids: gump-picture-ids, tile-picture-ids and text-ids. Mostly you'll receive those numbers (ids) by using <a href="http://lystramon.elitecoder.net/view.php?f=58" target="_blank"><b>Inside UO</b></a>. Only exception: Text-ids, but I will explain how to get their ids, later. <br>
<b>[color]</b> : color represents a color-id (which is as always an <b>integer</b>). Thus you can color your text.<br>
<b>[width]</b> : This parameter is only used for resizepics and textentries. It sets the width of the picture or <i>[for textentries]</i> the width of the area in which the player may click in order to enable textentry.<br>
<b>[height]</b> : This parameter is only used for resizepics and textentries. It sets the height of the picture or <i>[for textentries]</i> the height of the area in which the player may click in order to enable textentry.<br>
<b>[quit]</b> : This parameter is only to be found in buttons. It determines whether you want to exit the gump after clicking the button or not.<br>
<b>[radiogroup]</b> : This parameter represents a number (0-9) which mustn't be used twice, either.<br>
<b>[released-id]</b> : This parameter sets the image you want to use for a button that is <b>not</b> pressed.<br>
<b>[pressed-id]</b> : This parameter sets the image you want to use for a button that <b>is</b> pressed.<br>
<b>[status]</b> : This parameter determines whether the radio-button or checkbox is already checked/pressed or not.<br>
<b>[return-value]</b> : The return value is what tells the script which follows the gump what to do. A return-value for a button should never double another button. <i>[except they are only used as "send"-buttons for radio- or checkbox values]</i><br><br>
Now the commands...
<br><br>
<b>NoMove</b> : This command is (if neccessary) always to be found on top of a gump. It tells the client whether the player may move the gump-"window" around or not.<br>
<b>NoDispose</b> : If this command is set you can not close the gump by hitting Esc (which otherwise is possible). This command should <b>always</b> be present. Otherwise there may occur problems.<br>
<b>NoClose</b> : Adding this line to a gump means that it can not be closed by right-clicking the mouse.<br>
<b>Page</b> : You may create different pages in gumps - e.g. a cook-book. Everything situated below this page-command belongs to the specified page. If another page-command follows everything below it will belong to that page ... and so on.
Example:
<pre><b>page 0</b>
This line belongs to page 0
This line, too
And this one
<b>page 1</b>
This now belongs to page 1
</pre>
You should know that <i>page 0</i> contains all background-commands. Means that everything you specify between <i>page 0</i> and <i>page 1</i> will be visible <b>everytime</b> the gump is open.<br>
<b>GumpPic</b> : Using this command you include <b>gump</b>-pictures into your gump. <i>[You best get (if you don't already have it) <a href="http://lystramon.elitecoder.net/view.php?f=58" target="_blank"><b>Inside UO</b></a> which is (in my opinion) the best program for searching for ids of any kind]</i> An example of gump-pictures? Take the spellbook - it consists of gump-pictures ... <b>only</b> gump-pictures. All weapons which you can see in the paperdoll belong to those gump-pictures. In Inside UO you'll find them in the (guess what?) gump-category.<br>
<b>TilePic</b> : This command includes a <b>tile</b>-picture into your gump. Tile-pictures are graphics of those items which you can see lying on the ground ... or in a bank box. Thus you could create a picture of a bank box without having one opened (just an example).<br>
<b>ResizePic</b> : ResizePics are also pics, but (as their name says) they can be resized to the width and height you like. Though you should know that only specific ids are capable of being used as ResizePics. Those are the following (Int): 83; 2520; 2600; 2620; 3500; 3600; 5054; 5100; 5120; 5170;<br>
<b>Text</b> : The first 3 parameters should be clear to all of you. The 4th represents an id which is determined by a "text-array". Sounds difficult, but is very easy.<br>Gumps are constructed in two arrays <i>[which we will see when building our first gump]</i>: a layout-array and a text-array. The 4th parameter thus is the number of the chosen text-piece in the text-array. I think an example is better <i>[now you'll see how a gump-script will look like, but I'll keep it very simple]</i>:
<br/>
<div class="doc-guide-pre"><pre>
var gflayout := {
		"page 0",
		"text 100 100 0 <b>0</b>" // last parameter = text-id
		};
var gfdata := {
		"This is the first textline"
		};
</pre></div>
You see, the last parameter is <b>0</b> and therefore means that the text which is to be found in the data-array on position 0 will be shown at position x=100, y=100 in color 0 (black). Yes, the first position in an array is 0 <i>[for gumps]</i> - not 1 as usual in eScript. <i>[as I said - they have nearly nothing in common]</i><br>
<b>TextEntry</b> : This command allows you to define an area in which a player can click and then enter a text of his choice.<br>
<b>Button</b> : This command is used in nearly every interactive gump. You can switch to other pages or execute specific actions with them. If you want to access another page, "pressed-id" and "return-value" have to be set zero and the page-id gets set the number of the page to which should be switched. If you want to execute an action you have to set "page-id" zero, mostly "quit" to 1 and then enter a specific value for each possible action. <i>[Don't worry if you don't understand this at the moment, we will handle interactive gumps later]</i><br>
<b>Group</b> : Group is used like "page", except the fact that group defines a radio-group and no page. Thus you can press many radio-buttons on one page without the need of checkboxes (which are mostly more complicated).<br>
<b>Radio</b> : Radio creates a radio-button, which is just like a normal button, except it just adds a value to those which will be returned to the script if a "normal" button is pressed.<br>
<b>CheckBox</b> : The same as radio-buttons except you can check many boxes instead of just one.<br>
<b>HtmlGump</b> : As the name indicates, this command allows you to use html-commands. Actually it creates a text-area, either with a background or without it and with or without a scrollbar. Additionally you pass a text-id - and this text can be formatted in the following ways:<br>
<pre>
&lt;center&gt; = means "center" - the text in between &lt;center&gt; and &lt;/center&gt; is always in the middle of your text-area
&lt;br&gt; = means "break" - the text goes on in the next line (this one <b>does not</b> need a "&lt;/br&gt;" at the end)
&lt;b&gt; = means "bold" - the text in between &lt;b&gt; and &lt;/b&gt; is displayed in bold letters
&lt;i&gt; = means "italic" - the text in between &lt;i&gt; and &lt;/i&gt; is displayed in italic letters
&lt;u&gt; = means "underlined" - the text in between &lt;u&gt; and &lt;/u&gt; is underlined
</pre>
One example for a html-formatted data-array-entry:
<br/>
<div class="doc-guide-pre">
<pre>
var gfdata := {
	      "&lt;center&gt;Hello guys!&lt;/center&gt;;&lt;br&gt;
	       &lt;br&gt;
	       This one is a html-gump.&lt;br&gt;
	       Features?&lt;br&gt;
	       &lt;b&gt;o&lt;/b&gt;Html-tag supported
	       &lt;b&gt;o&lt;/b&gt;Real Scrollbar
	       &lt;b&gt;o&lt;/b&gt;Nice background ;)"
	      };
</pre></div>
One thing you can actually <b>not do</b> <span class="comment">[or I just don't know how]</span> : Change the color of your text. It is always black. <span class="comment">[If htmlgump does not put out what I mention here, you may be using a client that is too old. The only chance is to use a more up-to-date one]</span>
<br>
<b>CheckerTrans</b> : Checkertrans creates transparent gumps. Any picture or resizepicture you defined <b>before</b> "checkertrans" will become transparent in the area you define with checkertrans. <span class="comment">[The same as above: You need a client about 3.x.x in order to be able to see an effect when using checkertrans]</span><br>
An example:
<br/>
<div class="doc-guide-pre">
<pre>
var gflayout := {
		"page 0",
		"resizepic 100 100 5054 100 200",
		"checkertrans 110 110 80 180"
		};
</pre></div>
That would create a transparent resizepic with a full visible border. <span class="comment">[I'm not totally sure about its outlook, but you'll get the idea of the effect]</span>
<br>
<hr>
Now some generals you should always remember:
<ul>
<li>Gump-scripts are <b>not</b> case-sensitive</li>
<li>You may <b>only</b> use Integers in gumps - <b>no hex</b> <i>[Thus you should have <a href="http://lystramon.elitecoder.net/view.php?f=82" target="_blank"><b>this tool</b></a>]</i></li>
<li>Page 0 is the <b>background</b> of each gump and thus <b>always</b> visible</li>
<li>When opening a gump page 0 and <i>[if specified]</i> page 1 will be visible</li>
<li>Don't forget the quotation-marks around each line of gump-script and the comma at the end <i>[Only at the end of each gump-array you may leave out the comma]</i></li>
<li>You best name the gump-arrays "gflayout" for the layout-array and "gfdata" for the data-array. Why? Because in this way you may write your gump in "the original way" but also have the possibility to use "GFFunctions" <i>[GF-Functions always use "gflayout" and "gfdata" as names for their arrays]</i></li>
<li>You must <b>not ever</b> reverse the usual order of textlines. Means: After line 0 follows 1, then 2, then 3 and so on. You may <b>not</b> put textline 2 after textline 3! This always creates an error which kills "the players" client.<br>Here an example:
<div class="doc-guide-pre">
<pre>
var gflayout := {
		"page 0",
		"text 100 100 0 <b>0</b>",
		"text 100 115 0 <b>1</b>"
		};
</pre></div>This is <b>correct</b>.
<div class="doc-guide-pre">
<pre>
var gflayout := {
		"page 0",
		"text 100 100 0 <b>1</b>",
		"text 100 115 0 <b>0</b>"
		};
</pre></div>This is <b>incorrect</b>.
</li>
<li>The amount of space between two textlines <i>[one below another]</i> should be 15</li>
<li><b>Never</b> double return-values. <i>[It often causes problems when you want to have an accurate result]</i></li>
<li>The return-value of the pressed button is always to be found in sendgump[0].<br>
An example:
<div class="doc-guide-pre">
<pre>
var choice := SendDialogGump( who, gflayout, gfdata );
var pressed_buttons_id := choice[0];
</pre></div></li>
</ul>
<hr>
</p>
<br/>
<a word="#creation" name="creation"><b>Creating a picture-gump</b></a>
<p>
Now this was all very theoretical. Now we'll experiment with these things. I guess most of you will start here and take a look at the descriptions later.

I mentioned some of these things before, but repetition did never harm anyone.<br>
So, a gump is constructed in <b>two arrays</b>. <i>[You should already know arrays and how to handle respectively create or modify them. If <b>not</b> please read <a href="http://pol.tumbolia.org/files/docs/EScript.html" target="_blank"><b>Rac's EScript-guide</b></a> first!]</i><br>
Normally you just write value after value in an array but to make it more clearly you should put them <b>one below another</b> - just like this:
<div class="doc-guide-pre">
<pre>
var gflayout := {
		"page 0",
		"text 100 100 0 0",
		};
</pre></div>
Thus you'll get a better overview and can add comments more easily.
<hr>
Now one picture follows which represents the gump we want to have when we finish this part of the tutorial <i>[Creating a picture-gump]</i> - note, that this is only one page out of two. I just want to keep the second one secret ;) :<br><br>
<center><img src="./picture_gump_finish_1.jpg" border=0 alt="page 0 and page 1" onMouseOver="status='page 0 and page 1'; return true; " onMouseOut="status=''; return true; "></center>
<br><br><hr>
Well - each gump mostly starts with one of the first three commands from <a href="#commands"><b>the list on top of this document</b></a>. It's optional to add them but mostly you will want the player to make a choice. <i>[Though you should know that you can also cancel a gump by exiting UO. To prevent this you'll have to add props to the char and maybe jail him until he makes his choice. But this doesn't belong to this topic and will be up to your fantasy.]</i><br>
Then you create the first page <i>[page 0]</i> which represents the background of <b>every</b> following page - mostly you'll put in pictures and some text-lines here.<br>
Now let's try to build the gump shown above. "The player" should look at all pages, so you have to forbid right-click-closing and quit by esc. Also he shouldn't move the window around, because the gump is very interesting so he should focus on it. <i>[You smell a bit of irony? Aw! ;) ]</i> Now our script would look like this <i>[Don't worry - there are comments and I will explain the difficult parts again below]</i>:
<div class="doc-guide-pre">
<pre>
use uo;
use os;

include "include/attributes";

program gumptest( who )

var gflayout := {
		"noclose",   <i>// close by right-click forbidden</i>
		"nodispose", <i>// you did not want to hit esc, did you? </i>
		"nomove",    <i>// you musn't move the window around</i>
		"page 0",    <i>// we initialize the first page which is the background</i>
    "resizepic 100 100 3600 300 300", <i>// this is the dark background</i>
		"text 145 120 32 0",  <i>// the headline "my first picture-gump"</i>
		"page 1",	      <i>// now we initialize the second page ...</i>
		"text 120 160 50 1",  <i>// ... and create some text-lines...</i>
		"text 120 175 50 2",
		"text 120 190 50 3",
		"text 120 205 50 4",
		"text 120 220 50 5",
		"text 120 250 50 6",
		"text 120 265 50 7",
		"button 240 360 5540 5541 0 2 0", <i>// the button to switch to page 2</i>
		"page 2",	      <i>// we initialize the third page (page 2)</i>
		"text 140 230 50 8",  <i>// the last text-line ...</i>
		"button 188 230 249 248 1 0 0"  <i>// ... in which we place a button </i>
		};
var gfdata := {
		"<[ My first Picture-gump ]>",	 <i>// these are only textlines which are used</i>
		"Your skin-color  : "+who.color, <i>// in the layout-description above, namely</i>
		"Your gender     : "+who.gender, <i>// in order from top to bottom.</i>
		"Your strength   : "+getattribute(who,ATTRIBUTEID_STRENGTH),
		"Your dexterity   : "+getattribute(who,ATTRIBUTEID_DEXTERITY),
		"Your intelligence : "+getattribute(who,ATTRIBUTEID_INTELLIGENCE),
		"Click the arrow to proceed",
		"to the next page",
		"Press  [okay]  to exit"	  <i>// the text around (and behind) the okay-button</i>
		};

SendDialogGump( who, gflayout, gfdata );

endprogram
</pre></div>
Well - read the comments carefully? <i>[If not, do it now, please]</i> Then you should have understood the main "body" of a gump. You <b>always</b> need "page 0" - the rest is <b>optional</b>. <i>[Though a gump without content seems to be a bit useless, don't you think?]</i><br>
Now the full dialog in short words:
<ul>
<li>Opening layout-array</li>
<li>Disabling close through right-click</li>
<li>Disabling close through Esc</li>
<li>Disabling the possibility to move the window</li>
<li>Initializing first page (background / page 0)</li>
<li>Writing content of background (headline + picture)</li>
<li>Initializing second page (start-page / page 1)</li>
<li>Writing content of second page (textlines + page-button)</li>
<li>Initializing third page (page 2)</li>
<li>Writing content of third page (textline + exit-button)</li>
<li>Closing layout-array and opening data-array</li>
<li>Writing content of text-lines</li>
<li>Closing data-array and sending the gump to the player</li>
</ul>
I hope everyone now knows how to write a simple gump. Then we'll try to create one of those tricky button, radio and checkbox-gumps and take a look at how to handle their output.
</p>
<br/>
<br/>
<a word="#interactive" name="interactive"><b>Interactive gumps</b></a>
<br/>
<br/>
<p>
Ok, we should first make clear what <b>interactive gumps</b> are: I see an <b>interactive gump</b> as a gump which "communicates" with a player and returns his choice so the script does what he wants it to do. Take the hair-dye-gump as an example of such a gump.
<br><br>
You already know how to write a (rather simple) gump - but that's only a piece of what you can do with gumps. You can create multiple-choice menus, election-gumps, shopping-lists, theater-bills and so on ...
<br>
So - how do we get to know what the player wants? First we have to make the send-part of our program a variable so we can refer to it, after the player made his choice. Like this:
<div class="doc-guide-pre"><pre>
var his_choice := SendDialogGump( who, gflayout, gfdata );
</pre></div>
Easy? Yes!<br>
Now we have to know how to get his choice out of that variable. This is nothing you can know - it's just  like this - all keys are saved in one part of that variable, called "<b>keys</b>". These keys build an array - more return<b>ed</b>-values - more keys. To access these keys you use some line like this:
<div class="doc-guide-pre"><pre>
var his_choice := SendDialogGump( who, gflayout, gfdata );
var keys := his_choice.keys;
</pre></div>
Now you would have the whole key-array in a new array, called keys - but this is no use. We have to split the keys and get the right one so we can make the script act in the way we want it to.
<br><br>
If you don't return any key <i>[e.g. you close the gump by right-click <b>without</b> having touched <b>anything</b>]</i> the length of the key-array is 0 and you won't find any key in it. On the other side if you push a button which's return-value is <b>not 0</b> or choose a radio/checkbox your key-array would have a length of 2 or 3 or 4 <i>[or higher]</i>- depending on how many boxes you choose and whether you press a button or not.<br>There's a certain order in which the different keys are to be found in the array namely from the low values to the high values. An example:<br>
You have one button whichs return-value is 1000 and 2 radio-buttons the return-values of which are 500 and 1500. If you now press the 500-valued radio-button and the "normal" button, the output would look like this:
<div class="doc-guide-pre"><pre>
keys[0] = 0	<i>// always empty</i>
keys[1] = 500	<i>// 500 < 1000; therefore you'll find the <b>radio</b>-value here</i>
keys[2] = 1000	<i>// 1000 > 500; thus the <b>button</b>'s key has got this place</i>
</pre></div>
If you press the 1500-valued radio-button and the "normal" button the output would look like this:
<div class="doc-guide-pre"><pre>
keys[0] = 0	<i>// always empty</i>
keys[1] = 1000	<i>// 1000 < 1500; therefore you'll find the <b>button</b>-value here</i>
keys[2] = 1500	<i>// 1500 > 1000; thus the <b>radio</b>'s key has got this place</i>
</pre></div>
I bet you all see the difference.<br>
To make things easier you best make the important values the highest. Thus you can use a loop like the following to read out the important value:
<div class="doc-guide-pre"><pre>
var choice;
foreach key in keys
	choice := key;
endforeach
</pre></div>
After this loop you would have the highest value in the variable <i>choice</i>. Then you can work with it, create a case-statement or many ifs - it's up to you.<br>
To make things really clear I will give a complete example:
<div class="doc-guide-pre"><pre>

use uo;
use os;

program gumptest( who )

var gflayout := {
		"page 0",
		"nodispose",
		"resizepic 0 0 5170 200 300",
		"radio 100 100 5540 5541 0 <b>100</b>", <i>// All radio-values are important and thus</i>
		"radio 100 120 5540 5541 0 <b>200</b>", <i>// greater than any button-value [in this example: <b>1</b>]</i>
		"radio 100 140 5540 5541 0 <b>300</b>",
		"button 100 180 249 248 1 0 <b>1</b>"
		};
var gfdata := { };

var choice := SendDialogGump( who, gflayout, gfdata );
var his_choice;
foreach key in ( choice.keys )
	his_choice := key;
endforeach

var output;
case ( his_choice )
	<b>100</b>: output := "The player chose the button the value of which is 100";
	<b>200</b>: output := "The player chose the button the value of which is 200";
	<b>300</b>: output := "The player chose the button the value of which is 300";
	default: output := "The player must have canceled the gump without doing anything";
	<i>
	// default means if the value is neither 1 nor any of the radio-values
	// [so the player canceled the gump without doing anything]. In this example it doesn't matter
	// whether you press the okay-button or not, the values are counted anyways. This is because
	// it is not important for this example. If you're writing a gump and want it to work correctly
	// you should embed a test if the okay-button was pressed
	// [like this: if ( choice[0] == 1 ) <- then the player has pressed the okay-button.
	// Why? You should know]</i>
endcase

SendSysMessage( who, output );

endprogram
</pre></div>
This would be a working gump. Depending on what the player chooses one of the given messages will appear. Why? That is what you should already know. I only wrote this example so you can see how a working gump with radios looks like. Sometimes some expressions are not comprehensible at once. So the code should explain what maybe some of you didn't understand exactly.<br><br>
<b>But</b> what do we do if we have not only one button and/or radios, but <b>checkboxes</b>? Using checkboxes <i>[or radios plus radiogroups but this will follow later (or earlier - I'm not ready with the tut ;)]</i> we will receive many values in <i>[maybe]</i> a more or less great amount of keys. Then it won't be enough if we get one value <i>[key]</i>. We have to get them all. Thus we will use the loop which we used above - only slightly changed. Look here:
<div class="doc-guide-pre"><pre>
var choice;
foreach key in keys
	choice := <b>choice +</b> key;
endforeach
</pre></div>
Now we add each key to the new variable, called choice. If it was enough to look at one radio/button -value <i>[see example-code above]</i> you now have to ask for a sum of all values respectively <b>all possible</b> values.
<br>
<b>Example</b>: [button return-value] + [checkbox-value] + [checkbox-value] = [value to ask for].<br>This is surely more complicated but <i>[as nearly always]</i> more features require more work.<br>
In order to clarify things I will give an example of a checkbox-program:
<div class="doc-guide-pre"><pre>
use uo;
use os;

program gumptest( who )

var gflayout := {
		"page 0",
		"noclose",
		"nodispose",
		"resizepic 0 0 5170 200 300",
		"checkbox 100 100 5540 5541 <b>1</b> 100",<i> // I set the "status"-parameter to <b>1</b>
						       // In this way the player <b>has to</b> make a decision</i>
		"checkbox 100 120 5540 5541 0 200",
		"checkbox 100 140 5540 5541 0 400",
		"button 100 180 249 248 1 0 1"
		};
var gfdata := { };

var choice := SendDialogGump( who, gflayout, gfdata );
var his_choice;
foreach key in ( choice.keys )
	his_choice := <b>his_choice +</b> key;
endforeach

var output;
<i>// Now you either have to create a formula or you must calculate
// all possible values and then ask for them. I'll use the second method now
// CB = Checkbox</i>
case ( his_choice )
	101: output := "CB 1 + Button";
	201: output := "CB 2 + Button";
	301: output := "CB 1 + CB 2 + Button";
	401: output := "CB 3 + Button";
	501: output := "CB 1 + CB 3 + Button";
	601: output := "CB 2 + CB 3 + Button";
	701: output := "CB 1 + CB 2 + CB 3 + Button";
	default: "output: This should not have happened";
endcase

SendSysMessage( who, output );

endprogram
</pre></div>
Whenever you press a checkbox + button in this gump you should receive a message what you've done. <i>[It does not work? <a href="mailto:Lystramon@web.de?subject=Gump-Tutorial"><b>Tell me!</b></a>]</i>
</p>
<br>
<hr>
<br>
<a word="#translucent_gump" name="translucent_gump"><b>How to ... create a translucent gump</b></a><br>
<br>
You all know that you can create gumps which are translucent <i>[yes you do - at least now :)]</i>. Those gumps were introduced with <i>[I'm pretty sure ;)]</i> UO Renaissance. Thus you can <b>only</b> use that feature if you use a <b>client > 3.0.6</b> <i>[If it's not exactly that version, try another - at least with 3.0.8j it's definitely possible!]</i>. If you don't have the needed client, UO will ignore the command and display everything "as usual".<br><br>
Creating such a gump is pretty easy - here's an example of such a thing:

<div class="doc-guide-pre"><pre>
use uo;
use os;

program translucent_gump_test( who )

var gflayout := {   "page 0",
                    "resizepic 50 80 2620 400 40",  <i>// This resizepic is completely translucent</i>
		    "text 65 86 2101 0",            <i>// This text is translucent, too</i>
                    "checkertrans 56 86 386 28",
		    "text 250 86 2101 1"            <i>// This text is 100% visible</i>
		};

var gfdata := { "Durchsichtiger Text",
		"Undurchsichtiger Text"
	      };

SendDialogGump( who, gflayout, gfdata );

endprogram
</pre></div>

Well - why does the gump appear as it does? Simple explanation: The <i>checkertrans</i>-command defines an area - using the given parameters - and makes everything which <b>lies in that area and</b> which has been defined <b>before</b> it, translucent. Of course you may create those translucent areas as big or small as you like. Is that everything? Yes, it is!
<br><br>
<hr>
<br>
<a word="#textentry" name="textentry"><b>How to ... create and use textentries</b></a><br>
<br>
Some people asked how to create textareas so here's an example and - as usual - an explanation afterwards:
<br><br>
The following script will - after having been compiled and executed - print, what you typed into the given textarea if you right-click the gump-window.

<div class="doc-guide-pre"><pre>
use uo;
use os;
use util;
use cfgfile;

program textentry_gump( who )

var gflayout := {   "page 0",
                    "resizepic 50 80 2620 400 110",
                    "checkertrans 56 86 386 288",
                    "text 160 100 2101 0",
                    "TextEntry 150 130 200 20 40 <font color="#0000FF">0</font> <font color="#FF0000">1</font>"     <i>// An explanation of the parameters can be found in <a href="#commands"><b>this chapter</b></a></i>
		};

var gfdata := { "Textentry-Gump-Example",
                <font color="#FF0000">"YOUR TEXT HERE"</font>     <i>// The default text of the textentry</i>
	      };

var result := SendDialogGump( who, gflayout, gfdata );

sendsysmessage( who, "You typed: "+result[<font color="#0000FF">0</font>][4, len(result[<font color="#0000FF">0</font>])] );

endprogram
</pre></div>

<b>Short info for the script above</b>: As to the textentry, all things that are connected to each other are marked in the same colors.<br><br>
The result of the textentry is to be found as the first key in the result-array of the gump, because we have no button, nor any radios etc. etc. The "connection-colors" should clarify how the numbers are connected.<br>
We just fetch the data from the textentry - then we <b>could</b> use it if only UO wouldn't send the textentry-id in front of it. So we cut it off via <i>[4, len(result[0])]</i> <i>[As far as I know this method is also mentioned in Rac's EScript-guide, but I'm not sure]</i>. Thus we get all characters from the fourth on until the end of the string.<br>
<br>
Although this doesn't really belong here, I'll explain the "cut-off"-method:<br>
The formula: stringvar[from_char, how many chars after it?].<br><br>
<b>An example:</b>
<div class="doc-guide-pre"><pre>
var mystring := "I ate a hotdog";
    mystring[9, 3] := "";
</pre></div>

Now mystring contains: "I ate a dog".


<!-- END-TAG -->
<br/>
<br/>
<hr>
<b>Author:</b> Lystramon <i>(~lys)</i><br>
<b>E-mail:</b> <A href="mailto:lystramon<NOSPAM>@web.de?subject=POL-Gump-Tutorial&body=Remember to remove the <NOSPAM>-Tag in my adress!">Lystramon<NOSPAM>@web.de</A>
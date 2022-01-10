<h1>Gump Command List</h1>
<div class="container">
  <div class="doc-col_2">
    <ul>
      <li><a href="#noclose">NoClose</a></li>
      <li><a href="#nodispose">NoDispose</a></li>
      <li><a href="#nomove">NoMove</a></li>
      <li><a href="#group">Group</a></li>
      <li><a href="#endgroup">EndGroup</a></li>
      <li><a href="#page">Page</a></li>
      <li><a href="#mastergump">MasterGump</a></li>
    </ul>
    <br/>

    <ul>
      <li><a href="#button">Button</a></li>
      <li><a href="#buttontileart">ButtonTileArt</a></li>
      <li><a href="#checkbox">CheckBox</a></li>
      <li><a href="#checkertrans">CheckerTrans</a></li>
      <li><a href="#croppedtext">CroppedText</a></li>
      <li><a href="#gumppic">GumpPic</a></li>
      <li><a href="#gumppictiled">GumpPicTiled</a></li>
      <li><a href="#htmlgump">HtmlGump</a></li>
      <li><a href="#radio">Radio</a></li>
      <li><a href="#resizepic">ResizePic</a></li>
      <li><a href="#text">Text</a></li>
      <li><a href="#textentry">TextEntry</a></li>
      <li><a href="#textentrylimited">TextEntryLimited</a></li>
      <li><a href="#tilepic">TilePic</a></li>
      <li><a href="#tilepichue">TilePicHue</a></li>
      <li><a href="#tooltip">Tooltip</a></li>
      <li><a href="#xmfhtmlgump">XmfHtmlGump</a></li>
      <li><a href="#xmfhtmlgumpcolor">XmfHtmlGumpColor</a></li>
      <li><a href="#xmfhtmltok">XmfHtmlTok</a></li>
    </ul>
  </div>
  <div class="doc-col_23">
    <p>A short compilation of available gump commands.</p>

    <table width="98%">
      <tr>
        <td>
        <a name="noclose"></a>
        <table frame="box" rules="none" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
          <tr>
            <td width="120px">CmdName:</td>
            <td>NoClose</td>
          </tr>
          <tr>
            <td>Client introduced:</td>
            <td>-</td>
          </tr>
          <tr>
            <td>Description:</td>
            <td>Prevents that the gump can be closed by right clicking.</td>
          </tr>
        </table>
        </td>
      </tr>

      <tr>
        <td>
        <a name="nodispose"></a>
        <table frame="box" rules="none" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
          <tr>
            <td width="120px">CmdName:</td>
            <td>NoDispose</td>
          </tr>
          <tr>
            <td>Client introduced:</td>
            <td>-</td>
          </tr>
          <tr>
            <td>Description:</td>
            <td>Prevents that the gump can be closed by hitting Esc. Be carefull if you allow it, there may occur problems.</td>
          </tr>
        </table>
        </td>
      </tr>

      <tr>
        <td>
        <a name="nomove"></a>
        <table frame="box" rules="none" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
          <tr>
            <td width="120px">CmdName:</td>
            <td>NoMove</td>
          </tr>
          <tr>
            <td>Client introduced:</td>
            <td>-</td>
          </tr>
          <tr>
            <td>Description:</td>
            <td>Locks the gump in his position. Movement is not allowed.</td>
          </tr>
        </table>
        </td>
      </tr>

      <tr>
        <td>
        <a name="group"></a>
        <table frame="box" rules="none" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
          <tr>
            <td width="120px">CmdName:</td>
            <td>Group <i>[Number]</i></td>
          </tr>
          <tr>
            <td>Client introduced:</td>
            <td>-</td>
          </tr>
          <tr>
            <td>Description:</td>
            <td>Links radio buttons to a group. Add this before radiobuttons to do so. See also endgroup.</td>
          </tr>
        </table>
        </td>
      </tr>
      
      <tr>
        <td>
        <a name="endgroup"></a>
        <table frame="box" rules="none" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
          <tr>
            <td width="120px">CmdName:</td>
            <td>EndGroup</td>
          </tr>
          <tr>
            <td>Client introduced:</td>
            <td>between 4.0.4d and 5.0.0b</td>
          </tr>
          <tr>
            <td>Description:</td>
            <td>Ends the group-definition, only then group works on pages bigger than 1.</td>
          </tr>
        </table>
        </td>
      </tr>

      <tr>
        <td>
        <a name="page"></a>
        <table frame="box" rules="none" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
          <tr>
            <td width="120px">CmdName:</td>
            <td>Page <i>[Number]</i></td>
          </tr>
          <tr>
            <td>Client introduced:</td>
            <td>-</td>
          </tr>
          <tr>
            <td>Description:</td>
            <td>Specifies which page to define. Page 0 is the background thus always visible.</td>
          </tr>
        </table>
        </td>
      </tr>
      
      <tr>
        <td>
        <a name="mastergump"></a>
        <table frame="box" rules="none" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
          <tr>
            <td width="120px">CmdName:</td>
            <td>MasterGump <i>[Number]</i></td>
          </tr>
          <tr>
            <td>Client introduced:</td>
            <td>between 4.0.4d and 5.0.0b</td>
          </tr>
          <tr>
            <td>Description:</td>
            <td>Overrides the GumpID send with the gumppacket.</td>
          </tr>
        </table>
        </td>
      </tr>

      <tr>
      <td>
      &nbsp;
      </td>
      </tr>
      
      <tr>
        <td>
        <a name="button"></a>
        <table frame="box" rules="none" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
          <tr>
            <td width="120px">CmdName:</td>
            <td>Button <i>[x] [y] [released-id] [pressed-id] [quit] [page-id] [return-value]</i></td>
          </tr>
          <tr>
            <td>Client introduced:</td>
            <td>-</td>
          </tr>
          <tr>
            <td>Description:</td>
            <td>Adds a button to the gump with the specified coordinates and button graphics.
[released-id] and [pressed-id] specify the buttongraphic. If pressed check for [return-value].
Use [page-id] to switch between pages and [quit]=1/0 to close the gump.</td>
          </tr>
        </table>
        </td>
      </tr>

      <tr>
        <td>
        <a name="buttontileart"></a>
        <table frame="box" rules="none" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
          <tr>
            <td width="120px">CmdName:</td>
            <td>ButtonTileArt <i>[x] [y] [released-id] [pressed-id] [quit] [page-id] [return-value] [tilepic-id] [hue] [tile-x] [tile-y]</i></td>
          </tr>
          <tr>
            <td>Client introduced:</td>
            <td>between 4.0.4d and 5.0.0b</td>
          </tr>
          <tr>
            <td>Description:</td>
            <td>Adds a button to the gump with the specified coordinates and tilepic as graphic.
 [tile-x] and [tile-y] define the coordinates of the tile graphic and are relative to [x] and [y].</td>
          </tr>
        </table>
        </td>
      </tr>

      <tr>
        <td>
        <a name="checkbox"></a>
        <table frame="box" rules="none" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
          <tr>
            <td width="120px">CmdName:</td>
            <td>CheckBox <i>[x] [y] [released-id] [pressed-id] [status] [return-value]</i></td>
          </tr>
          <tr>
            <td>Client introduced:</td>
            <td>-</td>
          </tr>
          <tr>
            <td>Description:</td>
            <td>Adds a CheckBox to the gump. Multiple CheckBoxes can be pressed at the same time.
Check the [return-value] if you want to know which CheckBoxes were selected.</td>
          </tr>
        </table>
        </td>
      </tr>

      <tr>
        <td>
        <a name="checkertrans"></a>
        <table frame="box" rules="none" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
          <tr>
            <td width="120px">CmdName:</td>
            <td>CheckerTrans <i>[x] [y] [width] [height]</i></td>
          </tr>
          <tr>
            <td>Client introduced:</td>
            <td>-</td>
          </tr>
          <tr>
            <td>Description:</td>
            <td>Creates a transparent rectangle on position [x,y] using [width] and [height].</td>
          </tr>
        </table>
        </td>
      </tr>

      <tr>
        <td>
        <a name="croppedtext"></a>
        <table frame="box" rules="none" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
          <tr>
            <td width="120px">CmdName:</td>
            <td>CroppedText <i>[x] [y] [width] [height] [color] [text-id]</i></td>
          </tr>
          <tr>
            <td>Client introduced:</td>
            <td>-</td>
          </tr>
          <tr>
            <td>Description:</td>
            <td>Adds a text field to the gump. This is similar to the text command, but the text is cropped to the defined area.</td>
          </tr>
        </table>
        </td>
      </tr>

      <tr>
        <td>
        <a name="gumppic"></a>
        <table frame="box" rules="none" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
          <tr>
            <td width="120px">CmdName:</td>
            <td>GumpPic <i>[x] [y] [id] hue=[color] class=[class]</i></td>
          </tr>
          <tr>
            <td>Client introduced:</td>
            <td>-</td>
          </tr>
          <tr>
            <td>Description:</td>
            <td>Adds a graphic to the gump, where [id] is the graphic id of an item. For example use InsideUO to get them.
Optionaly there is a color parameter. The class parameter is also optional, triggers special behavior (e.g. "VirtueGump" will enable virtue tooltips).</td>
          </tr>
        </table>
        </td>
      </tr>

      <tr>
        <td>
        <a name="gumppictiled"></a>
        <table frame="box" rules="none" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
          <tr>
            <td width="120px">CmdName:</td>
            <td>GumpPicTiled <i>[x] [y] [width] [height] [id]</i></td>
          </tr>
          <tr>
            <td>Client introduced:</td>
            <td>3.0.4p</td>
          </tr>
          <tr>
            <td>Description:</td>
            <td>Similar to GumpPic, but the gumppic is tiled to the given [height] and [width].</td>
          </tr>
        </table>
        </td>
      </tr>

      <tr>
        <td>
        <a name="htmlgump"></a>
        <table frame="box" rules="none" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
          <tr>
            <td width="120px">CmdName:</td>
            <td>HtmlGump <i>[x] [y] [width] [height] [text-id] [background] [scrollbar]</i></td>
          </tr>
          <tr>
            <td>Client introduced:</td>
            <td>-</td>
          </tr>
          <tr>
            <td>Description:</td>
            <td>Defines a text-area where Html-commands are allowed. [background] and [scrollbar]
            	 can be 0 or 1 and define whether the background is transparent and a scrollbar s displayed.<br>
               <code>
               	&lt;B&gt;&lt;/B&gt; &lt;BIG&gt;&lt;/BIG&gt; &lt;SMALL&gt;&lt;/SMALL&gt; &lt;EM&gt;&lt;/EM&gt; &lt;I&gt;&lt;/I&gt;	&lt;U&gt;&lt;/U&gt;<br>
               	&lt;H1&gt;&lt;/H1&gt; &lt;H2&gt;&lt;/H2&gt; &lt;H3&gt;&lt;/H3&gt; 
               	&lt;H4&gt;&lt;/H4&gt; &lt;H5&gt;&lt;/H5&gt; &lt;H6&gt;&lt;/H6&gt;
               	&lt;a href=""&gt;&lt;/a&gt;<br>
               	&lt;div align=&quot;right&quot;&gt;&lt;div align=&quot;left&quot;&gt;&lt;/DIV&gt;
               	&lt;left&gt;&lt;/left&gt; &lt;P&gt;
               	&lt;CENTER&gt;&lt;/CENTER&gt; &lt;BR&gt;&lt;/BR&gt;
               	&lt;BASEFONT color=#ffffff size=1-7&gt;&lt;/BASEFONT&gt;
               </code>
            </td>
          </tr>
        </table>
        </td>
      </tr>

      <tr>
        <td>
        <a name="radio"></a>
        <table frame="box" rules="none" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
          <tr>
            <td width="120px">CmdName:</td>
            <td>Radio <i>[x] [y] [released-id] [pressed-id] [status] [return-value]</i></td>
          </tr>
          <tr>
            <td>Client introduced:</td>
            <td>-</td>
          </tr>
          <tr>
            <td>Description:</td>
            <td>Same as Checkbox, but only one Radiobutton can be pressed at the same time, except they are per linked
  via the 'Group' command.</td>
          </tr>
        </table>
        </td>
      </tr>

      <tr>
        <td>
        <a name="resizepic"></a>
        <table frame="box" rules="none" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
          <tr>
            <td width="120px">CmdName:</td>
            <td>ResizePic <i>[x] [y] [gump-id] [width] [height]</i></td>
          </tr>
          <tr>
            <td>Client introduced:</td>
            <td>-</td>
          </tr>
          <tr>
            <td width="120px">Description:</td>
            <td>Similar to GumpPic but the pic is automatically resized to the given [width] and [height].</td>
          </tr>
        </table>
        </td>
      </tr>

      <tr>
        <td>
        <a name="text"></a>
        <table frame="box" rules="none" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
          <tr>
            <td width="120px">CmdName:</td>
            <td>Text <i>[x] [y] [color] [text-id]</i></td>
          </tr>
          <tr>
            <td>Client introduced:</td>
            <td>-</td>
          </tr>
          <tr>
            <td>Description:</td>
            <td>Defines the position and color of a text (data) entry.</td>
          </tr>
        </table>
        </td>
      </tr>

      <tr>
        <td>
        <a name="textentry"></a>
        <table frame="box" rules="none" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
          <tr>
            <td width="120px">CmdName:</td>
            <td>TextEntry <i>[x] [y] [width] [height] [color] [return-value] [default-text-id]</i></td>
          </tr>
          <tr>
            <td>Client introduced:</td>
            <td>-</td>
          </tr>
          <tr>
            <td>Description:</td>
            <td>Defines an area where the [default-text-id] is displayed.
The player can modify this data. To get this data check the [return-value]. The maximum number of characters for a TextEntry element is 239.</td>
          </tr>
        </table>
        </td>
      </tr>

      <tr>
        <td>
        <a name="textentrylimited"></a>
        <table frame="box" rules="none" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
          <tr>
            <td width="120px">CmdName:</td>
            <td>TextEntryLimited <i>[x] [y] [width] [height] [color] [return-value] [default-text-id] [textlen]</i></td>
          </tr>
          <tr>
            <td>Client introduced:</td>
            <td>between 4.0.4d and 5.0.0b</td>
          </tr>
          <tr>
            <td>Description:</td>
            <td>Similar to TextEntry but you can specify via [textlen] the maximum of characters the player can type in.</td>
          </tr>
        </table>
        </td>
      </tr>

      <tr>
        <td>
        <a name="tilepic"></a>
        <table frame="box" rules="none" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
          <tr>
            <td width="120px">CmdName:</td>
            <td>TilePic <i>[x] [y] [id]</i></td>
          </tr>
          <tr>
            <td>Client introduced:</td>
            <td>-</td>
          </tr>
          <tr>
            <td>Description:</td>
            <td>Adds a Tilepicture to the gump. [id] defines the tile graphic-id. For example use InsideUO to get them.</td>
          </tr>
        </table>
        </td>
      </tr>

      <tr>
        <td>
        <a name="tilepichue"></a>
        <table frame="box" rules="none" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
          <tr>
            <td width="120px">CmdName:</td>
            <td>TilePicHue <i>[x] [y] [id] [hue]</i></td>
          </tr>
          <tr>
            <td>Client introduced:</td>
            <td>between 4.0.4d and 5.0.0b</td>
          </tr>
          <tr>
            <td>Description:</td>
            <td>Similar to the tilepic command, but with an additional hue parameter.</td>
          </tr>
        </table>
        </td>
      </tr>

      <tr>
        <td>
        <a name="tooltip"></a>
        <table frame="box" rules="none" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
          <tr>
            <td width="120px">CmdName:</td>
            <td>Tooltip <i>[cliloc-nr]</i></td>
          </tr>
          <tr>
            <td>Client introduced:</td>
            <td>between 3.0.8q and 4.0.0c</td>
          </tr>
          <tr>
            <td>Description:</td>
            <td>Adds to the previous layoutarray entry a Tooltip with the in [cliloc-nr] defined CliLoc entry.</td>
          </tr>
        </table>
        </td>
      </tr>

      <tr>
        <td>
        <a name="xmfhtmlgump"></a>
        <table frame="box" rules="none" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
          <tr>
            <td width="120px">CmdName:</td>
            <td>XmfHtmlGump <i>[x] [y] [width] [height] [cliloc-nr] [background] [scrollbar]</i></td>
          </tr>
          <tr>
            <td>Client introduced:</td>
            <td>1.26.4b</td>
          </tr>
          <tr>
            <td>Description:</td>
            <td>Similar to the htmlgump command, but in place of the [text-id] a CliLoc entry is used.</td>
          </tr>
        </table>
        </td>
      </tr>

      <tr>
        <td>
        <a name="xmfhtmlgumpcolor"></a>
        <table frame="box" rules="none" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
          <tr>
            <td width="120px">CmdName:</td>
            <td>XmfHtmlGumpColor <i>[x] [y] [width] [height] [cliloc-nr] [background] [scrollbar] [color]</i></td>
          </tr>
          <tr>
            <td>Client introduced:</td>
            <td>3.0.4p</td>
          </tr>
          <tr>
            <td>Description:</td>
            <td>Similar to the xmfhtmlgump command, but additionally a [color] can be specified.</td>
          </tr>
        </table>
        </td>
      </tr>

      <tr>
        <td>
        <a name="xmfhtmltok"></a>
        <table frame="box" rules="none" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
          <tr>
            <td width="120px">CmdName:</td>
            <td>XmfHtmlTok <i>[x] [y] [width] [height] [background] [scrollbar] [color] [cliloc-nr] @[arguments]@</i></td>
          </tr>
          <tr>
            <td>Client introduced:</td>
            <td>5.0.1d</td>
          </tr>
          <tr>
            <td>Description:</td>
            <td>Similar to xmfhtmlgumpcolor command, but the parameter order is different and an additionally [argument] entry enclosed with @'s can be used.
With this you can specify texts that will be added to the CliLoc entry.</td>
          </tr>
        </table>
        </td>
      </tr>
      <tr><td align="right"><b>Copyright ©2009 Turley</b></td></tr>
    </table>
  </div>
</div>
      

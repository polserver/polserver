<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="html" encoding="utf-8"/>
  <xsl:param name="offline" select="0"/>

  <xsl:template match="MODULES">
    <div id="main">
      <div class="container">
        <div class="doc-mainbox-big-gold">
        <center><h2>POL EScript Modules:</h2></center>
        <table class="doc-table" width="85%" border="0" CELLPADDING="2" CELLSPACING="4">
          <xsl:variable name="h" select="round(count(//MODULES/em) div 2)" />
          <xsl:for-each select="//MODULES/em">
            <xsl:variable name="p" select="position()" />
            <xsl:if test="$h>=$p">
              <tr>
                <td>
                  <xsl:apply-templates select="//MODULES/em[position()=$p]"/>
                </td>
                <td>
                  <xsl:apply-templates select="//MODULES/em[position()=$p+$h]"/>
                </td>
              </tr>
            </xsl:if>
          </xsl:for-each>
        </table>
      </div>
    </div>
  </div>

  <script type="text/javascript">
    // Called when a single function has been selected from a form
    function singleFunc(formEl)
    {
      var selectEl = formEl.elements['funcname'];
      var funcName = selectEl.options[selectEl.selectedIndex].value;
      window.location.href = formEl.action + '#' + funcName;
      return false;
    }
  </script>
  </xsl:template>

  <xsl:template match="em">
    <b>
      Select
      <xsl:value-of select="@nice"/>
      function:
    </b>
    <br/>
    <form name="form_{@name}" method="get" style="margin:0px">
      <xsl:choose>
        <xsl:when test="$offline=1">
          <xsl:attribute name="action"><xsl:value-of select="@name"/>.html</xsl:attribute>
        </xsl:when>
        <xsl:otherwise>
          <xsl:attribute name="action">fullfunc.php?xmlfile=<xsl:value-of select="@name"/></xsl:attribute>
        </xsl:otherwise>
      </xsl:choose>
      <xsl:attribute name="onsubmit">return singleFunc(this);</xsl:attribute>
      <select name="funcname">
        <xsl:for-each select="document(concat(@name,'.xml'))//ESCRIPT/function">
          <xsl:sort select="prototype"/>
          <option value="{@name}">
            <xsl:value-of select="@name"/>
          </option>
        </xsl:for-each>
      </select>
      <input type="hidden" name="xmlfile" value="{@name}"/>
      <input type="submit" value="Go"/>
      <a>
        <xsl:choose>
          <xsl:when test="$offline=1">
            <xsl:attribute name="href"><xsl:value-of select="@name"/>.html</xsl:attribute>
          </xsl:when>
          <xsl:otherwise>
            <xsl:attribute name="href">fullfunc.php?xmlfile=<xsl:value-of select="@name"/></xsl:attribute>
          </xsl:otherwise>
        </xsl:choose>
        (All of <xsl:value-of select="@nice"/>)
      </a>
    </form>
    <br/>
  </xsl:template>

</xsl:stylesheet>


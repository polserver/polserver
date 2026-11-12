<!--
	The module list of the documentation index: one row per module with its description
	and every function name, so a function can be found by scanning or with the filter
	above the list (docs.js).
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="html" encoding="utf-8"/>
  <xsl:param name="offline" select="0"/>

  <xsl:template match="MODULES">
    <div class="mods" id="modules">
      <xsl:apply-templates select="em"/>
    </div>
  </xsl:template>

  <xsl:template match="em">
    <xsl:variable name="doc" select="document(concat(@name,'.xml'))"/>
    <xsl:variable name="href">
      <xsl:choose>
        <xsl:when test="$offline=1"><xsl:value-of select="@name"/>.html</xsl:when>
        <xsl:otherwise>fullfunc.php?xmlfile=<xsl:value-of select="@name"/></xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="count" select="count($doc//ESCRIPT/function)"/>
    <!-- first sentence of the module description -->
    <xsl:variable name="desc" select="normalize-space($doc//ESCRIPT/fileheader/filedesc)"/>
    <xsl:variable name="sentence" select="substring-before(concat($desc, ' '), '. ')"/>

    <details class="mod" open="open" data-count="{$count}">
      <summary>
        <a class="name" href="{$href}"><xsl:value-of select="@nice"/>.em</a>
        <span class="n"><xsl:value-of select="$count"/><xsl:choose><xsl:when test="$count = 1"> function</xsl:when><xsl:otherwise> functions</xsl:otherwise></xsl:choose></span>
        <span class="d">
          <xsl:choose>
            <xsl:when test="$sentence != ''"><xsl:value-of select="$sentence"/>.</xsl:when>
            <xsl:otherwise><xsl:value-of select="$desc"/></xsl:otherwise>
          </xsl:choose>
        </span>
      </summary>
      <ul class="fnames">
        <xsl:for-each select="$doc//ESCRIPT/function">
          <xsl:sort select="@name"/>
          <li>
            <a href="{$href}#{@name}"><xsl:value-of select="@name"/></a>
          </li>
        </xsl:for-each>
      </ul>
    </details>
  </xsl:template>

</xsl:stylesheet>

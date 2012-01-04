<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="html" encoding="utf-8"/>
  <xsl:param name="offline" select="0"/>
  <xsl:template match="/">
    <div class="container">
      <div id="doc-mini-header">
        <div class="doc-home">
			    <a name="TOP">
				    <xsl:choose>
              <xsl:when test="$offline=1">
                <xsl:attribute name="href">index.html</xsl:attribute>
              </xsl:when>
              <xsl:otherwise>
                <xsl:attribute name="href">index.php</xsl:attribute>
              </xsl:otherwise>
            </xsl:choose>
            Home
				  </a>
				</div>
        <h1>
          <xsl:value-of select="ESCRIPT/header/topic"/>
        </h1>
        <b>
          Last Modified:
          <xsl:value-of select="ESCRIPT/header/datemodified"/>
        </b>
      </div>
    </div>
    <div id="main">
      <div class="container">
        <div class="doc-col_2">
          <div class="doc-smallbox-gold">
            <center>
              <h2>Releases:</h2>
              <span id="span_Functions">
                <ul>
                  <xsl:for-each select="ESCRIPT/version">
                    <li>
                      <a href="#{@name}">
                        <xsl:value-of select="@name"/>
                      </a>
                    </li>
                  </xsl:for-each>
                </ul>
              </span>
            </center>
          </div>
        </div>
        <div class="doc-col_23">
          <xsl:for-each select="ESCRIPT/version">
            <div class="doc-mainbox-gold">
              <div class="doc-anchor">
                <a name="{@name}" href="#TOP">[^]</a>
              </div>
              <h1><xsl:value-of select="@name"/></h1>
              <xsl:for-each select="entry">
                <table class="doc-changelog-table" frame="void" rules="groups" width="100%" border="1" cellpadding="1" cellspacing="0">
                  <colgroup align="left">
                    <col width="100px"/>
                    <col />
                  </colgroup>
                  <tbody>
                    <tr>
                      <th>
                        <xsl:value-of select="date"/>
                      </th>
                      <th>
                        <xsl:value-of select="author"/>
                      </th>
                    </tr>
                  </tbody>
                  <xsl:for-each select="change">
                    <tbody>
                      <xsl:choose>
                        <xsl:when test="(position() mod 2)=1">
                          <tr id="doc-table-light">
                            <td class="changelog-type">
                              <xsl:value-of select="@type"/>
                            </td>
                            <td class="changelog">
                              <xsl:copy-of select="node()"/>
                            </td>
                          </tr>
                        </xsl:when>
                        <xsl:otherwise>
                          <tr>
                            <td class="changelog-type">
                              <xsl:value-of select="@type"/>
                            </td>
                            <td class="changelog">
                              <xsl:copy-of select="node()"/>
                            </td>
                          </tr>
                        </xsl:otherwise>
                      </xsl:choose>
                    </tbody>
                  </xsl:for-each>
                </table>
              </xsl:for-each>
            </div>
          </xsl:for-each>
        </div>
      </div>
    </div>
  </xsl:template>
</xsl:stylesheet>

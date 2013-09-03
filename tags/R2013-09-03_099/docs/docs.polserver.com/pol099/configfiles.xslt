<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="html" encoding="utf-8"/>
  <xsl:param name="offline" select="0"/>
  <xsl:template match="/">
    <div class="container">
      <div id="doc-mini-header">
        <div class="doc-home">
			    <a>
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
        <xsl:for-each select="ESCRIPT/fileheader">
          <h1>
            <a name="TOP"/>
            <xsl:value-of select="@name"/>
          </h1>
          <p style="font-size: 10pt; font-weight: bold; text-align: left;">
            <xsl:value-of select="subheader"/>
          </p>
          <xsl:for-each select="desc">
            <xsl:value-of select="top"/>
            <ul>
              <xsl:for-each select="list">
                <li>
                  <xsl:value-of select="current()"/>
                </li>
              </xsl:for-each>
            </ul>
            <xsl:value-of select="bottom"/>
          </xsl:for-each>
          <b>
            <br/>
            Last Modified:
            <xsl:value-of select="datemodified"/>
          </b>
        </xsl:for-each>
      </div>
    </div>
    <div id="main">
      <div class="container">
        <div class="doc-col_2">
          <div class="doc-smallbox">
            <ul>
              <xsl:for-each select="ESCRIPT/cfgfile">
                <xsl:sort select="@name"/>
                <li>
                  <a href="#{@name}">
                    <xsl:value-of select="@name"/>
                  </a>
                </li>
              </xsl:for-each>
            </ul>
          </div>
        </div>
        <div class="doc-col_23">
          <xsl:for-each select="ESCRIPT/cfgfile">
            <xsl:sort select="@name"/>
            <div class="doc-mainbox-gold">
              <div class="doc-anchor">
                <a href="#TOP">[^]</a>
              </div>
              <table class="doc-table" frame="void" rules="groups" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
                <tbody>
                  <tr>
                    <th align="center" colspan="2">
                      <a name="{@name}">
                        <xsl:value-of select="@name"/>
                      </a>
                    </th>
                  </tr>
                </tbody>
                <xsl:if test="location">
                  <tbody>
                    <tr id="doc-table-dark">
                      <th align="left">Location:</th>
                      <td align="left">
                        <xsl:value-of select="location"/>
                      </td>
                    </tr>
                  </tbody>
                </xsl:if>
                <xsl:if test="numberallowed">
                  <tbody>
                    <tr id="doc-table-med">
                      <th align="left">Number Allowed</th>
                      <td align="left">
                        <xsl:value-of select="numberallowed"/>
                      </td>
                    </tr>
                  </tbody>
                </xsl:if>
                <xsl:if test="requiredtostart">
                  <tbody>
                    <tr id="doc-table-dark">
                      <th align="left">Required to Start?</th>
                      <td align="left">
                        <xsl:value-of select="requiredtostart"/>
                      </td>
                    </tr>
                  </tbody>
                </xsl:if>
                <xsl:if test="requiredby">
                  <tbody>
                    <tr id="doc-table-med">
                      <th align="left" colspan="2">Required/Used by</th>
                    </tr>
                  </tbody>
                  <tbody>
                    <xsl:for-each select="requiredby">
                      <xsl:choose>
                        <xsl:when test="(position() mod 2)=1">
                          <tr>
                            <td colspan="2">
                              <xsl:value-of select="current()"/>
                            </td>
                          </tr>
                        </xsl:when>
                        <xsl:otherwise>
                          <tr id="doc-table-light">
                            <td colspan="2">
                              <xsl:value-of select="current()"/>
                            </td>
                          </tr>
                        </xsl:otherwise>
                      </xsl:choose>
                    </xsl:for-each>
                  </tbody>
                </xsl:if>
                <tbody>
                  <tr id="doc-table-dark">
                    <th align="left" colspan="2">Purpose</th>
                  </tr>
                </tbody>
                <tbody>
                  <tr>
                    <td colspan="2">
                      <xsl:value-of select="purpose"/>
                    </td>
                  </tr>
                </tbody>
                <tbody>
                  <tr id="doc-table-dark">
                    <th align="left" colspan="2">Structure</th>
                  </tr>
                </tbody>
                <tbody>
                  <tr>
                    <td colspan="2">
                      <pre>
                        <xsl:value-of select="structure"/>
                      </pre>
                    </td>
                  </tr>
                </tbody>
                <xsl:if test="explain">
                  <tbody>
                    <tr id="doc-table-dark">
                      <th align="left" colspan="2">Explanation</th>
                    </tr>
                  </tbody>
                  <tbody>
                    <xsl:for-each select="explain">
                      <tr>
                        <td colspan="2">
                          <xsl:copy-of select="node()"/>
                        </td>
                      </tr>
                    </xsl:for-each>
                  </tbody>
                </xsl:if>
                <xsl:if test="related">
                <tbody>
                  <tr id="doc-table-dark">
                    <th align="left" colspan="2">Related Files</th>
                  </tr>
                </tbody>
                <tbody>
                  <xsl:for-each select="related">
                    <xsl:choose>
                      <xsl:when test="(position() mod 2)=1">
                        <tr>
                          <td colspan="2">
                            <a href="#{current()}">
                              <xsl:value-of select="current()"/>
                            </a>
                          </td>
                        </tr>
                      </xsl:when>
                      <xsl:otherwise>
                        <tr id="doc-table-light">
                          <td colspan="2">
                            <a href="#{current()}">
                              <xsl:value-of select="current()"/>
                            </a>
                          </td>
                        </tr>
                      </xsl:otherwise>
                    </xsl:choose>
                  </xsl:for-each>
                </tbody>
              </xsl:if>
            </table>
          </div>
        </xsl:for-each>
      </div>
    </div>
  </div>
</xsl:template>
</xsl:stylesheet>

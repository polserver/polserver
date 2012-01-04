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
				<h1>
					<xsl:value-of select="ESCRIPT/header/topic"/>
				</h1>
				<p>
					<xsl:value-of select="ESCRIPT/header/text"/>
				</p>
				<b>
					Last Modified:
					<xsl:value-of select="ESCRIPT/header/datemodified"/>
				</b>
			</div>
		</div>
		<div id="main">
			<div class="container">
				<div class="doc-mainbox">
					<ul>
						<br/>
						<xsl:for-each select="ESCRIPT/list">
							<xsl:sort select="@name"/>
							<li>
								<b>
									"
									<xsl:value-of select="@name"/>
									"
								</b>
								-
								<xsl:value-of select="."/>
							</li>
						</xsl:for-each>
					</ul>
				</div>
			</div>
		</div>
	</xsl:template>
</xsl:stylesheet>

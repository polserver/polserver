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
					<a name="TOP"/>
					<xsl:value-of select="ESCRIPT/header/fname"/>
					<br/>
				</h1>
				<p>Description:</p>
				<p>
					<xsl:value-of select="ESCRIPT/header/hdesc"/>
					<br/>
					<br/>
				</p>
				<b>
					Last Modified:
					<xsl:value-of select="ESCRIPT/header/datemodified"/>
				</b>
			</div>
		</div>
		<div id="main">
			<div class="container">
				<div class="doc-col_2">
					<div class="doc-smallbox">
						<ul>
							<xsl:for-each select="ESCRIPT/event">
								<xsl:sort select="ID"/>
								<li>
									<a href="#{name}">
										<xsl:value-of select="name"/>
									</a>
								</li>
							</xsl:for-each>
						</ul>
					</div>
				</div>
				<div class="doc-col_23">
					<xsl:for-each select="ESCRIPT/event">
						<xsl:sort select="ID"/>
						<div class="doc-mainbox-gold">
							<div class="doc-anchor">
								<a href="#TOP">[^]</a>
							</div>
							<table class="doc-table" frame="void" rules="groups" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
								<tbody>
									<tr>
										<th align="center" colspan="2">
											<a name="{name}">
												<xsl:value-of select="name"/>
											</a>
										</th>
									</tr>
								</tbody>
								<xsl:if test="explain">
									<tbody>
										<tr class="doc-table-dark">
											<th align="left" colspan="2">Explanation</th>
										</tr>
									</tbody>
									<tbody>
										<xsl:for-each select="explain">
											<tr>
												<td colspan="2">
													<xsl:value-of select="current()"/>
												</td>
											</tr>
										</xsl:for-each>
									</tbody>
								</xsl:if>
								<tbody>
									<tr class="doc-table-dark">
										<th align="left">ID#:</th>
										<td align="left">
											<xsl:value-of select="ID"/>
										</td>
									</tr>
								</tbody>
								<xsl:if test="member">
									<tbody>
										<tr class="doc-table-dark">
											<th align="left" colspan="2">Struct Members:</th>
										</tr>
									</tbody>
									<tbody>
										<tr class="doc-table-med">
											<td>Name</td>
											<td>Type</td>
										</tr>
									</tbody>
									<tbody>
										<xsl:for-each select="member">
											<tr>
												<td width="50%">
													<var>
														<xsl:value-of select="@mname"/>
													</var>
												</td>
												<td>
													<xsl:value-of select="@value"/>
												</td>
											</tr>
										</xsl:for-each>
									</tbody>
								</xsl:if>
								<xsl:if test="whensent">
									<tbody>
										<tr class="doc-table-dark">
											<th align="left" colspan="2">When sent:</th>
										</tr>
									</tbody>
									<tbody>
										<tr>
											<td colspan="2">
												<xsl:value-of select="whensent"/>
											</td>
										</tr>
									</tbody>
								</xsl:if>
								<xsl:if test="ranged">
									<tbody>
										<tr class="doc-table-dark">
											<th align="left" colspan="2">Ranged?</th>
										</tr>
									</tbody>
									<tbody>
										<tr>
											<td colspan="2">
												<xsl:value-of select="ranged"/>
											</td>
										</tr>
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

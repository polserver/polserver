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
				<b>
					<a name="TOP"/>
					Last Modified:
					<xsl:value-of select="ESCRIPT/fileheader/datemodified"/>
				</b>
			</div>
		</div>
		<div id="main">
			<div class="container">
				<div class="doc-mainbox">
					<table class="doc-table">
						<tr align="center">
							<td>
								<strong>
									<xsl:value-of select="ESCRIPT/fileheader/header"/>
								</strong>
							</td>
						</tr>
						<tr align="center">
							<td>
								<xsl:copy>
									<xsl:copy-of select="document('objref.svg')"/>
								</xsl:copy>
							</td>
						</tr>
					</table>
				</div>
			</div>
		</div>
		<div class="main">
			<xsl:for-each select="ESCRIPT/class">
				<xsl:sort select="@name"/>
				<div class="container" id="{@name}">
					<div class="doc-mainbox-big-gold">
						<div class="doc-anchor">
							<a href="#TOP">[^]</a>
						</div>
						<table class="doc-table" frame="void" rules="groups" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
						  <colgroup align="left">
								  <col width="25%"/>
								  <col width="18%"/>
								  <col width="52%"/>
								  <col width="5%"/>
							</colgroup>
							<tbody>
								<tr>
									<th align="center" colspan="4">
										<xsl:value-of select="@name"/>
									</th>
								</tr>
							</tbody>
							<xsl:if test="parent">
								<tbody>
									<tr class="doc-table-dark">
										<th align="left" colspan="4">Parent</th>
									</tr>
								</tbody>
								<tbody>
									<tr>
										<td colspan="4">
											<a href="#{parent}">
												<xsl:value-of select="parent"/>
											</a>
										</td>
									</tr>
								</tbody>
							</xsl:if>
							<xsl:if test="child">
								<tbody>
									<tr class="doc-table-dark">
										<th align="left" colspan="4">Children</th>
									</tr>
								</tbody>
								<tbody>
									<xsl:for-each select="child">
										<tr>
											<td colspan="4">
												<a href="#{current()}">
													<xsl:value-of select="current()"/>
												</a>
											</td>
										</tr>
									</xsl:for-each>
								</tbody>
							</xsl:if>
							<tbody>
								<tr class="doc-table-dark">
									<th align="left" colspan="4">Explanation</th>
								</tr>
							</tbody>
							<tbody>
								<tr>
									<td colspan="4">
										<xsl:value-of select="explain"/>
									</td>
								</tr>
							</tbody>
							<xsl:if test="member">
								<tbody>
									<tr class="doc-table-dark">
										<th align="left" colspan="4">Members</th>
									</tr>
								</tbody>
								<tbody>
									<tr class="doc-table-med">
										<td>Name</td>
										<td>Type</td>
										<td>Desc</td>
										<td>Access</td>
									</tr>
								</tbody>
								<tbody>
									<xsl:for-each select="member">
									  <xsl:sort select="@mname"/>
									  <xsl:choose>
									    <xsl:when test="(position() mod 2)=1">
  											<tr>
  												<td>
  													<xsl:value-of select="@mname"/>
  												</td>
  												<td>
  													<xsl:value-of select="@type"/>
  												</td>
  												<td>
                                                  <xsl:copy-of select="node()"/>
  												</td>
  												<td>
  													<xsl:value-of select="@access"/>
  												</td>
  											</tr>
										  </xsl:when>
										  <xsl:otherwise>
  											<tr class="doc-table-light">
  												<td>
  													<xsl:value-of select="@mname"/>
  												</td>
  												<td>
  													<xsl:value-of select="@type"/>
  												</td>
  												<td>
                                                  <xsl:copy-of select="node()"/>
  												</td>
  												<td>
  													<xsl:value-of select="@access"/>
  												</td>
  											</tr>
  									  </xsl:otherwise>
										</xsl:choose>
									</xsl:for-each>
								</tbody>
							</xsl:if>
							<xsl:if test="method">
								<tbody>
									<tr class="doc-table-dark">
										<th align="left" colspan="4">Methods</th>
									</tr>
								</tbody>

								<tbody>
									<tr class="doc-table-med">
										<td>Prototype</td>
										<td>Returns</td>
										<td colspan="2">Desc</td>
									</tr>
								</tbody>
								<tbody>
									<xsl:for-each select="method">
									  <xsl:sort select="@proto"/>
									  <xsl:choose>
  										<xsl:when test="(position() mod 2)=1">
  											<tr>
  												<td>
  													<xsl:value-of select="@proto"/>
  												</td>
  												<td>
  													<xsl:value-of select="@returns"/>
  												</td>
  												<td colspan="2">
                                                  <xsl:copy-of select="node()"/>
  												</td>
  											</tr>
  										</xsl:when>
  										<xsl:otherwise>
  											<tr class="doc-table-light">
  												<td>
  													<xsl:value-of select="@proto"/>
  												</td>
  												<td>
  													<xsl:value-of select="@returns"/>
  												</td>
  												<td colspan="2">
                                                  <xsl:copy-of select="node()"/>
  												</td>
  											</tr>
  										</xsl:otherwise>
  								</xsl:choose>
									</xsl:for-each>
								</tbody>
							</xsl:if>
						</table>
					</div>
				</div>
			</xsl:for-each>
		</div>
	</xsl:template>
</xsl:stylesheet>

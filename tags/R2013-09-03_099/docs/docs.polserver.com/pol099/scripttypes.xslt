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
					<p>
						<xsl:value-of select="desc"/>
					</p>
					<b>
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
							<xsl:for-each select="ESCRIPT/scripttype">
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
					<xsl:for-each select="ESCRIPT/scripttype">
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
									<tr>
										<th align="center" colspan="2">
											<xsl:value-of select="prototype"/>
										</th>
									</tr>
									<xsl:if test="parameter">
										<tr id="doc-table-dark">
											<th align="left" colspan="2">Parameters:</th>
										</tr>
										<tr>
											<td>Name</td>
											<td>Type</td>
										</tr>
										<xsl:for-each select="parameter">
											<tr>
												<td width="50%">
													<var>
														<xsl:value-of select="@name"/>
													</var>
												</td>
												<td>
													<xsl:value-of select="@value"/>
												</td>
											</tr>
										</xsl:for-each>
									</xsl:if>
									<tr id="doc-table-dark">
										<th align="left" colspan="2">Return values</th>
									</tr>
									<xsl:for-each select="return">
										<tr>
											<td colspan="2">
												<xsl:value-of select="current()"/>
											</td>
										</tr>
									</xsl:for-each>
									<tr id="doc-table-dark">
										<th align="left" colspan="2">Scheduler Type</th>
									</tr>
									<tr>
										<td colspan="2">
											<xsl:value-of select="schedtype"/>
										</td>
									</tr>
									<tr id="doc-table-dark">
										<th align="left" colspan="2">Default Priority</th>
									</tr>
									<tr>
										<td colspan="2">
											<xsl:value-of select="def_prio"/>
										</td>
									</tr>
									<tr id="doc-table-dark">
										<th align="left" colspan="2">When Is It Called?</th>
									</tr>
									<tr>
										<td colspan="2">
											<xsl:value-of select="whencalled"/>
										</td>
									</tr>
									<tr id="doc-table-dark">
										<th align="left" colspan="2">Where Does It Live?</th>
									</tr>
									<tr>
										<td colspan="2">
											<xsl:value-of select="wherelives"/>
										</td>
									</tr>
									<tr id="doc-table-dark">
										<th align="left" colspan="2">To Define</th>
									</tr>
									<tr>
										<td colspan="2">
											<xsl:value-of select="todefine"/>
										</td>
									</tr>
									<tr id="doc-table-dark">
										<th align="left" colspan="2">Explanation</th>
									</tr>
									<xsl:for-each select="(explain | explain/code)">
										<tr>
											<td colspan="2">
												<xsl:choose>
													<xsl:when test="name() != 'code'">
														<xsl:value-of select="text()"/>
													</xsl:when>
													<xsl:otherwise>
														<pre>
															<xsl:value-of select="."/>
														</pre>
													</xsl:otherwise>
												</xsl:choose>
											</td>
										</tr>
									</xsl:for-each>
									<tr id="doc-table-dark">
										<th align="left" colspan="2">Examples</th>
									</tr>
									<xsl:for-each select="example">
										<tr>
											<td colspan="2">
												<pre>
													<xsl:value-of select="current()"/>
												</pre>
											</td>
										</tr>
									</xsl:for-each>
									<xsl:if test="relatedobj">
										<tr id="doc-table-dark">
											<th align="left" colspan="2">Related Objects</th>
										</tr>
										<xsl:for-each select="relatedobj">
											<tr>
												<td colspan="2">
												  <a>
												    <xsl:choose>
                              <xsl:when test="$offline=1">
                                <xsl:attribute name="href">objref.html#<xsl:value-of select="current()"/></xsl:attribute>
                              </xsl:when>
                              <xsl:otherwise>
                                <xsl:attribute name="href">objref.php#<xsl:value-of select="current()"/></xsl:attribute>
                              </xsl:otherwise>
                            </xsl:choose>
                            <b>
														  <xsl:value-of select="current()"/>
													  </b>
													</a>
												</td>
											</tr>
										</xsl:for-each>
									</xsl:if>
									<xsl:if test="relatedcfg">
										<tr id="doc-table-dark">
											<th align="left" colspan="2">Related Config Files</th>
										</tr>
										<xsl:for-each select="relatedcfg">
											<tr>
												<td colspan="2">
												  <a>
												    <xsl:choose>
                              <xsl:when test="$offline=1">
                                <xsl:attribute name="href">configfiles.html#<xsl:value-of select="current()"/></xsl:attribute>
                              </xsl:when>
                              <xsl:otherwise>
                                <xsl:attribute name="href">configfiles.php#<xsl:value-of select="current()"/></xsl:attribute>
                              </xsl:otherwise>
                            </xsl:choose>
                            <b>
														  <xsl:value-of select="current()"/>
													  </b>
													</a>
												</td>
											</tr>
										</xsl:for-each>
									</xsl:if>
									<xsl:if test="relatedtype">
										<tr id="doc-table-dark">
											<th align="left" colspan="2">Related Script Types</th>
										</tr>
										<xsl:for-each select="relatedtype">
											<tr>
												<td colspan="2">
												  <a>
												    <xsl:choose>
                              <xsl:when test="$offline=1">
                                <xsl:attribute name="href">scripttypes.html#<xsl:value-of select="current()"/></xsl:attribute>
                              </xsl:when>
                              <xsl:otherwise>
                                <xsl:attribute name="href">scripttypes.php#<xsl:value-of select="current()"/></xsl:attribute>
                              </xsl:otherwise>
                            </xsl:choose>
                            <b>
														  <xsl:value-of select="current()"/>
													  </b>
													</a>
												</td>
											</tr>
										</xsl:for-each>
									</xsl:if>
								</tbody>
							</table>
						</div>
					</xsl:for-each>
				</div>
			</div>
		</div>
	</xsl:template>
</xsl:stylesheet>

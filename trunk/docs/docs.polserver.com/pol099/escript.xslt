<?xml version="1.0" encoding="iso-8859-1"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="html" encoding="utf-8"/>
	<xsl:param name="offline" select="0"/>
	<xsl:param name="xmlfile"/>
	<xsl:template match="/">
		<xsl:for-each select="ESCRIPT/fileheader">
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
						File:
						<xsl:value-of select="@fname"/>
					</h1>
					<p>
						Description:
						<xsl:value-of select="filedesc"/>
						<br/>
						<b>
							Last Modified:
							<xsl:value-of select="datemodified"/>
						</b>
					</p>
				</div>
			</div>
			<xsl:if test="constant">
				<div class="container">
					<div class="doc-mainbox">
						<div class="expander">
							<a href="javascript:ExpandContract('Constants')" id="bullet_Constants">[-]</a>
						</div>
						<center>
							<h2>File Constants:</h2>
						</center>
						<span id="span_Constants">
							<pre>
								<xsl:for-each select="constant">
									<xsl:value-of select="current()"/>
									<xsl:if test="position() != last()">
										<br/>
									</xsl:if>
								</xsl:for-each>
							</pre>
						</span>
					</div>
				</div>
			</xsl:if>
		</xsl:for-each>
		<div id="main">
			<div class="container">
				<div class="doc-col_2">
					<div class="doc-smallbox">
						<div class="expander">
							<a href="javascript:ExpandContract('Functions')" id="bullet_Functions">[-]</a>
						</div>
						<center>
							<h2>Functions:</h2>
						</center>
						<span id="span_Functions">
							<ul>
								<xsl:for-each select="ESCRIPT/function">
									<xsl:sort select="prototype"/>
									<li>
										<a href="#{@name}">
											<xsl:value-of select="@name"/>
										</a>
									</li>
								</xsl:for-each>
							</ul>
						</span>
					</div>
					<div class="doc-smallbox-gold">
						<div class="expander">
							<a href="javascript:ExpandContract('Modules')" id="bullet_Modules">[-]</a>
						</div>
						<center>
							<h2>Modules:</h2>
						</center>
						<span id="span_Modules">
							<ul>
								<xsl:for-each select="document('modules.xml')//MODULES/em">
									<xsl:variable name="classname">
										<xsl:choose>
											<xsl:when test="$xmlfile=@name">treenodeopen</xsl:when>
											<xsl:otherwise>treenode</xsl:otherwise>
										</xsl:choose>
									</xsl:variable>
									<xsl:variable name="filename" select="@name"/>
									<li class="{$classname}">
										<a href="javascript:void(0)">[+] </a>
										<xsl:choose>
											<xsl:when test="$xmlfile=@name">
												<xsl:value-of select="@nice"/>
											</xsl:when>
											<xsl:otherwise>
												<a>
													<xsl:choose>
														<xsl:when test="$offline=1">
															<xsl:attribute name="href"><xsl:value-of select="@name"/>.html</xsl:attribute>
														</xsl:when>
														<xsl:otherwise>
															<xsl:attribute name="href">fullfunc.php?xmlfile=<xsl:value-of select="@name"/></xsl:attribute>
														</xsl:otherwise>
													</xsl:choose>
													<xsl:value-of select="@nice"/>
												</a>
											</xsl:otherwise>
										</xsl:choose>

										<ul>
											<xsl:for-each select="document(concat(@name,'.xml'))//ESCRIPT/function">
												<xsl:sort select="prototype"/>
												<xsl:choose>
													<xsl:when test="$xmlfile=$filename">
													<li><a href="#{@name}"><xsl:value-of select="@name"/></a></li>
												</xsl:when>
												<xsl:otherwise>
													<li><a>
													<xsl:choose>
														<xsl:when test="$offline=1">
															<xsl:attribute name="href"><xsl:value-of select="$filename"/>.html#<xsl:value-of select="@name"/></xsl:attribute>
														</xsl:when>
														<xsl:otherwise>
															<xsl:attribute name="href">fullfunc.php?xmlfile=<xsl:value-of select="$filename"/>#<xsl:value-of select="@name"/></xsl:attribute>
														</xsl:otherwise>
													</xsl:choose>
													<xsl:value-of select="@name"/>
												</a></li>
											</xsl:otherwise>
										</xsl:choose>
									</xsl:for-each>
								</ul>
							</li>
						</xsl:for-each>
					</ul>
				</span>
			</div>
		</div>
		<div class="doc-col_23">
			<xsl:for-each select="ESCRIPT/function">
				<xsl:sort select="prototype"/>
				<div class="doc-mainbox-gold">
					<div class="doc-anchor">
						<a href="#TOP">[^]</a>
					</div>
					<table class="doc-table" frame="void" rules="groups" width="100%" border="1" CELLPADDING="1" CELLSPACING="0">
						<tbody>
							<tr>
								<th align="center" colspan="2">
									<a name="{@name}">
										<xsl:value-of select="prototype"/>
									</a>
								</th>
							</tr>
						</tbody>
						<xsl:if test="parameter">
							<tbody>
								<tr id="doc-table-dark">
									<th align="left" colspan="2">Parameters:</th>
								</tr>
							</tbody>
							<tbody>
								<tr id="doc-table-med">
									<td>Name</td>
									<td>Type</td>
								</tr>
							</tbody>
							<tbody>
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
							</tbody>
						</xsl:if>
						<tbody>
							<tr id="doc-table-dark">
								<th align="left" colspan="2">Explanation</th>
							</tr>
						</tbody>
						<tbody>
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
						</tbody>
						<tbody>
							<tr id="doc-table-dark">
								<th align="left" colspan="2">Return values</th>
							</tr>
						</tbody>
						<tbody>
							<xsl:for-each select="return">
								<tr>
									<td colspan="2">
										<xsl:value-of select="current()"/>
									</td>
								</tr>
							</xsl:for-each>
						</tbody>
						<xsl:if test="error">
							<tbody>
								<tr id="doc-table-dark">
									<th align="left" colspan="2">Errors</th>
								</tr>
							</tbody>
							<tbody>
								<xsl:for-each select="error">
									<tr>
										<td colspan="2">
											<xsl:value-of select="current()"/>
										</td>
									</tr>
								</xsl:for-each>
							</tbody>
						</xsl:if>
						<xsl:if test="related">
							<tbody>
								<tr id="doc-table-dark">
									<th align="left" colspan="2">Related</th>
								</tr>
							</tbody>
							<tbody>
								<xsl:for-each select="related">
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
							</tbody>
						</xsl:if>
						<xsl:if test="relatedcfg">
							<tbody>
								<tr id="doc-table-dark">
									<th align="left" colspan="2">Related Configs</th>
								</tr>
							</tbody>
							<tbody>
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

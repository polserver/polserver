<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="html" encoding="utf-8"/>
	<xsl:param name="funcname"/>
	<xsl:param name="xmlfile"/>
	<xsl:template match="/">
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
									<xsl:choose>
										<xsl:when test="$funcname=@name">
											<li>
												<xsl:value-of select="@name"/>
											</li>
										</xsl:when>
										<xsl:otherwise>
											<li>
												<a href="singlefunc.php?funcname={@name}&amp;xmlfile={$xmlfile}"><xsl:value-of select="@name"/></a>
											</li>
										</xsl:otherwise>
									</xsl:choose>
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
  								    <xsl:when test="$xmlfile=concat(@name,'.xml')">treenodeopen</xsl:when>
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
              			    <a href="singlefunc.php?xmlfile={@name}.xml">
                          <xsl:value-of select="@nice"/>
              				  </a>
    									</xsl:otherwise>
    								</xsl:choose>
    								<ul>
  									  <xsl:for-each select="document(concat(@name,'.xml'))//ESCRIPT/function">
  									    <xsl:sort select="prototype"/>
									      <li><a href="singlefunc.php?funcname={@name}&amp;xmlfile={$filename}.xml"><xsl:value-of select="@name"/></a></li>
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
						<xsl:if test="$funcname=@name">
							<div class="doc-mainbox-gold">
								<div class="doc-home">
									<a href="index.php">Home</a>
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
										<xsl:if test="parameter">
											<tr class="doc-table-dark">
												<th align="left" colspan="2">Parameters:</th>
											</tr>
											<tr class="doc-table-med">
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
										<tr class="doc-table-dark">
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
										<tr class="doc-table-dark">
											<th align="left" colspan="2">Return values</th>
										</tr>
										<xsl:for-each select="return">
											<tr>
												<td colspan="2">
													<xsl:value-of select="current()"/>
												</td>
											</tr>
										</xsl:for-each>
										<xsl:if test="error">
											<tr class="doc-table-dark">
												<th align="left" colspan="2">Errors</th>
											</tr>
											<xsl:for-each select="error">
												<tr>
													<td colspan="2">
														<xsl:value-of select="current()"/>
													</td>
												</tr>
											</xsl:for-each>
										</xsl:if>
										<xsl:if test="related">
											<tr class="doc-table-dark">
												<th align="left" colspan="2">Related</th>
											</tr>
											<xsl:for-each select="related">
												<tr>
													<td colspan="2">
														<a href="objref.php#{current()}">
															<b>
																<xsl:value-of select="current()"/>
															</b>
														</a>
													</td>
												</tr>
											</xsl:for-each>
										</xsl:if>
										<xsl:if test="relatedcfg">
											<tr class="doc-table-dark">
												<th align="left" colspan="2">Related Configs</th>
											</tr>
											<xsl:for-each select="relatedcfg">
												<tr>
													<td colspan="2">
														<a href="configfiles.php#{current()}">
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
						</xsl:if>
					</xsl:for-each>
				</div>
			</div>
		</div>
	</xsl:template>
</xsl:stylesheet>

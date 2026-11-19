<?xml version="1.0" encoding="iso-8859-1"?>
<!--
	Module reference page (fullfunc.php?xmlfile=..., or <module>.html offline).
	The classes used here are styled by the polserver.com stylesheet; the filter, the
	"current function" highlight and the folding menus on narrow screens are in docs.js.
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="html" encoding="utf-8"/>
	<xsl:param name="offline" select="0"/>
	<xsl:param name="xmlfile"/>
	<!-- the class names of the object reference, as "|Name|Name|...|" (see objrefclasses()) -->
	<xsl:param name="classes"/>

	<xsl:variable name="ext">
		<xsl:choose>
			<xsl:when test="$offline=1">html</xsl:when>
			<xsl:otherwise>php</xsl:otherwise>
		</xsl:choose>
	</xsl:variable>

	<xsl:template match="/">
		<xsl:for-each select="ESCRIPT/fileheader">
			<div class="doc-head">
				<div class="crumbs">
					<a href="index.{$ext}">Documentation</a>
					<xsl:text> &#8250; Modules &#8250; </xsl:text>
					<xsl:value-of select="@fname"/>
				</div>
				<h1 id="TOP">
					<xsl:text>File: </xsl:text>
					<xsl:value-of select="@fname"/>
				</h1>
				<p>
					<xsl:value-of select="filedesc"/>
				</p>
				<div class="meta">
					<xsl:text>Last modified: </xsl:text>
					<xsl:value-of select="datemodified"/>
				</div>
			</div>
		</xsl:for-each>

		<div class="doc-layout">
			<div class="doc-side">
				<details class="smallbox doc-menu" open="open">
					<summary>
						<h1>
							<xsl:text>Functions</xsl:text>
							<span class="cur" id="fn-current"/>
						</h1>
					</summary>
					<ul class="fn-list">
						<li class="filter">
							<input type="search" id="fn-filter" placeholder="Filter functions" aria-label="Filter functions" autocomplete="off"/>
						</li>
						<xsl:for-each select="ESCRIPT/function">
							<xsl:sort select="@name"/>
							<li>
								<a href="#{@name}">
									<xsl:value-of select="@name"/>
								</a>
							</li>
						</xsl:for-each>
					</ul>
				</details>
				<details class="smallbox-gold doc-menu">
					<summary>
						<h1>Modules</h1>
					</summary>
					<ul class="mod-links">
						<xsl:for-each select="document('modules.xml')//MODULES/em">
							<li>
								<xsl:choose>
									<xsl:when test="$xmlfile=@name">
										<b>
											<xsl:value-of select="@nice"/>
										</b>
									</xsl:when>
									<xsl:otherwise>
										<a>
											<xsl:call-template name="module-href"/>
											<xsl:value-of select="@nice"/>
										</a>
									</xsl:otherwise>
								</xsl:choose>
							</li>
						</xsl:for-each>
					</ul>
				</details>
			</div>

			<div class="doc-main">
				<xsl:if test="ESCRIPT/fileheader/constant">
					<details class="mainbox doc-consts">
						<summary>
							<h1>File Constants</h1>
						</summary>
						<pre class="doc-code">
							<xsl:for-each select="ESCRIPT/fileheader/constant">
								<xsl:value-of select="."/>
								<xsl:if test="position() != last()">
									<xsl:text>&#10;</xsl:text>
								</xsl:if>
							</xsl:for-each>
						</pre>
					</details>
				</xsl:if>

				<xsl:for-each select="ESCRIPT/function">
					<xsl:sort select="@name"/>
					<div class="fn" id="{@name}">
						<h2 class="fn-sig">
							<xsl:choose>
								<xsl:when test="contains(prototype, '(')">
									<b>
										<xsl:value-of select="normalize-space(substring-before(prototype, '('))"/>
									</b>
									<xsl:text>(</xsl:text>
									<xsl:call-template name="sig-args">
										<xsl:with-param name="s" select="substring-after(prototype, '(')"/>
									</xsl:call-template>
								</xsl:when>
								<xsl:otherwise>
									<b>
										<xsl:value-of select="prototype"/>
									</b>
								</xsl:otherwise>
							</xsl:choose>
							<a class="perma" href="#{@name}" title="Link to this function">#</a>
						</h2>

						<xsl:if test="parameter">
							<div class="fn-row">
								<h3>Parameters</h3>
								<div>
									<table class="fn-params">
										<xsl:for-each select="parameter">
											<tr>
												<td>
													<var>
														<xsl:value-of select="@name"/>
													</var>
												</td>
												<td>
													<xsl:call-template name="type">
														<xsl:with-param name="t" select="@value"/>
													</xsl:call-template>
												</td>
											</tr>
										</xsl:for-each>
									</table>
								</div>
							</div>
						</xsl:if>

						<xsl:if test="explain">
							<div class="fn-row">
								<h3>Explanation</h3>
								<div>
									<xsl:apply-templates select="explain"/>
								</div>
							</div>
						</xsl:if>

						<xsl:if test="return">
							<div class="fn-row">
								<h3>Return values</h3>
								<div>
									<xsl:for-each select="return">
										<div class="p">
											<xsl:value-of select="."/>
										</div>
									</xsl:for-each>
								</div>
							</div>
						</xsl:if>

						<xsl:if test="error">
							<div class="fn-row">
								<h3>Errors</h3>
								<div>
									<ul class="inline">
										<xsl:for-each select="error">
											<li>
												<xsl:value-of select="."/>
											</li>
										</xsl:for-each>
									</ul>
								</div>
							</div>
						</xsl:if>

						<xsl:if test="related">
							<div class="fn-row">
								<h3>Related</h3>
								<div>
									<ul class="inline">
										<xsl:for-each select="related">
											<li>
												<a href="objref.{$ext}#{.}">
													<xsl:value-of select="."/>
												</a>
											</li>
										</xsl:for-each>
									</ul>
								</div>
							</div>
						</xsl:if>

						<xsl:if test="relatedcfg">
							<div class="fn-row">
								<h3>Related config</h3>
								<div>
									<ul class="inline">
										<xsl:for-each select="relatedcfg">
											<li>
												<a href="configfiles.{$ext}#{.}">
													<xsl:value-of select="."/>
												</a>
											</li>
										</xsl:for-each>
									</ul>
								</div>
							</div>
						</xsl:if>
					</div>
				</xsl:for-each>
			</div>
		</div>
	</xsl:template>

	<!-- href of a module page; the context node is an <em> of modules.xml -->
	<xsl:template name="module-href">
		<xsl:attribute name="href">
			<xsl:choose>
				<xsl:when test="$offline=1"><xsl:value-of select="@name"/>.html</xsl:when>
				<xsl:otherwise>fullfunc.php?xmlfile=<xsl:value-of select="@name"/></xsl:otherwise>
			</xsl:choose>
		</xsl:attribute>
	</xsl:template>

	<!-- One paragraph of the explanation. "Notes:", "Tip:" and "Warning:" paragraphs become
	     call-outs; <code> children become code blocks. -->
	<xsl:template match="explain">
		<xsl:variable name="first" select="normalize-space(text()[1])"/>
		<xsl:variable name="prefix">
			<xsl:choose>
				<xsl:when test="starts-with($first, 'Notes:')">Notes:</xsl:when>
				<xsl:when test="starts-with($first, 'Note:')">Note:</xsl:when>
				<xsl:when test="starts-with($first, 'NOTE:')">NOTE:</xsl:when>
				<xsl:when test="starts-with($first, 'Tip:')">Tip:</xsl:when>
				<xsl:when test="starts-with($first, 'Warning:')">Warning:</xsl:when>
			</xsl:choose>
		</xsl:variable>
		<div>
			<xsl:attribute name="class">
				<xsl:choose>
					<xsl:when test="$prefix != ''">p note</xsl:when>
					<xsl:otherwise>p</xsl:otherwise>
				</xsl:choose>
			</xsl:attribute>
			<xsl:if test="$prefix != ''">
				<b>
					<xsl:choose>
						<xsl:when test="$prefix = 'Tip:'">Tip</xsl:when>
						<xsl:when test="$prefix = 'Warning:'">Warning</xsl:when>
						<xsl:otherwise>Note</xsl:otherwise>
					</xsl:choose>
				</b>
			</xsl:if>
			<xsl:for-each select="node()">
				<xsl:choose>
					<xsl:when test="self::code">
						<pre class="doc-code">
							<xsl:value-of select="."/>
						</pre>
					</xsl:when>
					<xsl:when test="self::text() and position() = 1 and $prefix != ''">
						<xsl:value-of select="substring-after(., $prefix)"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select="."/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:for-each>
		</div>
	</xsl:template>

	<!-- A parameter type. "<Class> Reference" links to that class in the object reference
	     when the class exists there. -->
	<xsl:template name="type">
		<xsl:param name="t"/>
		<xsl:variable name="class" select="substring-before($t, ' Reference')"/>
		<xsl:choose>
			<xsl:when test="$class != '' and not(contains($class, ' ')) and contains($classes, concat('|', $class, '|'))">
				<a href="objref.{$ext}#{$class}">
					<xsl:value-of select="$class"/>
					<xsl:text> Reference</xsl:text>
				</a>
				<xsl:value-of select="substring-after($t, ' Reference')"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="$t"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<!-- The argument list of a prototype (everything after the opening parenthesis),
	     with default values marked up so they can be shown muted. -->
	<xsl:template name="sig-args">
		<xsl:param name="s"/>
		<xsl:choose>
			<xsl:when test="contains($s, ',')">
				<xsl:call-template name="sig-arg">
					<xsl:with-param name="a" select="substring-before($s, ',')"/>
				</xsl:call-template>
				<xsl:text>,</xsl:text>
				<xsl:call-template name="sig-args">
					<xsl:with-param name="s" select="substring-after($s, ',')"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:otherwise>
				<xsl:variable name="last" select="normalize-space($s)"/>
				<xsl:choose>
					<xsl:when test="$last = ')'">
						<xsl:text>)</xsl:text>
					</xsl:when>
					<xsl:when test="substring($last, string-length($last)) = ')'">
						<xsl:text> </xsl:text>
						<xsl:call-template name="sig-arg">
							<xsl:with-param name="a" select="normalize-space(substring($last, 1, string-length($last) - 1))"/>
						</xsl:call-template>
						<xsl:text> )</xsl:text>
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select="$s"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template name="sig-arg">
		<xsl:param name="a"/>
		<xsl:choose>
			<xsl:when test="contains($a, ':=')">
				<xsl:value-of select="substring-before($a, ':=')"/>
				<span class="dim">
					<xsl:text>:=</xsl:text>
					<xsl:value-of select="substring-after($a, ':=')"/>
				</span>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="$a"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
</xsl:stylesheet>

<?xml version="1.0" encoding="utf-8"?>

<!-- Copyright 2020 Ben Asselstine, this file is licensed under the terms of
the GNU General Public License version 3, or at your option any later version of the license. -->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="xml" version="1.0" encoding="utf-8"/>

<xsl:template match="@*|node()">
        <xsl:copy>
                <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
</xsl:template>

<xsl:template match='lordsawarrc/@version'>
        <xsl:attribute name='version'>0.4.0</xsl:attribute>
</xsl:template>

<xsl:template match="lordsawarrc">
	<xsl:copy>
      		<xsl:apply-templates select="@*|node()"/>
        	<d_cities_can_produce_allies>false</d_cities_can_produce_allies>
  	</xsl:copy>
</xsl:template>

<xsl:template match="d_autosave_policy[text()='Configuration::NO_SAVING']">
  <d_autosave_policy>Configuration::NO_AUTOSAVING</d_autosave_policy>
</xsl:template>

<xsl:template match="lordsawarrc/d_musiccache"/>
<xsl:template match="lordsawarrc/d_lang"/>
<xsl:template match="lordsawarrc/d_zipfiles"/>
<xsl:template match="lordsawarrc/d_decorated"/>
<xsl:template match="lordsawarrc/d_font_size_override"/>

</xsl:stylesheet>

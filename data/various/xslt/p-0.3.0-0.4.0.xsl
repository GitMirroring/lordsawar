<?xml version="1.0" encoding="utf-8"?>

<!-- Copyright 2026 Ben Asselstine, this file is licensed under the terms of
the GNU General Public License version 3, or at your option any later version of the license. -->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="xml" version="1.0" encoding="utf-8"/>

<xsl:template match="@*|node()">
        <xsl:copy>
                <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
</xsl:template>

<xsl:template match='profilelist/@version'>
        <xsl:attribute name='version'>0.4.0</xsl:attribute>
</xsl:template>

<xsl:template match="/*">
        <xsl:copy>
            <xsl:apply-templates select="@*|node()"/>
	    <xsl:text>&#9;</xsl:text>
            <d_default></d_default>
            <xsl:text>&#10;</xsl:text>
        </xsl:copy>
</xsl:template>

</xsl:stylesheet>

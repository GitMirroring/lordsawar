<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="xml" version="1.0" encoding="utf-8"/>

<xsl:template match="@*|node()">
        <xsl:copy>
                <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
</xsl:template>

<xsl:template match='lordsawar/@version'>
        <xsl:attribute name='version'>0.3.3</xsl:attribute>
</xsl:template>

<xsl:template match="city">
         <city>
                 <xsl:copy-of select="d_id"/>
                 <xsl:copy-of select="d_x"/>
                 <xsl:copy-of select="d_y"/>
                 <xsl:copy-of select="d_name"/>
                 <d_description></d_description>
                 <xsl:copy-of select="d_owner"/>
                 <xsl:copy-of select="d_defense"/>
                 <xsl:copy-of select="d_gold"/>
                 <xsl:copy-of select="d_burnt"/>
                 <xsl:copy-of select="d_build_production"/>
                 <xsl:copy-of select="d_capital"/>
                 <xsl:copy-of select="d_capital_owner"/>
                 <xsl:copy-of select="d_vectoring"/>
                 <xsl:copy-of select="d_active_production_slot"/>
                 <xsl:copy-of select="d_duration"/>
                 <xsl:copy-of select="slot"/>
         </city>
</xsl:template>

</xsl:stylesheet>

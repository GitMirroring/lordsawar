<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="xml" version="1.0" encoding="utf-8"/>

<xsl:template match="@*|node()">
        <xsl:copy>
                <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
</xsl:template>
<xsl:template match='armyset/@version'>
        <xsl:attribute name='version'>0.3.0</xsl:attribute>
</xsl:template>

<xsl:template match="armyproto">
         <armyproto>
                 <xsl:variable name="count"><xsl:number/></xsl:variable>
                 <d_id><xsl:number count="armyproto" format="1" value="$count - 1"/></d_id>
                 <xsl:copy-of select="d_name"/>
                 <xsl:copy-of select="d_image_white"/>
                 <xsl:copy-of select="d_image_green"/>
                 <xsl:copy-of select="d_image_yellow"/>
                 <xsl:copy-of select="d_image_light_blue"/>
                 <xsl:copy-of select="d_image_red"/>
                 <xsl:copy-of select="d_image_dark_blue"/>
                 <xsl:copy-of select="d_image_dark_blue"/>
                 <xsl:copy-of select="d_image_orange"/>
                 <xsl:copy-of select="d_image_black"/>
                 <xsl:copy-of select="d_image_neutral"/>
                 <xsl:copy-of select="d_description"/>
                 <xsl:copy-of select="d_production"/>
                 <xsl:copy-of select="d_new_production_cost"/>
                 <xsl:copy-of select="d_production_cost"/>
                 <xsl:copy-of select="d_upkeep"/>
                 <xsl:copy-of select="d_awardable"/>
                 <xsl:copy-of select="d_defends_ruins"/>
                 <xsl:copy-of select="d_move_bonus"/>
                 <xsl:copy-of select="d_army_bonus"/>
                 <xsl:copy-of select="d_max_moves"/>
                 <xsl:copy-of select="d_gender"/>
                 <xsl:copy-of select="d_strength"/>
                 <xsl:copy-of select="d_sight"/>
                 <xsl:copy-of select="d_expvalue"/>
         </armyproto>
</xsl:template>

</xsl:stylesheet>

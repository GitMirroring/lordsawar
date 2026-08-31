<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="xml" version="1.0" encoding="utf-8"/>

<xsl:template match="@*|node()">
        <xsl:copy>
                <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
</xsl:template>

<xsl:template match="tileset">
         <tileset version="0.3.2">
                 <xsl:copy-of select="d_id"/>
                 <xsl:copy-of select="d_name"/>
                 <xsl:copy-of select="d_copyright"/>
                 <xsl:copy-of select="d_license"/>
                 <xsl:copy-of select="d_info"/>
                 <xsl:copy-of select="d_tilesize"/>
                 <xsl:copy-of select="d_large_selector"/>
                 <xsl:copy-of select="d_small_selector"/>
                 <xsl:copy-of select="d_explosion"/>
                 <xsl:copy-of select="d_roads"/>
                 <d_standing_stones></d_standing_stones>
                 <xsl:copy-of select="d_bridges"/>
                 <xsl:copy-of select="d_fog"/>
                 <xsl:copy-of select="d_flags"/>
                 <xsl:copy-of select="road_smallmap"/>
                 <xsl:copy-of select="ruin_smallmap"/>
                 <xsl:copy-of select="temple_smallmap"/>
                 <xsl:copy-of select="tile"/>
         </tileset>
</xsl:template>

</xsl:stylesheet>

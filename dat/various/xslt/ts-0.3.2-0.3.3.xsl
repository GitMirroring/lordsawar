<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="xml" version="1.0" encoding="utf-8"/>

<xsl:template match="@*|node()">
        <xsl:copy>
                <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
</xsl:template>

<xsl:template match='tileset'>
        <tileset version="0.3.3">
        <xsl:copy-of select="d_id"/>
        <xsl:copy-of select="d_name"/>
        <xsl:copy-of select="d_copyright"/>
        <xsl:copy-of select="d_license"/>
        <xsl:copy-of select="d_info"/>
        <xsl:copy-of select="d_tilesize"/>
        <xsl:copy-of select="d_large_selector"/>
        <d_large_selector_num_masks>1</d_large_selector_num_masks>
        <xsl:copy-of select="d_small_selector"/>
        <d_small_selector_num_masks>1</d_small_selector_num_masks>
        <xsl:copy-of select="d_explosion"/>
        <xsl:copy-of select="d_roads"/>
        <xsl:copy-of select="d_standing_stones"/>
        <xsl:copy-of select="d_bridges"/>
        <xsl:copy-of select="d_fog"/>
        <xsl:copy-of select="d_flags"/>
        <d_flags_num_masks>1</d_flags_num_masks>
        <d_movebonus_all/>
        <d_movebonus_water/>
        <d_movebonus_forest/>
        <d_movebonus_hills/>
        <d_movebonus_swamp/>
        <d_movebonus_mountains/>
        <xsl:copy-of select="road_smallmap"/>
        <xsl:copy-of select="ruin_smallmap"/>
        <xsl:copy-of select="temple_smallmap"/>
        <xsl:copy-of select="tile"/>
        </tileset>
</xsl:template>

</xsl:stylesheet>

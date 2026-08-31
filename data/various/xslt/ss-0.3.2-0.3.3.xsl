<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="xml" version="1.0" encoding="utf-8"/>

<xsl:template match="@*|node()">
        <xsl:copy>
                <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
</xsl:template>

<xsl:template match='shieldset/@version'>
        <xsl:attribute name='version'>0.3.3</xsl:attribute>
</xsl:template>


<xsl:template match="shield">
         <shield>
                 <xsl:copy-of select="d_owner"/>
                 <xsl:copy-of select="d_color"/>
                 <xsl:for-each select="shieldstyle">
                 <shieldstyle>
                        <xsl:copy-of select="d_type"/>
                        <xsl:copy-of select="d_image"/>
                        <d_image_num_masks>1</d_image_num_masks>
                 </shieldstyle>
                 </xsl:for-each>
                 <xsl:copy-of select="d_left_tartan_image"/>
                 <xsl:copy-of select="d_center_tartan_image"/>
                 <xsl:copy-of select="d_right_tartan_image"/>
                 <d_left_tartan_image_num_masks>1</d_left_tartan_image_num_masks>
                 <d_center_tartan_image_num_masks>1</d_center_tartan_image_num_masks>
                 <d_right_tartan_image_num_masks>1</d_right_tartan_image_num_masks>
         </shield>
</xsl:template>

</xsl:stylesheet>

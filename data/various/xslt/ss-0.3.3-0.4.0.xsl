<?xml version="1.0" encoding="utf-8"?>
<!--
Copyright (C) 2026 Ben Asselstine
This file is licensed under the terms of the GNU GPL version 3 or later.
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="xml" version="1.0" encoding="utf-8"/>

<xsl:template match="@*|node()">
        <xsl:copy>
                <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
</xsl:template>

<xsl:template match='shieldset/@version'>
        <xsl:attribute name='version'>0.4.0</xsl:attribute>
</xsl:template>


<xsl:template match="d_small_width"/>
<xsl:template match="d_medium_width"/>
<xsl:template match="d_large_width"/>
<xsl:template match="d_small_height"/>
<xsl:template match="d_medium_height"/>
<xsl:template match="d_large_height"/>


<xsl:template match="d_image_num_masks"/>
<xsl:template match="d_left_image_num_masks"/>
<xsl:template match="d_center_image_num_masks"/>
<xsl:template match="d_right_image_num_masks"/>

</xsl:stylesheet>


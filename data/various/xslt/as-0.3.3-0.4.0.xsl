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

<xsl:template match='armyset/@version'>
        <xsl:attribute name='version'>0.4.0</xsl:attribute>
</xsl:template>


<xsl:template match="d_stackship_num_masks"/>
<xsl:template match="d_plantedstandard_num_masks"/>
<xsl:template match="d_white_small_selector_num_masks"/>
<xsl:template match="d_green_small_selector_num_masks"/>
<xsl:template match="d_yellow_small_selector_num_masks"/>
<xsl:template match="d_dark_blue_small_selector_num_masks"/>
<xsl:template match="d_orange_small_selector_num_masks"/>
<xsl:template match="d_light_blue_small_selector_num_masks"/>
<xsl:template match="d_red_small_selector_num_masks"/>
<xsl:template match="d_black_small_selector_num_masks"/>
<xsl:template match="d_white_large_selector_num_masks"/>
<xsl:template match="d_green_large_selector_num_masks"/>
<xsl:template match="d_yellow_large_selector_num_masks"/>
<xsl:template match="d_dark_blue_large_selector_num_masks"/>
<xsl:template match="d_orange_large_selector_num_masks"/>
<xsl:template match="d_light_blue_large_selector_num_masks"/>
<xsl:template match="d_red_large_selector_num_masks"/>
<xsl:template match="d_black_large_selector_num_masks"/>
<xsl:template match="d_image_white_num_masks"/>
<xsl:template match="d_image_green_num_masks"/>
<xsl:template match="d_image_yellow_num_masks"/>
<xsl:template match="d_image_light_blue_num_masks"/>
<xsl:template match="d_image_red_num_masks"/>
<xsl:template match="d_image_dark_blue_num_masks"/>
<xsl:template match="d_image_orange_num_masks"/>
<xsl:template match="d_image_black_num_masks"/>
<xsl:template match="d_image_neutral_num_masks"/>
<xsl:template match="d_description"/>
</xsl:stylesheet>

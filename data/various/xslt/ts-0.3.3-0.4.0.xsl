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

<xsl:template match='tileset/@version'>
        <xsl:attribute name='version'>0.4.0</xsl:attribute>
</xsl:template>

<xsl:template match="d_flags">
        <d_white_flags><xsl:apply-templates select="@*|node()"/></d_white_flags>
        <d_green_flags><xsl:apply-templates select="@*|node()"/></d_green_flags>
        <d_yellow_flags><xsl:apply-templates select="@*|node()"/></d_yellow_flags>
        <d_dark_blue_flags><xsl:apply-templates select="@*|node()"/></d_dark_blue_flags>
        <d_orange_flags><xsl:apply-templates select="@*|node()"/></d_orange_flags>
        <d_light_blue_flags><xsl:apply-templates select="@*|node()"/></d_light_blue_flags>
        <d_red_flags><xsl:apply-templates select="@*|node()"/></d_red_flags>
        <d_black_flags><xsl:apply-templates select="@*|node()"/></d_black_flags>
        <d_neutral_flags><xsl:apply-templates select="@*|node()"/></d_neutral_flags>
</xsl:template>

<xsl:template match="d_large_selector_num_masks"/>
<xsl:template match="d_small_selector_num_masks"/>
<xsl:template match="d_flags_num_masks"/>

</xsl:stylesheet>


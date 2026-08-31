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

<xsl:template match='lordsawar/@version'>
        <xsl:attribute name='version'>0.4.0</xsl:attribute>
</xsl:template>

<xsl:template match="scenario">
	<xsl:copy>
      		<xsl:apply-templates select="@*|node()"/>
        	<d_cities_can_produce_allies>false</d_cities_can_produce_allies>
  	</xsl:copy>
</xsl:template>

<xsl:template match="d_hero_newlevel_male_image_num_masks"/>
<xsl:template match="d_hero_newlevel_female_image_num_masks"/>
<xsl:template match="d_color"/>

    <xsl:template match="player/d_id[text()='0']">
        <xsl:copy>
            <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
        <d_shield>Shield::WHITE</d_shield>
    </xsl:template>
    <xsl:template match="player/d_id[text()='1']">
        <xsl:copy>
            <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
        <d_shield>Shield::GREEN</d_shield>
    </xsl:template>
    <xsl:template match="player/d_id[text()='2']">
        <xsl:copy>
            <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
        <d_shield>Shield::YELLOW</d_shield>
    </xsl:template>
    <xsl:template match="player/d_id[text()='3']">
        <xsl:copy>
            <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
        <d_shield>Shield::DARK_BLUE</d_shield>
    </xsl:template>
    <xsl:template match="player/d_id[text()='4']">
        <xsl:copy>
            <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
        <d_shield>Shield::ORANGE</d_shield>
    </xsl:template>
    <xsl:template match="player/d_id[text()='5']">
        <xsl:copy>
            <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
        <d_shield>Shield::LIGHT_BLUE</d_shield>
    </xsl:template>
    <xsl:template match="player/d_id[text()='6']">
        <xsl:copy>
            <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
        <d_shield>Shield::RED</d_shield>
    </xsl:template>
    <xsl:template match="player/d_id[text()='7']">
        <xsl:copy>
            <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
        <d_shield>Shield::BLACK</d_shield>
    </xsl:template>
    <xsl:template match="player/d_id[text()='8']">
        <xsl:copy>
            <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
        <d_shield>Shield::NEUTRAL</d_shield>
    </xsl:template>

</xsl:stylesheet>

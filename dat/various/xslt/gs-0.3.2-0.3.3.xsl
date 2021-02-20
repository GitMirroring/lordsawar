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

<xsl:template match="heroproto">
        <heroproto>
                <d_hero_id>0</d_hero_id>
                <xsl:copy-of select="d_id"/>
                <xsl:copy-of select="d_name"/>
                <xsl:copy-of select="d_description"/>
                <xsl:copy-of select="d_production_cost"/>
                <xsl:copy-of select="d_new_production_cost"/>
                <xsl:copy-of select="d_production"/>
                <xsl:copy-of select="d_upkeep"/>
                <xsl:copy-of select="d_move_bonus"/>
                <xsl:copy-of select="d_army_bonus"/>
                <xsl:copy-of select="d_max_moves"/>
                <xsl:copy-of select="d_sight"/>
                <xsl:copy-of select="d_expvalue"/>
                <xsl:copy-of select="d_awardable"/>
                <xsl:copy-of select="d_defends_ruins"/>
                <xsl:copy-of select="d_gender"/>
                <xsl:copy-of select="d_owner"/>
                <xsl:copy-of select="d_armyset"/>
                <xsl:copy-of select="d_image_white"/>
                <xsl:copy-of select="d_image_green"/>
                <xsl:copy-of select="d_image_yellow"/>
                <xsl:copy-of select="d_image_light_blue"/>
                <xsl:copy-of select="d_image_red"/>
                <xsl:copy-of select="d_image_dark_blue"/>
                <xsl:copy-of select="d_image_orange"/>
                <xsl:copy-of select="d_image_black"/>
                <xsl:copy-of select="d_image_neutral"/>
        </heroproto>
</xsl:template>

<xsl:template match="herotemplate">
        <herotemplate>
                <d_hero_id>0</d_hero_id>
                <d_description></d_description>
                <d_starting_items></d_starting_items>
                <herostrategy>
                        <d_type>HeroStrategy::NONE</d_type>
                </herostrategy>
                <xsl:copy-of select="d_owner"/>
                <xsl:copy-of select="d_gender"/>
                <xsl:copy-of select="d_name"/>
        </herotemplate>
</xsl:template>

<xsl:template match="hero">
        <hero>
                <d_hero_type>0</d_hero_type>
                <xsl:copy-of select="d_name"/>
                <xsl:copy-of select="d_gender"/>
                <xsl:copy-of select="d_upkeep"/>
                <xsl:copy-of select="d_move_bonus"/>
                <xsl:copy-of select="d_army_bonus"/>
                <xsl:copy-of select="d_max_moves"/>
                <xsl:copy-of select="d_strength"/>
                <xsl:copy-of select="d_sight"/>
                <xsl:copy-of select="d_expvalue"/>
                <xsl:copy-of select="d_id"/>
                <xsl:copy-of select="d_armyset"/>
                <xsl:copy-of select="d_type"/>
                <xsl:copy-of select="d_hp"/>
                <xsl:copy-of select="d_ship"/>
                <xsl:copy-of select="d_moves"/>
                <xsl:copy-of select="d_xp"/>
                <xsl:copy-of select="d_max_moves_multiplier"/>
                <xsl:copy-of select="d_level"/>
                <xsl:copy-of select="d_medals"/>
                <xsl:copy-of select="d_battlesnumber"/>
                <xsl:copy-of select="d_visited_temples"/>
                <xsl:copy-of select="backpack"/>
        </hero>
</xsl:template>

<xsl:template match="itemstack">
        <itemstack>
                <d_owner>8</d_owner>
                <xsl:copy-of select="d_x"/>
                <xsl:copy-of select="d_y"/>
                <xsl:copy-of select="backpack"/>
        </itemstack>
</xsl:template>

</xsl:stylesheet>

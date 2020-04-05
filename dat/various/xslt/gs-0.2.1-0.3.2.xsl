<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="xml" version="1.0" encoding="utf-8"/>

<xsl:template match="@*|node()">
        <xsl:copy>
                <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
</xsl:template>

<xsl:template match="scenario">
	 <scenario>
		<d_vectoring_mode>GameParameters::VECTORING_ALWAYS_TWO_TURNS</d_vectoring_mode>
		<d_build_production_mode>GameParameters::BUILD_PRODUCTION_ALWAYS</d_build_production_mode>
		<d_sacking_mode>GameParameters::SACKING_ALWAYS</d_sacking_mode>
                 <xsl:copy-of select="d_id"/>
                 <xsl:copy-of select="d_name"/>
                 <xsl:copy-of select="d_comment"/>
                 <xsl:copy-of select="d_copyright"/>
                 <xsl:copy-of select="d_license"/>
                 <xsl:copy-of select="d_turn"/>
                 <xsl:copy-of select="d_turnmode"/>
                 <xsl:copy-of select="d_view_enemies"/>
                 <xsl:copy-of select="d_view_production"/>
                 <xsl:copy-of select="d_quests"/>
                 <xsl:copy-of select="d_hidden_map"/>
                 <xsl:copy-of select="d_diplomacy"/>
                 <xsl:copy-of select="d_cusp_of_war"/>
                 <xsl:copy-of select="d_neutral_cities"/>
                 <xsl:copy-of select="d_razing_cities"/>
                 <xsl:copy-of select="d_intense_combat"/>
                 <xsl:copy-of select="d_military_advisor"/>
                 <xsl:copy-of select="d_random_turns"/>
                 <xsl:copy-of select="d_surrender_already_offered"/>
                 <xsl:copy-of select="d_playmode"/>
         </scenario>
</xsl:template>

<xsl:template match='lordsawar/@version'>
        <xsl:attribute name='version'>0.3.2</xsl:attribute>
</xsl:template>

<xsl:template match="city">
         <city>
                 <xsl:copy-of select="d_id"/>
                 <xsl:copy-of select="d_x"/>
                 <xsl:copy-of select="d_y"/>
                 <xsl:copy-of select="d_name"/>
                 <xsl:copy-of select="d_owner"/>
                 <xsl:copy-of select="d_defense"/>
                 <xsl:copy-of select="d_gold"/>
                 <xsl:copy-of select="d_burnt"/>
                 <d_build_production>true</d_build_production>
                 <xsl:copy-of select="d_capital"/>
                 <xsl:copy-of select="d_capital_owner"/>
                 <xsl:copy-of select="d_vectoring"/>
                 <xsl:copy-of select="d_active_production_slot"/>
                 <xsl:copy-of select="d_duration"/>
                 <xsl:copy-of select="slot"/>
         </city>
</xsl:template>

<xsl:template match="ruin">
         <ruin>
                 <xsl:copy-of select="d_id"/>
                 <xsl:copy-of select="d_x"/>
                 <xsl:copy-of select="d_y"/>
                 <xsl:copy-of select="d_name"/>
                 <xsl:copy-of select="d_description"/>
                 <xsl:copy-of select="d_type"/>
                 <xsl:copy-of select="d_searched"/>
                 <xsl:copy-of select="d_sage"/>
                 <xsl:copy-of select="d_hidden"/>
                 <xsl:copy-of select="d_owner"/>
                 <keeper>
                   <d_name>some defenders</d_name>
                 <xsl:copy-of select="stack"/>
                 <xsl:copy-of select="reward"/>
                 </keeper>
         </ruin>
</xsl:template>

</xsl:stylesheet>

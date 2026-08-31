<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="xml" version="1.0" encoding="utf-8"/>

<xsl:template match="@*|node()">
        <xsl:copy>
                <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
</xsl:template>
<xsl:template match='lordsawar/@version'>
        <xsl:attribute name='version'>0.2.1</xsl:attribute>
</xsl:template>

<xsl:template match="action">
        <xsl:choose>
                <xsl:when test="contains(d_type, 'Action::USE_ITEM')">
                        <action>
                        <d_type><xsl:value-of select="d_type"/></d_type>
                        <d_hero><xsl:value-of select="d_hero"/></d_hero>
                        <d_item><xsl:value-of select="d_item"/></d_item>
                        <d_victim_player><xsl:value-of select="d_victim_player"/></d_victim_player>
                        <d_friendly_city>0</d_friendly_city>
                        <d_enemy_city>0</d_enemy_city>
                        <d_neutral_city>0</d_neutral_city>
                        <d_city>0</d_city>
                        </action>
                </xsl:when>
                <xsl:otherwise>
                        <xsl:copy-of select="."/>
                </xsl:otherwise>
        </xsl:choose>
</xsl:template>

<xsl:template match="history">
        <xsl:choose>
                <xsl:when test="contains(d_type, 'History::USE_ITEM')">
                <history>
                        <d_type><xsl:value-of select="d_type"/></d_type>
                        <d_hero_name><xsl:value-of select="d_hero_name"/></d_hero_name>
                        <d_item_name><xsl:value-of select="d_item_name"/></d_item_name>
                        <d_item_bonus><xsl:value-of select="d_item_bonus"/></d_item_bonus>
                        <d_opponent_id><xsl:value-of select="d_opponent_id"/></d_opponent_id>
                        <d_friendly_city_id>0</d_friendly_city_id>
                        <d_enemy_city_id>0</d_enemy_city_id>
                        <d_neutral_city_id>0</d_neutral_city_id>
                        <d_city_id>0</d_city_id>
                </history>
                </xsl:when>
                <xsl:otherwise>
                        <xsl:copy-of select="."/>
                </xsl:otherwise>
        </xsl:choose>
</xsl:template>

<xsl:template match="itemproto">
        <itemproto> 
        <xsl:copy-of select="d_name"/>
        <d_bonus><xsl:value-of select="d_bonus"/></d_bonus>
        <xsl:if test="
                contains(d_bonus, 'ItemProto::STEAL_GOLD') or
                contains(d_bonus, 'ItemProto::BANISH_WORMS') or
                contains(d_bonus, 'ItemProto::SINK_SHIPS') or
                contains(d_bonus, 'ItemProto::PICK_UP_BAGS') or
                contains(d_bonus, 'ItemProto::ADD_2MP_STACK')">
                <d_uses_left><xsl:value-of select="d_uses_left"/></d_uses_left>
        </xsl:if>
        <xsl:if test="contains(d_bonus, 'ItemProto::STEAL_GOLD')">
                <d_steal_gold_percent>50.0</d_steal_gold_percent>
        </xsl:if>
        <xsl:if test="contains(d_bonus, 'ItemProto::BANISH_WORMS')">
                <d_kill_army_type>18</d_kill_army_type>
        </xsl:if>
        <xsl:if test="contains(d_bonus, 'ItemProto::ADD_2MP_STACK')">
                <d_mp_to_add>2</d_mp_to_add>
        </xsl:if>
        </itemproto>
</xsl:template>

<xsl:template match="item">
        <item>
        <xsl:copy-of select="d_name"/>
        <d_plantable><xsl:value-of select="d_plantable"/></d_plantable>
        <d_id><xsl:value-of select="d_id"/></d_id>
        <d_type><xsl:value-of select="d_type"/></d_type>
        <xsl:if test="
                contains(d_bonus, 'ItemProto::STEAL_GOLD') or
                contains(d_bonus, 'ItemProto::BANISH_WORMS') or
                contains(d_bonus, 'ItemProto::SINK_SHIPS') or
                contains(d_bonus, 'ItemProto::PICK_UP_BAGS') or
                contains(d_bonus, 'ItemProto::ADD_2MP_STACK')">
                <d_uses_left><xsl:value-of select="d_uses_left"/></d_uses_left>
        </xsl:if>
        <d_bonus><xsl:value-of select="d_bonus"/></d_bonus>
        <xsl:if test="contains(d_bonus, 'ItemProto::STEAL_GOLD')">
                <d_steal_gold_percent>50.0</d_steal_gold_percent>
        </xsl:if>
        <xsl:if test="contains(d_bonus, 'ItemProto::BANISH_WORMS')">
                <d_kill_army_type>18</d_kill_army_type>
        </xsl:if>
        <xsl:if test="contains(d_bonus, 'ItemProto::ADD_2MP_STACK')">
                <d_mp_to_add>2</d_mp_to_add>
        </xsl:if>
        </item>
</xsl:template>

</xsl:stylesheet>

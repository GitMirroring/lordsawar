<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="xml" version="1.0" encoding="utf-8"/>

<xsl:template match="@*|node()">
        <xsl:copy>
                <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
</xsl:template>
<xsl:template match='armyset'>
        <armyset version="0.3.3">
                 <xsl:copy-of select="d_id"/>
                 <xsl:copy-of select="d_name"/>
                 <xsl:copy-of select="d_copyright"/>
                 <xsl:copy-of select="d_license"/>
                 <xsl:copy-of select="d_info"/>
                 <xsl:copy-of select="d_tilesize"/>
                 <xsl:copy-of select="d_stackship"/>
                 <d_stackship_num_masks>1</d_stackship_num_masks>
                 <xsl:copy-of select="d_plantedstandard"/>
                 <d_plantedstandard_num_masks>1</d_plantedstandard_num_masks>
                 <xsl:copy-of select="d_bag"/>
                 <d_white_small_selector></d_white_small_selector>
                 <d_white_small_selector_num_masks>0</d_white_small_selector_num_masks>
                 <d_green_small_selector></d_green_small_selector>
                 <d_green_small_selector_num_masks>0</d_green_small_selector_num_masks>
                 <d_yellow_small_selector></d_yellow_small_selector>
                 <d_yellow_small_selector_num_masks>0</d_yellow_small_selector_num_masks>
                 <d_dark_blue_small_selector></d_dark_blue_small_selector>
                 <d_dark_blue_small_selector_num_masks>0</d_dark_blue_small_selector_num_masks>
                 <d_orange_small_selector></d_orange_small_selector>
                 <d_orange_small_selector_num_masks>0</d_orange_small_selector_num_masks>
                 <d_light_blue_small_selector></d_light_blue_small_selector>
                 <d_light_blue_small_selector_num_masks>0</d_light_blue_small_selector_num_masks>
                 <d_red_small_selector></d_red_small_selector>
                 <d_red_small_selector_num_masks>0</d_red_small_selector_num_masks>
                 <d_black_small_selector></d_black_small_selector>
                 <d_black_small_selector_num_masks>0</d_black_small_selector_num_masks>
                 <d_white_large_selector></d_white_large_selector>
                 <d_white_large_selector_num_masks>0</d_white_large_selector_num_masks>
                 <d_green_large_selector></d_green_large_selector>
                 <d_green_large_selector_num_masks>0</d_green_large_selector_num_masks>
                 <d_yellow_large_selector></d_yellow_large_selector>
                 <d_yellow_large_selector_num_masks>0</d_yellow_large_selector_num_masks>
                 <d_dark_blue_large_selector></d_dark_blue_large_selector>
                 <d_dark_blue_large_selector_num_masks>0</d_dark_blue_large_selector_num_masks>
                 <d_orange_large_selector></d_orange_large_selector>
                 <d_orange_large_selector_num_masks>0</d_orange_large_selector_num_masks>
                 <d_light_blue_large_selector></d_light_blue_large_selector>
                 <d_light_blue_large_selector_num_masks>0</d_light_blue_large_selector_num_masks>
                 <d_red_large_selector></d_red_large_selector>
                 <d_red_large_selector_num_masks>0</d_red_large_selector_num_masks>
                 <d_black_large_selector></d_black_large_selector>
                 <d_black_large_selector_num_masks>0</d_black_large_selector_num_masks>
                 <xsl:for-each select="armyproto">
                 <armyproto>
                        <xsl:copy-of select="d_id"/>
                        <xsl:copy-of select="d_name"/>
                        <xsl:copy-of select="d_image_white"/>
                        <d_image_white_num_masks>1</d_image_white_num_masks>
                        <xsl:copy-of select="d_image_green"/>
                        <d_image_green_num_masks>1</d_image_green_num_masks>
                        <xsl:copy-of select="d_image_yellow"/>
                        <d_image_yellow_num_masks>1</d_image_yellow_num_masks>
                        <xsl:copy-of select="d_image_light_blue"/>
                        <d_image_light_blue_num_masks>1</d_image_light_blue_num_masks>
                        <xsl:copy-of select="d_image_red"/>
                        <d_image_red_num_masks>1</d_image_red_num_masks>
                        <xsl:copy-of select="d_image_dark_blue"/>
                        <d_image_dark_blue_num_masks>1</d_image_dark_blue_num_masks>
                        <xsl:copy-of select="d_image_orange"/>
                        <d_image_orange_num_masks>1</d_image_orange_num_masks>
                        <xsl:copy-of select="d_image_black"/>
                        <d_image_black_num_masks>1</d_image_black_num_masks>
                        <xsl:copy-of select="d_image_neutral"/>
                        <d_image_neutral_num_masks>1</d_image_neutral_num_masks>
                        <xsl:copy-of select="d_description"/>
                        <xsl:copy-of select="d_production"/>
                        <xsl:copy-of select="d_new_production_cost"/>
                        <xsl:copy-of select="d_production_cost"/>
                        <xsl:copy-of select="d_upkeep"/>
                        <xsl:copy-of select="d_awardable"/>
                        <xsl:copy-of select="d_defends_ruins"/>
                        <xsl:copy-of select="d_move_bonus"/>
                        <xsl:copy-of select="d_army_bonus"/>
                        <xsl:copy-of select="d_max_moves"/>
                        <xsl:copy-of select="d_gender"/>
                        <xsl:copy-of select="d_strength"/>
                        <xsl:copy-of select="d_sight"/>
                        <xsl:copy-of select="d_expvalue"/>
                 </armyproto>
                 </xsl:for-each>
        </armyset>
</xsl:template>

</xsl:stylesheet>

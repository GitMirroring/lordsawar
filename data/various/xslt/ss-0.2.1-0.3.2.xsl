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
        <xsl:attribute name='version'>0.3.2</xsl:attribute>
</xsl:template>

<xsl:template match="shield">
         <shield>
                 <xsl:copy-of select="d_owner"/>
                 <xsl:copy-of select="d_color"/>
                 <xsl:copy-of select="shieldstyle"/>
                 <d_left_tartan_image>left-tartan-image</d_left_tartan_image>
                 <d_center_tartan_image>center-tartan-image</d_center_tartan_image>
                 <d_right_tartan_image>right-tartan-image</d_right_tartan_image>
         </shield>
</xsl:template>

</xsl:stylesheet>

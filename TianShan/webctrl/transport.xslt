<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="html" version="4.0" encoding="utf-8"/>
  <xsl:variable name="empty-line">
    <div style="height:12px"></div>
  </xsl:variable>
  <xsl:template match="/">
    <html>
      <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
      <header>
        <title>Transfports</title>
        <style type="text/css">
          table, th, td { border:gray 1px solid; text-align:left; }
          th { background-color:#cecf9c; }
          td { background-color:#e7e7ce; }
          h3
          {
	      FONT-SIZE: 120%;
	      FONT-WEIGHT: bold;
	      MARGIN-BOTTOM: 3px;
	      MARGIN-TOP: 20px;
	      PADDING-LEFT: 0px;
          }
          span.type { font-style:italic }
        </style>
      </header>
      <body>
        <xsl:for-each select="TianShanTransport/ServiceGroups">
          <xsl:call-template name="servicegroup"/>
        </xsl:for-each>
        <xsl:for-each select="TianShanTransport/Storages">
          <xsl:call-template name="storage"/>
        </xsl:for-each>
        <xsl:for-each select="TianShanTransport/Streamers">
          <xsl:call-template name="streamer"/>
        </xsl:for-each>
        <h3>StorageLinks</h3>
        <xsl:call-template name="storagelink">
          <xsl:with-param name="links" select="TianShanTransport/StorageLinks/Link"/>
        </xsl:call-template>

        <h3>StreamLinks</h3>
        <xsl:call-template name="streamlink">
          <xsl:with-param name="links" select="TianShanTransport/StreamLinks/Link"/>
        </xsl:call-template>
      </body>
    </html>
  </xsl:template>
  <xsl:template name="servicegroup">
    <h3>ServiceGroups</h3>
    <table>
      <tr><th>id</th><th>type</th><th>desc</th></tr>
      <xsl:for-each select="Group">
        <tr>
          <td><xsl:value-of select="@id"/></td>
          <td>ServiceGroup</td>
          <td><xsl:value-of select="@desc"/></td>
        </tr>
      </xsl:for-each>
    </table>
  </xsl:template>

  <xsl:template name="storage">
    <h3>Storages</h3>
    <table>
      <tr><th>netid</th><th>type</th><th>ifep</th><th>desc</th></tr>
      <xsl:for-each select="Storage">
        <tr>
          <td><xsl:value-of select="@netid"/></td>
          <td><xsl:value-of select="@type"/></td>
          <td><xsl:value-of select="@ifep"/></td>
          <td><xsl:value-of select="@desc"/></td>
        </tr>
      </xsl:for-each>
    </table>
  </xsl:template>

  <xsl:template name="streamer">
    <h3>Streamers</h3>
    <table>
      <tr><th>netid</th><th>type</th><th>ifep</th><th>desc</th></tr>
      <xsl:for-each select="Streamer">
        <tr>
          <td><xsl:value-of select="@netid"/></td>
          <td><xsl:value-of select="@type"/></td>
          <td><xsl:value-of select="@ifep"/></td>
          <td><xsl:value-of select="@desc"/></td>
        </tr>
      </xsl:for-each>
    </table>
  </xsl:template>

  <xsl:template name="storagelink">
    <xsl:param name="links"/>
    <xsl:if test="count($links) != 0">
      <xsl:variable name="type" select="$links[1]/@type"/>
      <xsl:copy-of select="$empty-line"/>
      <span class="type"><xsl:value-of select="$type"/></span>
      <table>
        <tr><th>id</th><th>storage</th><th>streamer</th>
          <xsl:for-each select="$links[1]/PrivateData">
            <xsl:call-template name="pd-header"/>
          </xsl:for-each>
        </tr>
        <xsl:for-each select="$links[@type = $type]">
          <xsl:sort select="@streamerId"/>
          <tr>
            <td><xsl:value-of select="@linkId"/></td>
            <td><xsl:value-of select="@storageId"/></td>
            <td><xsl:value-of select="@streamerId"/></td>
            <xsl:for-each select="PrivateData">
              <xsl:call-template name="pd-content"/>
            </xsl:for-each>
          </tr>
        </xsl:for-each>
      </table>
      <xsl:call-template name="storagelink">
        <xsl:with-param name="links" select="$links[@type != $type]"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template name="streamlink">
    <xsl:param name="links"/>
    <xsl:if test="count($links) != 0">
      <xsl:variable name="type" select="$links[1]/@type"/>
      <xsl:copy-of select="$empty-line"/>
      <span class="type"><xsl:value-of select="$type"/></span>
      <table>
        <tr><th>id</th><th>servicegroup</th><th>streamer</th>
          <xsl:for-each select="$links[1]/PrivateData">
            <xsl:call-template name="pd-header"/>
          </xsl:for-each>
        </tr>
        <xsl:for-each select="$links[@type = $type]">
          <xsl:sort select="@servicegroupId"/>
          <tr>
            <td><xsl:value-of select="@linkId"/></td>
            <td><xsl:value-of select="@servicegroupId"/></td>
            <td><xsl:value-of select="@streamerId"/></td>
            <xsl:for-each select="PrivateData">
              <xsl:call-template name="pd-content"/>
            </xsl:for-each>
          </tr>
        </xsl:for-each>
      </table>
      <xsl:call-template name="streamlink">
        <xsl:with-param name="links" select="$links[@type != $type]"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template name="pd-header">
    <xsl:for-each select="Data">
      <xsl:sort select="@key"/>
      <th><xsl:value-of select="@key"/></th>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="pd-content">
    <xsl:for-each select="Data">
      <xsl:sort select="@key"/>
      <td>
      <xsl:for-each select="Item">
        <xsl:if test="position() != 1">
          <xsl:text>,</xsl:text>
        </xsl:if>
        <xsl:value-of select="@value"/>
      </xsl:for-each>
      </td>
    </xsl:for-each>
  </xsl:template>
</xsl:stylesheet>

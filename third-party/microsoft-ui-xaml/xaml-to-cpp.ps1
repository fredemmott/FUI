# Copyright 2024 Fred Emmott <fred@fredemmott.com>
# SPDX-License-Identifier: MIT
param($InputFile, $OutputFile, [switch]$Macros, [switch]$Enums, [switch]$Themes, [switch]$Types)

$XMLNS = @{
  p = "http://schemas.microsoft.com/winfx/2006/xaml/presentation";
  x = "http://schemas.microsoft.com/winfx/2006/xaml";
}
$ThemesXML = Select-Xml `
  -Path $InputFile `
  -Namespace $XMLNS `
  -XPath "/p:ResourceDictionary/p:ResourceDictionary.ThemeDictionaries/p:ResourceDictionary"

function Get-Key($Color)
{
  return $Color.GetAttribute("Key", $XMLNS.x)
}

function Get-Color($Node)
{
  $Key = Get-Key($Node)
  $value = $Node.InnerText -replace '#'
  if ($value.Length -eq 6)
  {
    # RGB -> ARGB
    $value = "ff${value}"
  }
  $value = "0x${value}"

  return @{
    Key = $Key;
    Value = $value;
  }
}

function Resolve-Color($Value, $Colors)
{
  switch -regex ($Value)
  {
    'Transparent' {
      return 'SK_ColorTRANSPARENT'
    }
    '#FF00FF' {
      # Other hex values are possible, but not actually used. This seems to be a test color
      return 'SK_ColorMAGENTA'
    }
    '{ThemeResource System(Accent)?Color.*' {
      return "SystemTheme::$( $Value -replace '.* (System.+)}', '$1' )"
    }
    '{StaticResource .*Color.*}' {
      $Name = $Value -replace '.* ([A-Z][^ ]+)}$', '$1'
      $Color = $Colors | Where-Object -Property Key -eq $Name
      return $Color.Value
    }
    default {
      Write-Host "ERROR: Can't figure out how to parse ${Value} in SolidColorBrush"
      exit -1
    }
  }
}

function Get-SolidColorBrush($Colors, $Brush)
{
  $Key = (Get-Key $Brush)
  $Lookup = $Brush.GetAttribute('Color')
  $Value = Resolve-Color $Lookup $Colors
  return @{
    Key = $Key;
    Value = "SolidColorBrush { ${Value} }";
  }
}

function Get-LinearGradientBrush($Colors, $Brush)
{
  $Key = (Get-Key $Brush)
  $Mode = $Brush.GetAttribute('MappingMode')
  $Start = $Brush.GetAttribute('StartPoint')
  $End = $Brush.GetAttribute('EndPoint')
  $Stops = @()

  $XmlStops = Select-Xml -Xml $Brush `
    -Namespace $XMLNS `
    -XPath "p:LinearGradientBrush.GradientStops/p:GradientStop"
  foreach ($Stop in $XmlStops)
  {
    $Node = $Stop.Node;
    $Offset = $Node.GetAttribute('Offset')
    $Color = Resolve-Color $Node.GetAttribute('Color') $Colors
    $Stops += "{$Offset, $Color }"
  }

  $RelativeScale = ''
  $TransformQuery = Select-Xml -Xml $Brush `
    -Namespace $XMLNS `
    -XPath "p:LinearGradientBrush.RelativeTransform/*"
  if ($TransformQuery)
  {
    $N = $TransformQuery.Node
    if ($N.LocalName -ne 'ScaleTransform')
    {
      Write-Host "Unhandled LinearGradient transform: $( $N.LocalName )"
      exit(-1)
    }
    $scaleX = $N.HasAttribute('ScaleX') ? $N.GetAttribute('ScaleX') : 1
    $scaleY = $N.HasAttribute('ScaleY') ? $N.GetAttribute('ScaleY') : 1
    $originX = $N.HasAttribute('CenterX') ? $N.GetAttribute('CenterX') : 0
    $originY = $N.HasAttribute('CenterY') ? $N.GetAttribute('CenterY') : 0
    $RelativeScale = "`n  /* scale = */ { .mOrigin = { $originX, $originY }, .mScaleX = $scaleX, .mScaleY = $scaleY }"
  }

  return @{
    Key = $Key;
    Value = @"
LinearGradientBrush {
  LinearGradientBrush::MappingMode::$Mode,
  /* start = */ SkPoint { $Start },
  /* end = */ SkPoint { $End },
  /* stops = */ { $( $Stops -join ', ' ) },$RelativeScale
}
"@
  }
}

function Get-Theme($Theme)
{
  $Colors = $Theme.Color | Sort-Object -Property { Get-Key($_) } | ForEach-Object { Get-Color($_) }
  $SolidColorBrushes = $Theme.SolidColorBrush | ForEach-Object { Get-SolidColorBrush $Colors $_ }
  if ($Theme.LinearGradientBrush)
  {
    $LinearGradientBrushes = $Theme.LinearGradientBrush | ForEach-Object { Get-LinearGradientBrush $Colors $_ }
  }
  $Brushes = ($SolidColorBrushes + $LinearGradientBrushes) | Sort-Object -Property Key
  return @{
    Name = $Theme.Key;
    Colors = $Colors;
    Brushes = $Brushes;
  }
}

$ThemeData = $ThemesXML | ForEach-Object { Get-Theme $_.Node }
$ColorKeys = $ThemeData[0].Colors | ForEach-Object { $_.Key } | Sort-Object | Get-Unique
$BrushKeys = $ThemeData[0].Brushes | ForEach-Object { $_.Key } | Sort-Object | Get-Unique

$CppNs = "FredEmmott::GUI::gui_detail::WinUI3Themes"

function Get-Enums-Cpp()
{
  return @"
  namespace ${CppNs} {

enum class Colors {$(
  $ColorKeys.foreach({ "`n  $( $PSItem )," }) )
  };

enum class Brushes {$(
  $BrushKeys.foreach({ "`n  $( $PSItem )," }) )
  };

  }
"@
}

function Get-Macros-Cpp()
{
  return @"
  #define FUI_WINUI_THEME_COLORS(X) $(
  $ColorKeys.foreach({ "\`n  X($PSItem)" }) )
  #define FUI_WINUI_THEME_BRUSHES(X) $(
  $BrushKeys.foreach({ "\`n  X($PSItem)" }) )
"@
}

function Get-Theme-Cpp($Theme)
{
  return @"
const Theme $( $Theme.Name )Theme {$(
  $Theme.Colors.foreach({
    "`n  .m$( $PSItem.Key ) = $( $PSItem.Value ),"
  })
  $Theme.Brushes.foreach({
    "`n  .m$( $PSItem.Key ) = $( $PSItem.Value ),"
  })
  )
  }; `n
"@
}

function Get-Types-Cpp()
{

  return @"
  namespace $CppNs {

struct Theme {$(
  $ColorKeys.foreach({ "`n  SkColor m$PSItem;" })
  $BrushKeys.foreach({ "`n  Brush m$PSItem;" })
  )
  };

  }
"@
}

function Get-Themes-Cpp()
{
  @"
  namespace $CppNs {

$( ($ThemeData | ForEach-Object { Get-Theme-Cpp $_ }) -join "`n" )
  }
"@
}

$Content = @"
// $( '@' )generated by: $( $( Get-Item $PSCommandPath ).Name ) $( $( Get-Item ${InputFile} ).Name )
#pragma once

$( if ($Themes)
{
  Write-Output @"
#include <skia/Core/SkColor.h>
#include <FredEmmott/GUI/Brush.hpp>
#include <FredEmmott/GUI/LinearGradientBrush.hpp>
#include <FredEmmott/GUI/SystemTheme.hpp>
#include "Types.hpp"
"@
} )

$( if ($Macros)
{
  Get-Macros-Cpp
} )

$( if ($Enums)
{
  Get-Enums-Cpp
} )

$( if ($Types)
{
  Get-Types-Cpp
} )

$( if ($Themes)
{
  Get-Themes-Cpp
} )
"@

($content -split "`r" -join "") `
  | Set-Content -Encoding utf8 "$OutputFile" -NoNewline

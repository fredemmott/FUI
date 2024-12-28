# Copyright 2024 Fred Emmott <fred@fredemmott.com>
# SPDX-License-Identifier: MIT
param(
  $Component,
  $InputFile,
  $OutputFile,
  [ValidateSet("Cpp", "Hpp")][string]$Mode)

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
      return $Name;
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
StaticThemedLinearGradientBrush {
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
  $colors = @{ }
  foreach ($xml in $Theme.Color | Sort-Object -Property { Get-Key($_) })
  {
    $Color = Get-Color($xml)
    $colors[$Color.Key] = $Color
  }

  $SolidColorBrushes = $Theme.SolidColorBrush | ForEach-Object { Get-SolidColorBrush $Colors $_ }
  if ($Theme.LinearGradientBrush)
  {
    $LinearGradientBrushes = $Theme.LinearGradientBrush | ForEach-Object { Get-LinearGradientBrush $Colors $_ }
  }
  $brushes = @{ }
  foreach ($Brush in ($SolidColorBrushes + $LinearGradientBrushes) | Sort-Object -Property Key)
  {
    $brushes[$Brush.Key] = $Brush;
  }
  return @{
    Name = $Theme.Key;
    Colors = $colors;
    Brushes = $brushes;
  }
}

$ThemeData = $ThemesXML | ForEach-Object { Get-Theme $_.Node }
$ColorKeys = $ThemeData[0].Colors.Keys
$BrushKeys = $ThemeData[0].Brushes.Keys
$ThemesByName = @{ }
foreach ($theme in $ThemeData)
{
  $ThemesByName[$theme.Name] = $Theme
}
$DefaultTheme = $ThemesByName.Default
$LightTheme = $ThemesByName.Light
$HighContrastTheme = $ThemesByName.HighContrast


$CppNs = "FredEmmott::GUI::StaticTheme::inline $Component"

function Get-Hpp()
{
  @"
#pragma once
#include <FredEmmott/GUI/Brush.hpp>
#include <FredEmmott/GUI/Color.hpp>
#include <FredEmmott/GUI/StaticTheme/Resource.hpp>

namespace $CppNs {

$( ($ColorKeys | ForEach-Object { "extern const Resource<Color>* $_;" }) -join "`n" )

  $( ($BrushKeys | ForEach-Object { "extern const Resource<Brush>* $_;" }) -join "`n" )

}
"@
}

function Get-Color-Cpp($Key)
{
  @"
const Resource<Color> g$Key {
  .mDefault = SkColor { $( $DefaultTheme.Colors[$Key].Value ) },
  .mLight = SkColor { $( $LightTheme.Colors[$Key].Value ) },
  .mHighContrast = SkColor { $( $HighContrastTheme.Colors[$Key].Value ) },
};
const Resource<Color>* $Key = &g$Key;
"@
}

function Get-Brush-Cpp($Key)
{
  @"
const Resource<Brush> g$Key {
  .mDefault = ResolveBrush<Theme::Dark>($( $DefaultTheme.Brushes[$Key].Value )),
  .mLight = ResolveBrush<Theme::Light>($( $LightTheme.Brushes[$Key].Value )),
  .mHighContrast = ResolveBrush<Theme::HighContrast>($( $HighContrastTheme.Brushes[$Key].Value )),
};
const Resource<Brush>* $Key = &g$Key;
"@
}

function Get-Cpp()
{
  @"
#include "$Component.hpp"

#include <skia/core/SkColor.h>
#include <FredEmmott/GUI/Brush.hpp>
#include <FredEmmott/GUI/SystemTheme.hpp>
#include <FredEmmott/GUI/StaticTheme/detail/StaticThemedLinearGradientBrush.hpp>
#include <FredEmmott/GUI/StaticTheme/detail/ResolveColor.hpp>

#include <vector>

namespace $CppNs {

$( ($ColorKeys | ForEach-Object { Get-Color-Cpp $_ }) -join "`n" )
  $( ($BrushKeys | ForEach-Object { Get-Brush-Cpp $_ }) -join "`n" )

}
"@
}

$Content = @"
// $( '@' )generated by: $( $( Get-Item $PSCommandPath ).Name ) $( $( Get-Item ${InputFile} ).Name )
$( if ($Mode -eq "Hpp")
{
  Get-Hpp
}
else
{
  Get-Cpp
} )
"@

($content -split "`r" -join "") `
  | Set-Content -Encoding utf8 "$OutputFile" -NoNewline

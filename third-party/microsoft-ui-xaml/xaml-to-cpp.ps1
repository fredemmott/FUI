# Copyright 2024 Fred Emmott <fred@fredemmott.com>
# SPDX-License-Identifier: MIT
param(
  $Component,
  $InputFiles,
  $OutputFile,
  [ValidateSet("Cpp", "Hpp")][string]$Mode)

$InputFiles = $InputFiles -split ';'

$XMLNS = @{
  p = "http://schemas.microsoft.com/winfx/2006/xaml/presentation";
  x = "http://schemas.microsoft.com/winfx/2006/xaml";
}

function Get-Key($It)
{
  return $It.GetAttribute("Key", $XMLNS.x)
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
    '^#.*' {
      $Hex = $Value.Substring(1)
      $bytes = @()
      for($i = 0; $i -lt $Hex.Length; $i += 2) {
        $bytes += $Hex.Substring($i, 2)
      }
      if ($bytes.Length -eq 3)
      {
        return "SkColorSetRGB(0x$( $bytes[0] ), 0x$( $bytes[1] ), 0x$( $bytes[2] ))"
      }
      return "SkColorSetARGB(0x$( $bytes[0] ), 0x$( $bytes[1] ), 0x$( $bytes[2] ), 0x$( $bytes[3] ))"
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

function Get-StaticResource($StaticResource)
{
  $Key = Get-Key($StaticResource);
  return @{
    Key = $Key
    Value = $StaticResource.GetAttribute("ResourceKey")
  }
}

function Get-Theme($Theme)
{
  $colors = @{ }
  if ($Theme.Color)
  {
    foreach ($Color in $Theme.Color | ForEach-Object { Get-Color $_ } | Sort-Object -Property Key)
    {
      $colors[$Color.Key] = $Color
    }
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

  $aliases = @{ }
  if ($Theme.StaticResource)
  {
    foreach ($Alias in $Theme.StaticResource | ForEach-Object { Get-StaticResource $_ } | Sort-Object -Property Key)
    {
      $aliases[$Alias.Key] = $Alias
    }
  }

  return @{
    Name = $Theme.Key;
    Aliases = $aliases;
    Colors = $colors;
    Brushes = $brushes;
  }
}

foreach ($InputFile in $InputFiles)
{
  Write-Host $InputFile
  $ThemesXML = Select-Xml `
    -Path $InputFile `
    -Namespace $XMLNS `
    -XPath "/p:ResourceDictionary/p:ResourceDictionary.ThemeDictionaries/p:ResourceDictionary"
  $FileData = $ThemesXML | ForEach-Object { Get-Theme $_.Node }
  if ($ThemeData)
  {
    $ThemeData[0].Aliases += $FileData[0].Aliases
    $ThemeData[0].Colors += $FileData[0].Colors
    $ThemeData[0].Brushes += $FileData[0].Brushes
  }
  else
  {
    $ThemeData = $FileData
  }
}
$AliasKeys = $ThemeData[0].Aliases.Keys
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

$CppNs = "FredEmmott::GUI::StaticTheme::$Component"
$DetailNs = "detail_StaticTheme_${Component}"

function Get-Alias-Hpp($Key)
{
  $Default = $DefaultTheme.Aliases[$Key].Value
  @"
using ${Key}_t = std::decay_t<decltype(*$Default)>;
const ${Key}_t* $Key { nullptr };
"@
}

function Resolve-Alias($Theme, $Value)
{
  # Not actually a System color...
  if ($Value -eq 'SystemControlTransparentColor')
  {
    return "$Value->Resolve(StaticTheme::Theme::$Theme)"
  }
  if ($Value -match "^System.+Color$")
  {
    return "SystemTheme::${Value}"
  }
  "$Value->Resolve(StaticTheme::Theme::$Theme)"
}

function Get-Alias-Cpp($Key)
{
  $Default = $DefaultTheme.Aliases[$Key].Value
  $Light = $LightTheme.Aliases.contains($Key) ? $LightTheme.Aliases[$Key].Value : $Default
  $HighContrast = $HighContrastTheme.Aliases.contains($Key) ? $HighContrastTheme.Aliases[$Key].Value : $Default
  @"

  static const ${Key}_t s$Key {
    .mDefault = $( Resolve-Alias 'Dark' $Default ),
    .mLight = $( Resolve-Alias 'Light' $Light ),
    .mHighContrast = $( Resolve-Alias 'HighContrast' $HighContrast ),
  };
  this->$Key = &s$Key;
"@
}

function Get-Hpp()
{
  if (-not ($Component -eq "Common"))
  {
    $Inheritance = ": Common::detail_StaticTheme_Common::Theme"
  }
  @"
#pragma once
#include <FredEmmott/GUI/Brush.hpp>
#include <FredEmmott/GUI/Color.hpp>
#include <FredEmmott/GUI/StaticTheme/Resource.hpp>
$( if (-not ($Component -eq "Common"))
  {
    "#include <FredEmmott/GUI/StaticTheme/Common.hpp>"
  } )

namespace $CppNs::$DetailNs {

struct Theme $Inheritance {
  Theme();
  static Theme* GetInstance();

$( ($ColorKeys | ForEach-Object {
    "const Resource<Color>* $_ {nullptr};"
  }) -join "`n" )

  $( ($BrushKeys | ForEach-Object {
    "const Resource<Brush>* $_ {nullptr};"
  }) -join "`n" )

  $( ($AliasKeys | ForEach-Object {
    Get-Alias-Hpp $_
  }) -join "`n" )
};

}

namespace $CppNs {

$( foreach ($Key in $ColorKeys + $BrushKeys + $AliasKeys)
  {
    "inline const auto $Key = $DetailNs::Theme::GetInstance()->$Key;`n"
  } )

}
"@
}


function Get-Color-Cpp($Key)
{
  @"

  static const Resource<Color> s$Key {
    .mDefault = SkColor { $( $DefaultTheme.Colors[$Key].Value ) },
    .mLight = SkColor { $( $LightTheme.Colors[$Key].Value ) },
    .mHighContrast = SkColor { $( $HighContrastTheme.Colors[$Key].Value ) },
  };
  this->$Key = &s$Key;
"@
}

function Get-Brush-Cpp($Key)
{
  $Default = $DefaultTheme.Brushes[$Key].Value
  $Light = $LightTheme.Brushes.contains($Key) ? $LightTheme.Brushes[$Key].Value : $Default
  $HighContrast = $HighContrastTheme.Brushes.contains($Key) ? $HighContrastTheme.Brushes[$Key].Value : $Default
  @"

  static const Resource<Brush> s$Key {
    .mDefault = ResolveBrush<StaticTheme::Theme::Dark>($( $Default )),
    .mLight = ResolveBrush<StaticTheme::Theme::Light>($( $Light )),
    .mHighContrast = ResolveBrush<StaticTheme::Theme::HighContrast>($( $HighContrast )),
  };
  this->$Key = &s$Key;
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

#include <thread>
#include <vector>

namespace $CppNs::$DetailNs {

Theme::Theme() {
  $( ($ColorKeys | ForEach-Object { Get-Color-Cpp $_ }) -join "`n" )
  $( ($BrushKeys | ForEach-Object { Get-Brush-Cpp $_ }) -join "`n" )
  $( ($AliasKeys | ForEach-Object { Get-Alias-Cpp $_ }) -join "`n" )
}

Theme* Theme::GetInstance() {
  static std::once_flag sOnce;
  static std::unique_ptr<$DetailNs::Theme> sInstance;
  std::call_once(sOnce, [&it = sInstance]() {
    it = std::make_unique<$DetailNs::Theme>();
  });
  return sInstance.get();
}

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

#~ $Id$
# -*- coding: latin-1 -*-

import string
from pathlib import Path

VCPROJHEAD = """\
<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|Win32">
      <Configuration>Debug</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|Win32">
      <Configuration>Release</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Debug|x64">
      <Configuration>Debug</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|x64">
      <Configuration>Release</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
  </ItemGroup>
  <PropertyGroup Label="Globals">
    <VCProjectVersion>18.0</VCProjectVersion>
    <Keyword>Win32Proj</Keyword>
    <ProjectGuid>{a44a6573-df1a-4af7-a835-bbdb2ec21db0}</ProjectGuid>
    <RootNamespace>FileCopy</RootNamespace>
    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'" Label="Configuration">
    <ConfigurationType>Utility</ConfigurationType>
    <UseDebugLibraries>true</UseDebugLibraries>
    <PlatformToolset>v145</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'" Label="Configuration">
    <ConfigurationType>Utility</ConfigurationType>
    <UseDebugLibraries>false</UseDebugLibraries>
    <PlatformToolset>v145</PlatformToolset>
    <WholeProgramOptimization>true</WholeProgramOptimization>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" Label="Configuration">
    <ConfigurationType>Utility</ConfigurationType>
    <UseDebugLibraries>true</UseDebugLibraries>
    <PlatformToolset>v145</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'" Label="Configuration">
    <ConfigurationType>Utility</ConfigurationType>
    <UseDebugLibraries>false</UseDebugLibraries>
    <PlatformToolset>v145</PlatformToolset>
    <WholeProgramOptimization>true</WholeProgramOptimization>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
  <ImportGroup Label="ExtensionSettings">
  </ImportGroup>
  <ImportGroup Label="Shared">
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <PropertyGroup Label="UserMacros" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
    <OutDir>$(SolutionDir)\Output\Product\$(Platform)\$(Configuration)\</OutDir>
    <IntDir>$(SolutionDir)\Output\Intermediate\$(Platform)\$(ProjectName)\$(Configuration)\</IntDir>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
    <OutDir>$(SolutionDir)\Output\Product\$(Platform)\$(Configuration)\</OutDir>
    <IntDir>$(SolutionDir)\Output\Intermediate\$(Platform)\$(ProjectName)\$(Configuration)\</IntDir>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <OutDir>$(SolutionDir)\Output\Product\$(Platform)\$(Configuration)\</OutDir>
    <IntDir>$(SolutionDir)\Output\Intermediate\$(Platform)\$(ProjectName)\$(Configuration)\</IntDir>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <OutDir>$(SolutionDir)\Output\Product\$(Platform)\$(Configuration)\</OutDir>
    <IntDir>$(SolutionDir)\Output\Intermediate\$(Platform)\$(ProjectName)\$(Configuration)\</IntDir>
  </PropertyGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>WIN32;_DEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <LanguageStandard>stdcpp20</LanguageStandard>
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <GenerateDebugInformation>true</GenerateDebugInformation>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>WIN32;NDEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <LanguageStandard>stdcpp20</LanguageStandard>
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <GenerateDebugInformation>true</GenerateDebugInformation>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>_DEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <LanguageStandard>stdcpp20</LanguageStandard>
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <GenerateDebugInformation>true</GenerateDebugInformation>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>NDEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <LanguageStandard>stdcpp20</LanguageStandard>
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <GenerateDebugInformation>true</GenerateDebugInformation>
    </Link>
  </ItemDefinitionGroup>
  <ItemGroup>
"""

VCPROJFOOT = """\
  </ItemGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
  <ImportGroup Label="ExtensionTargets">
  </ImportGroup>
</Project>
"""

TEXFILETEMPLATE = """\
    <CopyFileToFolders Include="%s">
      <FileType>Document</FileType>
      <DestinationFolders>$(OutDir)\Templates\%s</DestinationFolders>
    </CopyFileToFolders>
"""


FILTERSHEAD = """\
<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup>
"""

FILTERDEF = """\
    <Filter Include="%s">
    </Filter>
"""

FILTERFILEENDGROUPSTART = """\
  </ItemGroup>
  <ItemGroup>
"""

FILTERFILE = """\
    <CopyFileToFolders Include="%s">
      <Filter>%s</Filter>
    </CopyFileToFolders>
"""

FILTERSFOOT = """\
  </ItemGroup>
</Project>
"""

def WriteHeads(outfileproj, outfilefilter):
    outfileproj.write(VCPROJHEAD)
    outfilefilter.write(FILTERSHEAD)


def WriteFeet(outfileproj, outfilefilter):
    outfileproj.write(VCPROJFOOT)
    outfilefilter.write(FILTERSFOOT)


def ProcessFile(outfileproj, outfilefilter, texfile, groupname):
    #~ .\Documents\Deutsch\OptionenLayout.tex
    SourceFileName = texfile
    DestFolder = texfile.parent
    print(SourceFileName)
    outfileproj.write(TEXFILETEMPLATE % (SourceFileName, DestFolder))
    outfilefilter.write(FILTERFILE % (SourceFileName, groupname))


def ProcessDir(outfileproj, outfilefilter, path, OptionalPrefix = None):
    if (OptionalPrefix == None):
        groupname = path.name
    else:
        groupname = OptionalPrefix + "\\" + path.name
    print("%s: %s" % (groupname, path))
    
    allgroupnames = [groupname]

    for d in path.iterdir():
        if d.is_dir():
            allgroupnames += ProcessDir(outfileproj, outfilefilter, d, groupname)
            
    for texfile in path.glob("*.tex"):
        ProcessFile(outfileproj, outfilefilter, texfile, groupname)
        
    return allgroupnames



def WriteGroupNames(outfilefilter, GroupNames):
    outfilefilter.write(FILTERFILEENDGROUPSTART)

    for g in GroupNames:
        outfilefilter.write(FILTERDEF % g)
        


import sys
import locale
locale.getpreferredencoding()

if __name__ == "__main__":

    #~ print(locale.getpreferredencoding())

    #~ Test = "Abkürzung"
    #~ print(Test)
    #~ print(Test.encode("cp1252"))
    #~ print(Test.encode("latin-1").decode("utf-8"))
    #~ print(Test.encode("latin-1"))
    #~ print(Test.encode("utf-8"))
    #~ print(Test)

    #~ sys.exit(0)

    #~ Paths
    templatepaths = [Path("Documents"), Path("Projects"), Path("Preview")]

    with Path("Templates.vcxproj").open(mode="w", encoding="utf-8") as outfileproj:
        with Path("Templates.vcxproj.filters").open(mode="w", encoding="utf-8") as outfilefilter:

            WriteHeads(outfileproj, outfilefilter)

            GroupNames = list()
            for p in templatepaths:
                GroupNames += ProcessDir(outfileproj, outfilefilter, p)

            WriteGroupNames(outfilefilter, GroupNames)
                    
            WriteFeet(outfileproj, outfilefilter)

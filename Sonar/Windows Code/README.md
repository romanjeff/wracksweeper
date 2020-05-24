<!-- 
    README.md
    Alex Higgins, higginja AT ece DOT pdx DOT edu
    02-Mar-2020 11:00:38
    
    Readme file for the YellowFin sidescan SONAR RecordData.cpp example. This
    is a port of the original code base that used MFC < v100 (Visual Studio
    2010). It's aimed at using Win10 and MS Visual Studio 2019. 
-->

# README.md

## Dependancies

+ Microsoft Visual Studio 2019 (download)[https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=Community&rel=16]
+ Installation requires Administrator privledges on the host workstation
+ During installation select the following packages under **Workloads**;
  - Desktop development with C++
  - Linux development with C++
+ During installation select the following packages under **Individual components**;
  - C++ MFC for latest v142 build toold (x86 & x64)
  - C++ ATL for latest v142 build toold (x86 & x64)
  - C++ MFC for latest v141 build toold (x86 & x64)
  - C++ ATL for latest v141 build toold (x86 & x64)

## MAKEFILE

+ Launch Visual Studio developer command prompt
+ Navigate to project directory
+ Run command ".\make -generate"

## Configure RecordData VS Solution

+ Open the "cmake_build/RecordData.sln" using VS 2019
+ In the _Solution Explorer_ right-click the **RecordData** project and select "Set as StartUp Project"
+ In the _Solution Explorer_ right-click the **RecordData** project and select "Properties"
  - From the **Configuration Properties** drop-down list select _Advanced_ > _Use of MFC_ > **Use MFC in sharded DLL**
  - From the **C/C++** drop-down list select _Precompiled Headers_ > _Precompilded Header_ > **Create (/Yc)**
  - From the **C/C++** drop-down list select _Precompiled Headers_ > _Precompilded Header File_ > **pch.h**
  - Click the **OK** button to comfirm changes
+ In the _Solution Explorer_ right-click the **RecordData** project and select "Build"
+ Verify that the build was successful in the _Output_ panel. Final line should look like below
  `========== Build: 2 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========`
+ You can now move on to Debugging the newly built executable

## Debugging

TODO:: Write-up simple debugging procedures for MS Visual Studio 2019

---
Updated: 03-Mar-2020 12:29:15  
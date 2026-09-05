; SPDX-License-Identifier: GPL-2.0-or-later
;
; Inno Setup script for cb-pitch-shift. The installer is not code-signed, so
; Windows SmartScreen shows an "unknown publisher" prompt on first run; the ZIP
; release is the alternative.
;
; Values come from Package-Windows.ps1 via ISCC /D defines; the fallbacks below
; let the script compile on its own.
;
; This file is UTF-8 with a BOM on purpose: [CustomMessages] contains CJK text
; and Inno Setup expects a BOM for non-ASCII scripts. Do not strip it.
;
; {#AppName} (the preprocessor define) is the folder and file name; the [Setup]
; AppName directive is the localized product name shown to the user.

#ifndef AppName
  #define AppName "cb-pitch-shift"
#endif
#ifndef AppVersion
  #define AppVersion "0.0.0"
#endif
#ifndef AppSourceDir
  #define AppSourceDir "..\..\release\RelWithDebInfo\cb-pitch-shift"
#endif
#ifndef AppOutputDir
  #define AppOutputDir "..\..\release"
#endif
#ifndef AppOutputBaseName
  #define AppOutputBaseName "cb-pitch-shift-windows-x64"
#endif

[Setup]
; A fixed AppId is what ties every install/upgrade/uninstall together and drives
; the "Apps & features" entry. Never change it once released.
AppId={{B8F3A2E1-7C4D-4A6B-9E2F-3D5C1A8B6F09}
AppName={cm:AppDisplayName}
AppVersion={#AppVersion}
AppVerName={cm:AppDisplayName} {#AppVersion}
AppPublisher=ChiwaBots
AppPublisherURL=https://chiwabots.com
; With AppName set from a custom message ISCC cannot derive these, and the .exe
; would have empty file properties. English is fine for exe metadata.
VersionInfoDescription=Pitch Shift (ChiwaBots) Setup
VersionInfoProductName=Pitch Shift (ChiwaBots)
; OBS on Windows scans %ProgramData%\obs-studio\plugins regardless of where OBS
; itself is installed, so the path is fixed and the install is per-machine.
DefaultDirName={commonappdata}\obs-studio\plugins\{#AppName}
DisableDirPage=yes
DisableProgramGroupPage=yes
UninstallDisplayName={cm:AppDisplayName}
PrivilegesRequired=admin
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
; Auto-use the language matching the system locale (no picker), like the macOS .pkg.
ShowLanguageDialog=no
OutputDir={#AppOutputDir}
OutputBaseFilename={#AppOutputBaseName}

[Languages]
; English / Japanese / Korean / Spanish ship with Inno Setup; Traditional and
; Simplified Chinese are unofficial translations vendored under isl/. The wizard
; auto-picks the language matching the system locale.
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "zh_TW"; MessagesFile: "isl\ChineseTraditional.isl"
Name: "zh_CN"; MessagesFile: "isl\ChineseSimplified.isl"
Name: "ja"; MessagesFile: "compiler:Languages\Japanese.isl"
Name: "ko"; MessagesFile: "compiler:Languages\Korean.isl"
Name: "es"; MessagesFile: "compiler:Languages\Spanish.isl"

[Files]
Source: "{#AppSourceDir}\bin\64bit\*"; DestDir: "{app}\bin\64bit"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#AppSourceDir}\data\*"; DestDir: "{app}\data"; Flags: ignoreversion recursesubdirs createallsubdirs

[CustomMessages]
; Product name shown in the wizard and in "Apps & features". Keep in sync with
; the PitchShift value in data/locale/*.ini plus the suffix in
; src/pitch-shared.hpp. No "OBS" in the name (OBS forum resource policy).
en.AppDisplayName=Pitch Shift (ChiwaBots)
zh_TW.AppDisplayName=升降 key (ChiwaBots)
zh_CN.AppDisplayName=升降 key (ChiwaBots)
ja.AppDisplayName=キー変更 (ChiwaBots)
ko.AppDisplayName=키 조절 (ChiwaBots)
es.AppDisplayName=Cambio de tono (ChiwaBots)

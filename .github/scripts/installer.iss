; SPDX-License-Identifier: GPL-2.0-or-later
;
; Inno Setup script for cb-pitch-shift (UNSIGNED — no code-signing certificate).
; Produces a one-click Windows installer alongside the plain ZIP. Because it is
; unsigned, Windows SmartScreen shows a one-time "unknown publisher" prompt; the
; ZIP avoids that but has to be extracted by hand.
;
; All values come from Package-Windows.ps1 via ISCC /D defines; the fallbacks
; below only exist so the script can be opened/compiled standalone.

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
AppName=Pitch Shift (ChiwaBots.com)
AppVersion={#AppVersion}
AppVerName=Pitch Shift (ChiwaBots.com) {#AppVersion}
AppPublisher=ChiwaBots
AppPublisherURL=https://chiwabots.com
; OBS on Windows only scans %ProgramData%\obs-studio\plugins (machine-wide),
; regardless of where OBS itself is installed — so the path is fixed and the
; install is per-machine (hence admin).
DefaultDirName={commonappdata}\obs-studio\plugins\{#AppName}
DisableDirPage=yes
DisableProgramGroupPage=yes
UninstallDisplayName=Pitch Shift (ChiwaBots.com) OBS plugin
PrivilegesRequired=admin
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
OutputDir={#AppOutputDir}
OutputBaseFilename={#AppOutputBaseName}

[Files]
Source: "{#AppSourceDir}\bin\64bit\*"; DestDir: "{app}\bin\64bit"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#AppSourceDir}\data\*"; DestDir: "{app}\data"; Flags: ignoreversion recursesubdirs createallsubdirs

[Messages]
FinishedLabelNoIcons=Setup has installed Pitch Shift (ChiwaBots.com). Restart OBS Studio to load the plugin.

[Setup]
AppName=oj-client
AppVersion=0.1.0
DefaultDirName={autopf}\oj-client
DisableDirPage=no
UsePreviousAppDir=no
CloseApplications=force
CloseApplicationsFilter=oj.exe
RestartApplications=no
DefaultGroupName=oj-client
OutputDir=installer-out
OutputBaseFilename=oj-client-setup
Compression=lzma
SolidCompression=yes
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64compatible
SetupIconFile=images\icon-page-1.ico
UninstallDisplayIcon={app}\images\icon-page-1.ico

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional icons:"

[Files]
Source: "build\oj.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\*.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "build\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "build\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "build\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "build\sqldrivers\*"; DestDir: "{app}\sqldrivers"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "build\tls\*"; DestDir: "{app}\tls"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "build\networkinformation\*"; DestDir: "{app}\networkinformation"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "build\multimedia\*"; DestDir: "{app}\multimedia"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "build\generic\*"; DestDir: "{app}\generic"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "build\qtwebview\*"; DestDir: "{app}\qtwebview"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist
Source: "build\translations\*"; DestDir: "{app}\translations"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist
Source: "images\light_mode\*"; DestDir: "{app}\images\light_mode"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "images\dark_mode\*"; DestDir: "{app}\images\dark_mode"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "images\icon-page-1.ico"; DestDir: "{app}\images"; Flags: ignoreversion
Source: "images\icon.svg"; DestDir: "{app}\images"; Flags: ignoreversion skipifsourcedoesntexist
Source: "vedio\鍝堝熀绫宠捣搴?- MyRingtone.mp3"; DestDir: "{app}\vedio"; Flags: ignoreversion

[Icons]
Name: "{group}\oj-client"; Filename: "{app}\oj.exe"; IconFilename: "{app}\images\icon-page-1.ico"
Name: "{autodesktop}\oj-client"; Filename: "{app}\oj.exe"; IconFilename: "{app}\images\icon-page-1.ico"; Tasks: desktopicon

[Run]
Filename: "{app}\oj.exe"; Description: "Launch oj-client"; Flags: nowait postinstall skipifsilent

; Note: appstate.toml is no longer generated here. The application creates it in
; the per-user data directory on first run, with ring_path pointing to the
; bundled ringtone under {app}\vedio.

[UninstallDelete]
Type: filesandordirs; Name: "{app}\cache"
Type: filesandordirs; Name: "{app}\data"
Type: filesandordirs; Name: "{app}\images"
Type: filesandordirs; Name: "{app}\platforms"
Type: filesandordirs; Name: "{app}\iconengines"
Type: filesandordirs; Name: "{app}\imageformats"
Type: filesandordirs; Name: "{app}\styles"
Type: filesandordirs; Name: "{app}\sqldrivers"
Type: filesandordirs; Name: "{app}\tls"
Type: filesandordirs; Name: "{app}\networkinformation"
Type: filesandordirs; Name: "{app}\multimedia"
Type: filesandordirs; Name: "{app}\generic"
Type: filesandordirs; Name: "{app}\qtwebview"
Type: filesandordirs; Name: "{app}\translations"
Type: filesandordirs; Name: "{app}\vedio"
Type: files; Name: "{app}\*.dll"
Type: files; Name: "{app}\*.exe"
Type: files; Name: "{app}\*.toml"


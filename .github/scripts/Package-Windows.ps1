[CmdletBinding()]
param(
    [ValidateSet('x64')]
    [string] $Target = 'x64',
    [ValidateSet('Debug', 'RelWithDebInfo', 'Release', 'MinSizeRel')]
    [string] $Configuration = 'RelWithDebInfo'
)

$ErrorActionPreference = 'Stop'

if ( $DebugPreference -eq 'Continue' ) {
    $VerbosePreference = 'Continue'
    $InformationPreference = 'Continue'
}

if ( $env:CI -eq $null ) {
    throw "Package-Windows.ps1 requires CI environment"
}

if ( ! ( [System.Environment]::Is64BitOperatingSystem ) ) {
    throw "Packaging script requires a 64-bit system to build and run."
}

if ( $PSVersionTable.PSVersion -lt '7.2.0' ) {
    Write-Warning 'The packaging script requires PowerShell Core 7. Install or upgrade your PowerShell version: https://aka.ms/pscore6'
    exit 2
}

function Package {
    trap {
        Write-Error $_
        exit 2
    }

    $ScriptHome = $PSScriptRoot
    $ProjectRoot = Resolve-Path -Path "$PSScriptRoot/../.."
    $BuildSpecFile = "${ProjectRoot}/buildspec.json"

    $UtilityFunctions = Get-ChildItem -Path $PSScriptRoot/utils.pwsh/*.ps1 -Recurse

    foreach( $Utility in $UtilityFunctions ) {
        Write-Debug "Loading $($Utility.FullName)"
        . $Utility.FullName
    }

    $BuildSpec = Get-Content -Path ${BuildSpecFile} -Raw | ConvertFrom-Json
    $ProductName = $BuildSpec.name
    $ProductVersion = $BuildSpec.version

    $OutputName = "${ProductName}-${ProductVersion}-windows-${Target}"

    $RemoveArgs = @{
        ErrorAction = 'SilentlyContinue'
        Path = @(
            "${ProjectRoot}/release/${ProductName}-*-windows-*.zip"
            "${ProjectRoot}/release/${ProductName}-*-windows-*.exe"
        )
    }

    Remove-Item @RemoveArgs

    Log-Group "Archiving ${ProductName}..."
    $CompressArgs = @{
        Path = (Get-ChildItem -Path "${ProjectRoot}/release/${Configuration}" -Exclude "${OutputName}*.*")
        CompressionLevel = 'Optimal'
        DestinationPath = "${ProjectRoot}/release/${OutputName}.zip"
        Verbose = ($Env:CI -ne $null)
    }
    Compress-Archive -Force @CompressArgs
    Log-Group

    # Unsigned one-click installer alongside the ZIP (see installer.iss). The
    # release workflow already globs *.exe, so this artifact attaches itself.
    Log-Group "Building installer for ${ProductName}..."
    $InstallerScript = "${ScriptHome}/installer.iss"
    $SourceDir = (Resolve-Path -Path "${ProjectRoot}/release/${Configuration}/${ProductName}").Path

    $Iscc = Get-Command 'iscc' -ErrorAction SilentlyContinue
    if ( $null -ne $Iscc ) {
        $IsccPath = $Iscc.Source
    } else {
        $IsccPath = @(
            "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe"
            "${env:ProgramFiles}\Inno Setup 6\ISCC.exe"
        ) | Where-Object { Test-Path $_ } | Select-Object -First 1

        if ( $null -eq $IsccPath ) {
            Write-Information 'Inno Setup not found; installing via Chocolatey...'
            choco install innosetup --no-progress -y
            $IsccPath = "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe"
        }
    }

    $IsccArgs = @(
        "/DAppName=${ProductName}"
        "/DAppVersion=${ProductVersion}"
        "/DAppSourceDir=$($SourceDir -replace '/', '\')"
        "/DAppOutputDir=$("${ProjectRoot}/release" -replace '/', '\')"
        "/DAppOutputBaseName=${OutputName}"
        "$($InstallerScript -replace '/', '\')"
    )
    & "${IsccPath}" @IsccArgs
    if ( $LASTEXITCODE -ne 0 ) {
        throw "Inno Setup compilation failed with exit code ${LASTEXITCODE}."
    }
    Log-Group
}

Package

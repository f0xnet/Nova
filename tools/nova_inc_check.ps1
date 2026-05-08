# Nova incremental build — single-call dependency checker
#
# Pour chaque .cpp dans -ListFile, décide s'il doit être recompilé en
# comparant son timestamp à celui du .o correspondant. Crée un fichier
# marker "<basename>.compile" dans -MarkerDir pour chaque .cpp à recompiler.
#
# Le .bat itère ensuite sur ses sources et teste l'existence du marker.
#
# Args:
#   -SourceDir  : racine des .cpp (ex: C:\Nova\client)
#   -ObjDir     : dossier des .o     (ex: C:\Nova\client\obj\Release)
#   -ListFile   : un .cpp par ligne, chemin relatif à SourceDir
#   -MarkerDir  : dossier où écrire les markers (réinitialisé à chaque run)

param(
    [Parameter(Mandatory=$true)][string]$SourceDir,
    [Parameter(Mandatory=$true)][string]$ObjDir,
    [Parameter(Mandatory=$true)][string]$ListFile,
    [Parameter(Mandatory=$true)][string]$MarkerDir
)

$ErrorActionPreference = 'Stop'

# Reset marker dir — on repart d'une décision propre à chaque build
if (Test-Path -LiteralPath $MarkerDir) {
    Remove-Item -LiteralPath $MarkerDir -Recurse -Force -ErrorAction SilentlyContinue
}
New-Item -ItemType Directory -Path $MarkerDir -Force | Out-Null

$files = Get-Content -LiteralPath $ListFile

foreach ($rel in $files) {
    $rel = $rel.Trim()
    if (-not $rel) { continue }

    $srcPath  = Join-Path $SourceDir $rel
    $baseName = [System.IO.Path]::GetFileNameWithoutExtension($rel)
    $objPath  = Join-Path $ObjDir "$baseName.o"

    $needs = $false
    if (-not (Test-Path -LiteralPath $objPath)) {
        $needs = $true
    } elseif (Test-Path -LiteralPath $srcPath) {
        $srcTime = (Get-Item -LiteralPath $srcPath).LastWriteTime
        $objTime = (Get-Item -LiteralPath $objPath).LastWriteTime
        if ($srcTime -gt $objTime) {
            $needs = $true
        }
    }

    if ($needs) {
        $marker = Join-Path $MarkerDir "$baseName.compile"
        Set-Content -LiteralPath $marker -Value '' -NoNewline
    }
}

exit 0

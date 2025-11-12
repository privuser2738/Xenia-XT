$iso = "c:/Games/sfxt.iso"
$f = [System.IO.File]::OpenRead($iso)

$gameOffset = 0xFD90000
$sector32Offset = $gameOffset + (32 * 2048)

Write-Host "Checking GDFX header at sector 32:"
Write-Host "  Offset: 0x$($sector32Offset.ToString('X'))"
Write-Host ""

$f.Seek($sector32Offset, [System.IO.SeekOrigin]::Begin) | Out-Null
$header = New-Object byte[] 64
$f.Read($header, 0, 64) | Out-Null

# Print magic (bytes 0-19)
$magic = [System.Text.Encoding]::ASCII.GetString($header, 0, 20)
Write-Host "Magic (bytes 0-19): '$magic'"

# Print root sector (bytes 20-23)
$rootSector = [BitConverter]::ToUInt32($header, 20)
Write-Host "Root sector (bytes 20-23): $rootSector (0x$($rootSector.ToString('X')))"

# Print root size (bytes 24-27)
$rootSize = [BitConverter]::ToUInt32($header, 24)
Write-Host "Root size (bytes 24-27): $rootSize bytes (0x$($rootSize.ToString('X')))"

# Print all header bytes for analysis
Write-Host ""
Write-Host "Full header (first 64 bytes):"
for ($i = 0; $i -lt 64; $i += 16) {
    $line = ""
    for ($j = 0; $j -lt 16 -and ($i + $j) -lt 64; $j++) {
        $line += $header[$i + $j].ToString("X2") + " "
    }
    Write-Host ("  {0:X4}: {1}" -f $i, $line)
}

# Check if root size seems reasonable
Write-Host ""
if ($rootSize -lt 100) {
    Write-Host "WARNING: Root size of $rootSize bytes seems too small!" -ForegroundColor Red
    Write-Host "  This may only hold $([Math]::Floor($rootSize / 18)) entries maximum" -ForegroundColor Red
} elseif ($rootSize -gt 10MB) {
    Write-Host "WARNING: Root size of $rootSize bytes seems too large!" -ForegroundColor Red
} else {
    Write-Host "Root size appears reasonable" -ForegroundColor Green
}

$f.Close()

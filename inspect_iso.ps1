$iso = "c:/Games/sfxt.iso"
$f = [System.IO.File]::OpenRead($iso)
$magic = "MICROSOFT*XBOX*MEDIA"
$found = $false
$offsets = @(0x00000000, 0x0000FB20, 0x00020600, 0x02080000, 0x0FD90000)

Write-Host "Checking for GDFX magic at known offsets..."
Write-Host ""

foreach ($off in $offsets) {
    $pos = $off + (32 * 2048)
    if ($pos -lt $f.Length) {
        $f.Seek($pos, [System.IO.SeekOrigin]::Begin) | Out-Null
        $bytes = New-Object byte[] 20
        $f.Read($bytes, 0, 20) | Out-Null
        $str = [System.Text.Encoding]::ASCII.GetString($bytes)

        if ($str -eq $magic) {
            Write-Host "Found GDFX magic at game_offset 0x$($off.ToString('X'))"
            Write-Host "  Magic position: 0x$($pos.ToString('X'))"

            $f.Seek($pos + 20, [System.IO.SeekOrigin]::Begin) | Out-Null
            $rootData = New-Object byte[] 8
            $f.Read($rootData, 0, 8) | Out-Null
            $rootSector = [BitConverter]::ToUInt32($rootData, 0)
            $rootSize = [BitConverter]::ToUInt32($rootData, 4)

            Write-Host "  Root sector: $rootSector"
            Write-Host "  Root size: $rootSize bytes (0x$($rootSize.ToString('X')))"

            $rootOffset = $off + ($rootSector * 2048)
            Write-Host "  Root offset: 0x$($rootOffset.ToString('X'))"

            $f.Seek($rootOffset, [System.IO.SeekOrigin]::Begin) | Out-Null
            $firstBytes = New-Object byte[] 32
            $f.Read($firstBytes, 0, 32) | Out-Null
            $hexStr = ($firstBytes | ForEach-Object { $_.ToString('X2') }) -join ' '
            Write-Host "  First 32 bytes of root: $hexStr"

            $found = $true
            break
        }
    }
}

if (-not $found) {
    Write-Host "GDFX magic NOT found at any expected offset"
    Write-Host "This is NOT an Xbox 360 disc image!"
    Write-Host ""
    Write-Host "File info:"
    Write-Host "  Size: $($f.Length) bytes ($([Math]::Round($f.Length/1MB, 2)) MB)"
    Write-Host ""
    Write-Host "This appears to be a regular ISO 9660 filesystem,"
    Write-Host "not an Xbox 360 GDFX disc image."
}

$f.Close()

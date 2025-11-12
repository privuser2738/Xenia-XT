$iso = "c:/Games/sfxt.iso"
$f = [System.IO.File]::OpenRead($iso)

$gameOffset = 0xFD90000
$rootSector = 46061
$rootOffset = $gameOffset + ($rootSector * 2048)
$rootSize = 2048

Write-Host "Parsing GDFX directory structure:"
Write-Host "  Root offset: 0x$($rootOffset.ToString('X'))"
Write-Host "  Root size: $rootSize bytes"
Write-Host ""

$f.Seek($rootOffset, [System.IO.SeekOrigin]::Begin) | Out-Null
$rootBuffer = New-Object byte[] $rootSize
$f.Read($rootBuffer, 0, $rootSize) | Out-Null

function Parse-GDFXEntry {
    param($buffer, $ordinal, $depth = 0)

    $offset = $ordinal * 4
    $indent = "  " * $depth

    if ($offset + 18 -gt $buffer.Length) {
        Write-Host "$($indent)ERROR: Entry $ordinal at offset $offset exceeds buffer size $($buffer.Length)" -ForegroundColor Red
        return $false
    }

    $node_l = [BitConverter]::ToUInt16($buffer, $offset + 0)
    $node_r = [BitConverter]::ToUInt16($buffer, $offset + 2)
    $sector = [BitConverter]::ToUInt32($buffer, $offset + 4)
    $length = [BitConverter]::ToUInt32($buffer, $offset + 8)
    $attributes = $buffer[$offset + 12]
    $name_length = $buffer[$offset + 13]

    if ($offset + 14 + $name_length -gt $buffer.Length) {
        Write-Host "$($indent)ERROR: Name length $name_length at entry $ordinal exceeds buffer" -ForegroundColor Red
        return $false
    }

    $name = [System.Text.Encoding]::ASCII.GetString($buffer, $offset + 14, $name_length)

    $isDir = ($attributes -band 0x10) -ne 0
    $type = if ($isDir) { "DIR " } else { "FILE" }

    Write-Host "$($indent)Entry $ordinal (offset $offset): $type '$name'"
    Write-Host "$($indent)  node_l=$node_l, node_r=$node_r, sector=$sector, length=$length, attr=0x$($attributes.ToString('X2'))"

    if ($depth -gt 5) {
        Write-Host "$($indent)  (stopping at depth 5)" -ForegroundColor Yellow
        return $true
    }

    return $true
}

Write-Host "Root directory entries:"
Write-Host ""

# Parse first few entries
for ($i = 0; $i -lt 10; $i++) {
    $result = Parse-GDFXEntry $rootBuffer $i 0
    if (-not $result) {
        Write-Host "Failed at entry $i" -ForegroundColor Red
        break
    }
    Write-Host ""
}

$f.Close()

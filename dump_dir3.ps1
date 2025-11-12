$iso = "c:/Games/sfxt.iso"
$f = [System.IO.File]::OpenRead($iso)
$rootOffset = 0x15786800
$rootSize = 2048

$f.Seek($rootOffset, [System.IO.SeekOrigin]::Begin) | Out-Null
$dir = New-Object byte[] $rootSize
$f.Read($dir, 0, $rootSize) | Out-Null

function Show-Entry {
    param($offset)

    if ($offset + 14 -gt $rootSize) {
        Write-Host "Entry at offset $offset: EXCEEDS BUFFER SIZE" -ForegroundColor Red
        return
    }

    $node_l = [BitConverter]::ToUInt16($dir, $offset)
    $node_r = [BitConverter]::ToUInt16($dir, $offset + 2)
    $sector = [BitConverter]::ToUInt32($dir, $offset + 4)
    $len = [BitConverter]::ToUInt32($dir, $offset + 8)
    $attr = $dir[$offset + 12]
    $nlen = $dir[$offset + 13]

    $type = if ($attr -band 0x10) { "DIR " } else { "FILE" }

    Write-Host "Entry at offset $offset ($type):"
    Write-Host "  node_l=$node_l (offset $($node_l * 4)), node_r=$node_r (offset $($node_r * 4))"

    if ($offset + 14 + $nlen -gt $rootSize) {
        Write-Host "  ERROR: name_len $nlen would exceed buffer (need $($offset + 14 + $nlen), have $rootSize)" -ForegroundColor Red
        Write-Host "  sector=$sector, length=$len, attr=0x$($attr.ToString('X2'))"
    } else {
        $name = [System.Text.Encoding]::ASCII.GetString($dir, $offset + 14, $nlen)
        Write-Host "  name='$name', sector=$sector, length=$len, attr=0x$($attr.ToString('X2'))"
    }
}

Write-Host "=== Tracing the tree structure ==="
Write-Host ""

Write-Host "Entry 0 (root):"
Show-Entry 0
Write-Host ""

Write-Host "Entry 7 (left child of 0):"
Show-Entry 28
Write-Host ""

Write-Host "Entry 13 (left child of 7):"
Show-Entry 52
Write-Host ""

Write-Host "Entry 20 (right child of 7):"
Show-Entry 80
Write-Host ""

Write-Host "Entry 28 (right child of 0):"
Show-Entry 112
Write-Host ""

$f.Close()

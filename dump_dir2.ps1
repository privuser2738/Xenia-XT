$iso = "c:/Games/sfxt.iso"
$f = [System.IO.File]::OpenRead($iso)

$gameOffset = 0xFD90000
$rootOffset = 0x15786800
$rootSize = 2048

$f.Seek($rootOffset, [System.IO.SeekOrigin]::Begin) | Out-Null
$dir = New-Object byte[] $rootSize
$f.Read($dir, 0, $rootSize) | Out-Null

Write-Host "Entry 0 (offset 0):"
$node_l = [BitConverter]::ToUInt16($dir, 0)
$node_r = [BitConverter]::ToUInt16($dir, 2)
$name_len = $dir[13]
$name = [System.Text.Encoding]::ASCII.GetString($dir, 14, $name_len)
Write-Host "  node_l=$node_l (offset $($node_l * 4))"
Write-Host "  node_r=$node_r (offset $($node_r * 4))"
Write-Host "  name_len=$name_len"
Write-Host "  name='$name'"
Write-Host "  entry_size=$(14 + $name_len)"
Write-Host ""

Write-Host "Bytes 0-31:"
for ($i = 0; $i -lt 32; $i += 16) {
    $hex = ($dir[$i..($i+15)] | ForEach-Object { $_.ToString('X2') }) -join ' '
    Write-Host "  $($i.ToString('X2')): $hex"
}
Write-Host ""

Write-Host "Entry 7 (offset 28):"
$off = 28
if ($off + 14 -le $rootSize) {
    $node_l7 = [BitConverter]::ToUInt16($dir, $off)
    $node_r7 = [BitConverter]::ToUInt16($dir, $off + 2)
    $sector7 = [BitConverter]::ToUInt32($dir, $off + 4)
    $len7 = [BitConverter]::ToUInt32($dir, $off + 8)
    $attr7 = $dir[$off + 12]
    $nlen7 = $dir[$off + 13]

    Write-Host "  node_l=$node_l7 (offset $($node_l7 * 4))"
    Write-Host "  node_r=$node_r7 (offset $($node_r7 * 4))"
    Write-Host "  sector=$sector7"
    Write-Host "  length=$len7"
    Write-Host "  attr=0x$($attr7.ToString('X2'))"
    Write-Host "  name_len=$nlen7"

    if ($off + 14 + $nlen7 -le $rootSize) {
        $name7 = [System.Text.Encoding]::ASCII.GetString($dir, $off + 14, $nlen7)
        Write-Host "  name='$name7'"
    } else {
        Write-Host "  ERROR: name_len $nlen7 would read past buffer end!" -ForegroundColor Red
        Write-Host "  Buffer size: $rootSize"
        Write-Host "  Would need: $($off + 14 + $nlen7)" -ForegroundColor Red
    }
} else {
    Write-Host "  ERROR: offset exceeds buffer!" -ForegroundColor Red
}
Write-Host ""

Write-Host "Bytes 28-63:"
for ($i = 28; $i -lt 64; $i += 16) {
    $hex = ($dir[$i..([Math]::Min($i+15,$rootSize-1))] | ForEach-Object { $_.ToString('X2') }) -join ' '
    Write-Host "  $($i.ToString('X2')): $hex"
}

$f.Close()

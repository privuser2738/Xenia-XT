$isoSize = (Get-Item 'c:/Games/sfxt.iso').Length
$rootOffset = 0x15786800
$gameOffset = 0xFD90000

Write-Host "File Analysis:"
Write-Host "  ISO file size: 0x$($isoSize.ToString('X')) ($($isoSize.ToString('N0')) bytes, $([Math]::Round($isoSize/1MB, 2)) MB)"
Write-Host "  Game offset:   0x$($gameOffset.ToString('X'))"
Write-Host "  Root offset:   0x$($rootOffset.ToString('X')) ($($rootOffset.ToString('N0')) bytes)"
Write-Host ""

if ($rootOffset -gt $isoSize) {
    Write-Host "ERROR: Root offset (0x$($rootOffset.ToString('X'))) exceeds file size!" -ForegroundColor Red
    Write-Host "  Overflow by: $(($rootOffset - $isoSize).ToString('N0')) bytes" -ForegroundColor Red
} else {
    $remaining = $isoSize - $rootOffset
    Write-Host "Valid: Root is $($remaining.ToString('N0')) bytes before end of file" -ForegroundColor Green
    Write-Host "  This is enough for: $([Math]::Floor($remaining / 2048)) directory sectors" -ForegroundColor Green
}

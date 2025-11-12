$iso = "c:/Games/sfxt.iso"
$f = [System.IO.File]::OpenRead($iso)

$gameOffset = 0xFD90000
$rootSector = 46061
$rootOffset = $gameOffset + ($rootSector * 2048)
$rootSize = 2048

Write-Host "Dumping directory buffer from offset 0x$($rootOffset.ToString('X')):"
Write-Host ""

$f.Seek($rootOffset, [System.IO.SeekOrigin]::Begin) | Out-Null
$dirBuffer = New-Object byte[] $rootSize
$f.Read($dirBuffer, 0, $rootSize) | Out-Null

# Function to parse a directory entry
function Parse-Entry {
    param($buffer, $offset)

    if ($offset + 14 -gt $buffer.Length) {
        Write-Host "Offset $offset exceeds buffer size" -ForegroundColor Red
        return $null
    }

    $node_l = [BitConverter]::ToUInt16($buffer, $offset + 0)
    $node_r = [BitConverter]::ToUInt16($buffer, $offset + 2)
    $sector = [BitConverter]::ToUInt32($buffer, $offset + 4)
    $length = [BitConverter]::ToUInt32($buffer, $offset + 8)
    $attr = $buffer[$offset + 12]
    $name_len = $buffer[$offset + 13]

    if ($offset + 14 + $name_len -gt $buffer.Length) {
        Write-Host "Entry at offset $offset: name_length $name_len would exceed buffer" -ForegroundColor Red
        return @{
            offset = $offset
            node_l = $node_l
            node_r = $node_r
            sector = $sector
            length = $length
            attr = $attr
            name_len = $name_len
            name = "ERROR: name exceeds buffer"
            valid = $false
        }
    }

    $name = [System.Text.Encoding]::ASCII.GetString($buffer, $offset + 14, $name_len)
    $entry_size = 14 + $name_len
    $padded_size = [Math]::Ceiling($entry_size / 4) * 4

    return @{
        offset = $offset
        node_l = $node_l
        node_r = $node_r
        sector = $sector
        length = $length
        attr = $attr
        name_len = $name_len
        name = $name
        entry_size = $entry_size
        padded_size = $padded_size
        valid = $true
    }
}

# Parse entry 0
Write-Host "=== Entry 0 (at offset 0) ==="
$entry0 = Parse-Entry $dirBuffer 0
if ($entry0) {
    Write-Host ("  node_l: {0} (points to offset {1})" -f $entry0.node_l, ($entry0.node_l * 4))
    Write-Host ("  node_r: {0} (points to offset {1})" -f $entry0.node_r, ($entry0.node_r * 4))
    Write-Host ("  sector: {0}" -f $entry0.sector)
    Write-Host ("  length: {0}" -f $entry0.length)
    Write-Host ("  attr: 0x{0:X2}" -f $entry0.attr)
    Write-Host ("  name_len: {0}" -f $entry0.name_len)
    Write-Host ("  name: '{0}'" -f $entry0.name)
    Write-Host ("  entry_size: {0} bytes" -f $entry0.entry_size)
    Write-Host ("  padded_size: {0} bytes" -f $entry0.padded_size)
    Write-Host ""

    # Show padding bytes
    Write-Host "  Bytes at end of entry 0:"
    for ($i = $entry0.entry_size; $i -lt $entry0.padded_size; $i++) {
        Write-Host ("    Byte {0}: 0x{1:X2}" -f $i, $dirBuffer[$i])
    }
    Write-Host ""
}

# Parse entry at node_l (entry 7, offset 28)
Write-Host "=== Entry 7 (at offset 28, left child of entry 0) ==="
$entry7 = Parse-Entry $dirBuffer 28
if ($entry7) {
    if ($entry7.valid) {
        Write-Host ("  node_l: {0} (points to offset {1})" -f $entry7.node_l, ($entry7.node_l * 4))
        Write-Host ("  node_r: {0} (points to offset {1})" -f $entry7.node_r, ($entry7.node_r * 4))
        Write-Host ("  sector: {0}" -f $entry7.sector)
        Write-Host ("  length: {0}" -f $entry7.length)
        Write-Host ("  attr: 0x{0:X2}" -f $entry7.attr)
        Write-Host ("  name_len: {0}" -f $entry7.name_len)
        Write-Host ("  name: '{0}'" -f $entry7.name)
    } else {
        Write-Host "  INVALID ENTRY!" -ForegroundColor Red
        Write-Host ("  node_l: {0}" -f $entry7.node_l)
        Write-Host ("  node_r: {0}" -f $entry7.node_r)
        Write-Host ("  sector: {0}" -f $entry7.sector)
        Write-Host ("  length: {0}" -f $entry7.length)
        Write-Host ("  attr: 0x{0:X2}" -f $entry7.attr)
        Write-Host ("  name_len: {0} (EXCEEDS BUFFER)" -f $entry7.name_len)
    }
    Write-Host ""
}

# Show raw bytes at offset 28
Write-Host "Raw bytes at offset 28-60:"
for ($i = 28; $i -lt 60 -and $i -lt $dirBuffer.Length; $i += 16) {
    $line = "  {0:X4}: " -f $i
    for ($j = 0; $j -lt 16 -and ($i + $j) -lt $dirBuffer.Length; $j++) {
        $line += "{0:X2} " -f $dirBuffer[$i + $j]
    }
    Write-Host $line
}

$f.Close()

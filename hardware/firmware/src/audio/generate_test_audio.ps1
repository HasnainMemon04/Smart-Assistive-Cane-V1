# Smart Cane Audio File Generator
# This PowerShell script generates test audio files using Windows Speech API
# Run this script to create placeholder audio files for testing the audio system

Param(
    [string]$OutputPath = ".",
    [switch]$GenerateAll = $false,
    [switch]$GenerateTestOnly = $true
)

# Add required assemblies for speech synthesis
Add-Type -AssemblyName System.Speech

# Create speech synthesizer
$synth = New-Object System.Speech.Synthesis.SpeechSynthesizer

# Configure speech settings
$synth.Rate = -2  # Slower speech for clarity
$synth.Volume = 100  # Maximum volume

# Get available voices and select a clear one
$voices = $synth.GetInstalledVoices()
Write-Host "Available voices:"
foreach ($voice in $voices) {
    Write-Host "  - $($voice.VoiceInfo.Name) ($($voice.VoiceInfo.Culture))"
}

# Try to select a good voice (prefer female voices for clarity)
$preferredVoices = @("Microsoft Zira Desktop", "Microsoft Hazel Desktop", "Microsoft Eva Desktop")
foreach ($preferred in $preferredVoices) {
    try {
        $synth.SelectVoice($preferred)
        Write-Host "Selected voice: $preferred"
        break
    } catch {
        # Voice not available, continue
    }
}

Write-Host "Using voice: $($synth.Voice.Name)"

# Function to generate audio file
function Generate-AudioFile {
    param(
        [string]$Text,
        [string]$FilePath,
        [string]$Description = ""
    )
    
    try {
        # Ensure directory exists
        $dir = Split-Path $FilePath -Parent
        if (!(Test-Path $dir)) {
            New-Item -ItemType Directory -Path $dir -Force | Out-Null
        }
        
        Write-Host "Generating: $FilePath - $Description"
        
        # Generate speech to WAV file
        $synth.SetOutputToWaveFile($FilePath)
        $synth.Speak($Text)
        $synth.SetOutputToDefaultAudioDevice()
        
        # Convert to 16kHz mono using FFmpeg if available
        if (Get-Command ffmpeg -ErrorAction SilentlyContinue) {
            $tempFile = $FilePath + ".temp"
            Move-Item $FilePath $tempFile
            & ffmpeg -i $tempFile -ar 16000 -ac 1 -sample_fmt s16 $FilePath -y 2>$null
            Remove-Item $tempFile -ErrorAction SilentlyContinue
        }
        
        return $true
    } catch {
        Write-Error "Failed to generate $FilePath : $($_.Exception.Message)"
        return $false
    }
}

# Test audio files (essential for system validation)
$testFiles = @(
    @{Path="critical/system_ready.wav"; Text="System ready"; Desc="System startup complete"},
    @{Path="critical/obstacle_ahead.wav"; Text="Obstacle ahead"; Desc="Immediate danger warning"},
    @{Path="critical/low_battery.wav"; Text="Low battery"; Desc="Battery warning"},
    @{Path="digits/num0.wav"; Text="zero"; Desc="Number 0"},
    @{Path="digits/num1.wav"; Text="one"; Desc="Number 1"},
    @{Path="digits/num2.wav"; Text="two"; Desc="Number 2"},
    @{Path="digits/num3.wav"; Text="three"; Desc="Number 3"},
    @{Path="digits/num4.wav"; Text="four"; Desc="Number 4"},
    @{Path="digits/num5.wav"; Text="five"; Desc="Number 5"},
    @{Path="digits/point.wav"; Text="point"; Desc="Decimal point"},
    @{Path="digits/centimeters.wav"; Text="centimeters"; Desc="Distance unit"},
    @{Path="digits/celsius.wav"; Text="degrees celsius"; Desc="Temperature unit"},
    @{Path="digits/lux.wav"; Text="lux"; Desc="Light unit"},
    @{Path="serial/serial_001.wav"; Text="WiFi connected"; Desc="WiFi connection established"},
    @{Path="serial/serial_207.wav"; Text="Smart Cane System Status"; Desc="System status header"},
    @{Path="serial/serial_208.wav"; Text="Smart Assistive Cane version one point zero"; Desc="System version"},
    @{Path="navigation/go_straight.wav"; Text="Go straight"; Desc="Direction instruction"},
    @{Path="navigation/turn_left.wav"; Text="Turn left"; Desc="Direction instruction"},
    @{Path="navigation/turn_right.wav"; Text="Turn right"; Desc="Direction instruction"},
    @{Path="environmental/bright_light.wav"; Text="Bright lighting"; Desc="Light condition"}
)

# Complete digit files
$digitFiles = @(
    @{Path="digits/num0.wav"; Text="zero"},
    @{Path="digits/num1.wav"; Text="one"},
    @{Path="digits/num2.wav"; Text="two"},
    @{Path="digits/num3.wav"; Text="three"},
    @{Path="digits/num4.wav"; Text="four"},
    @{Path="digits/num5.wav"; Text="five"},
    @{Path="digits/num6.wav"; Text="six"},
    @{Path="digits/num7.wav"; Text="seven"},
    @{Path="digits/num8.wav"; Text="eight"},
    @{Path="digits/num9.wav"; Text="nine"},
    @{Path="digits/num10.wav"; Text="ten"},
    @{Path="digits/num20.wav"; Text="twenty"},
    @{Path="digits/num30.wav"; Text="thirty"},
    @{Path="digits/num40.wav"; Text="forty"},
    @{Path="digits/num50.wav"; Text="fifty"},
    @{Path="digits/num60.wav"; Text="sixty"},
    @{Path="digits/num70.wav"; Text="seventy"},
    @{Path="digits/num80.wav"; Text="eighty"},
    @{Path="digits/num90.wav"; Text="ninety"},
    @{Path="digits/hundred.wav"; Text="hundred"},
    @{Path="digits/point.wav"; Text="point"},
    @{Path="digits/centimeters.wav"; Text="centimeters"},
    @{Path="digits/meters.wav"; Text="meters"},
    @{Path="digits/celsius.wav"; Text="degrees celsius"},
    @{Path="digits/lux.wav"; Text="lux"},
    @{Path="digits/percent.wav"; Text="percent"}
)

# Critical system files
$criticalFiles = @(
    @{Path="critical/system_ready.wav"; Text="System ready"},
    @{Path="critical/obstacle_ahead.wav"; Text="Obstacle ahead"},
    @{Path="critical/distance_sensor_failed.wav"; Text="Distance sensor failed"},
    @{Path="critical/low_battery.wav"; Text="Low battery"},
    @{Path="critical/emergency_alert.wav"; Text="Emergency alert"},
    @{Path="critical/gps_no_signal.wav"; Text="GPS signal lost"}
)

# Key serial statement files
$serialFiles = @(
    @{Path="serial/serial_001.wav"; Text="WiFi connected"},
    @{Path="serial/serial_002.wav"; Text="WiFi connection failed"},
    @{Path="serial/serial_021.wav"; Text="Testing vibration motors"},
    @{Path="serial/serial_025.wav"; Text="BLE connected"},
    @{Path="serial/serial_026.wav"; Text="BLE disconnected"},
    @{Path="serial/serial_202.wav"; Text="Running full system diagnostic"},
    @{Path="serial/serial_203.wav"; Text="Diagnostic complete"},
    @{Path="serial/serial_207.wav"; Text="Smart Cane System Status"},
    @{Path="serial/serial_208.wav"; Text="Smart Assistive Cane version one point zero"},
    @{Path="serial/serial_222.wav"; Text="Available commands"}
)

# Navigation files
$navigationFiles = @(
    @{Path="navigation/turn_left.wav"; Text="Turn left"},
    @{Path="navigation/turn_right.wav"; Text="Turn right"},
    @{Path="navigation/go_straight.wav"; Text="Go straight"},
    @{Path="navigation/stairs_ahead.wav"; Text="Stairs ahead"},
    @{Path="navigation/destination_reached.wav"; Text="Destination reached"},
    @{Path="navigation/satellites_connected.wav"; Text="Satellites connected"}
)

# Environmental files
$environmentalFiles = @(
    @{Path="environmental/dark_environment.wav"; Text="Dark environment"},
    @{Path="environmental/dim_light.wav"; Text="Dim lighting"},
    @{Path="environmental/bright_light.wav"; Text="Bright lighting"},
    @{Path="environmental/wet_surface.wav"; Text="Wet surface detected"},
    @{Path="environmental/uneven_ground.wav"; Text="Uneven ground"}
)

# Generate files based on parameters
Write-Host "Starting audio file generation..."
Write-Host "Output path: $OutputPath"

$successCount = 0
$totalCount = 0

if ($GenerateTestOnly) {
    Write-Host "Generating test files only..."
    foreach ($file in $testFiles) {
        $fullPath = Join-Path $OutputPath $file.Path
        $totalCount++
        if (Generate-AudioFile -Text $file.Text -FilePath $fullPath -Description $file.Desc) {
            $successCount++
        }
    }
} elseif ($GenerateAll) {
    Write-Host "Generating all audio files..."
    
    $allFiles = $digitFiles + $criticalFiles + $serialFiles + $navigationFiles + $environmentalFiles
    
    foreach ($file in $allFiles) {
        $fullPath = Join-Path $OutputPath $file.Path
        $totalCount++
        if (Generate-AudioFile -Text $file.Text -FilePath $fullPath) {
            $successCount++
        }
    }
} else {
    Write-Host "Generating essential files..."
    $essentialFiles = $digitFiles + $criticalFiles + $serialFiles[0..4]  # First 5 serial files
    
    foreach ($file in $essentialFiles) {
        $fullPath = Join-Path $OutputPath $file.Path
        $totalCount++
        if (Generate-AudioFile -Text $file.Text -FilePath $fullPath) {
            $successCount++
        }
    }
}

# Cleanup
$synth.Dispose()

Write-Host ""
Write-Host "Audio generation complete!"
Write-Host "Successfully generated: $successCount / $totalCount files"

if ($successCount -lt $totalCount) {
    Write-Host "Some files failed to generate. Check error messages above." -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Next steps:"
Write-Host "1. Copy the audio folder to your SD card"
Write-Host "2. Insert SD card into the Smart Cane"
Write-Host "3. Test audio system with 'audiotest' command"
Write-Host "4. Use 'audiostatus' to check system status"
Write-Host ""
Write-Host "For better quality audio files, consider using:"
Write-Host "- Professional text-to-speech services (Google TTS, Amazon Polly)"
Write-Host "- Professional voice recording"
Write-Host "- Audio editing software (Audacity)"

Write-Host ""
Write-Host "Audio file requirements:"
Write-Host "- Format: WAV, 16kHz, 16-bit, Mono"
Write-Host "- Duration: 1-3 seconds per file"
Write-Host "- Clear pronunciation for accessibility"

if (!(Get-Command ffmpeg -ErrorAction SilentlyContinue)) {
    Write-Host ""
    Write-Host "Note: FFmpeg not found. Audio files may not be in optimal format." -ForegroundColor Yellow
    Write-Host "Install FFmpeg for automatic format conversion to 16kHz mono."
}
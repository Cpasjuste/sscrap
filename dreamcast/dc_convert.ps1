$media_path=$args[0]

if([string]::IsNullOrWhiteSpace($media_path)) {
    Write-Output "usage: .\dc_convert.ps1 [medias_path]"
    exit 1
}

$media_path=Resolve-Path($args[0])

# optimise png
##############
Write-Output "`nImage conversion (imagemagick/pngquant)`n======================================`n"
Get-ChildItem -Path . -Filter *.png -Recurse -ErrorAction SilentlyContinue -Force |
Foreach-Object {
    $file = $_.FullName
    Write-Output "Processing: $file"
    .\magick.exe mogrify -resize 256x256 $file
    .\pngquant.exe --ext .png --force 256 $file
}


# convert videos
################
Write-Output "`nVideo conversion (ffmpeg)`n==========================`n"
Get-ChildItem -Path . -Filter *.mp4 -Recurse -ErrorAction SilentlyContinue -Force |
Foreach-Object {
    $path = $_.Directory
    $file = [System.Io.Path]::GetFileNameWithoutExtension($_.Name);
    $input = "$path\$file.mp4"
    $output = "$path\$file.roq"
    if(![System.IO.File]::Exists($output)) {
        Write-Output "Processing: $file.mp4 > $file.roq"
        .\ffmpeg.exe -v 8 -i $input -ar 22050 -framerate 30 -vf "scale=256:-2" -t 30 $output
        if (!$?) {
            Write-Output "error: could not convert $input"
        } else {
            Remove-Item $input
        }
    } else {
        Write-Output "skipped: $input (already converted)"
    }
}

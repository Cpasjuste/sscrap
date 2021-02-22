# Set-ExecutionPolicy Unrestricted

function write-error($msg){
    $msg | write-host -fore green -back red;
}

$games_path = "GAMES/"
$media_path = "RD/medias/"

if(![string]::IsNullOrWhiteSpace($args[0])) {
    $games_path = $args[0];
}

if(![string]::IsNullOrWhiteSpace($args[1])) {
    $media_path = $args[1];
}

if(-not(Test-Path $games_path)) {
    write-error "Error: game path doesn't exist: $games_path"
    exit 1
}

# set paths
New-Item $media_path -ItemType Directory -Force
$games_path = Resolve-Path($games_path)
$media_path = Resolve-Path($media_path)
Write-Output "games_path: $games_path"
Write-Output "media_path: $media_path"

# sscrap
.\sscrap\sscrap-utility.exe -u cpasjuste -p NuakvokVezCeog9 -sid 23 -r $games_path -m $media_path -dlm video-normalized box-2D


# optimise png
##############
Write-Output "`nImage conversion (imagemagick/pngquant)`n======================================`n"
Get-ChildItem -Path $media_path -Filter *.png -Recurse -ErrorAction SilentlyContinue -Force |
        Foreach-Object {
            $file = $_.FullName
            Write-Output "Processing: $file"
            .\sscrap\magick.exe mogrify -resize 256x256 $file
            .\sscrap\pngquant.exe --ext .png --force 256 $file
        }


# convert videos
################
Write-Output "`nVideo conversion (ffmpeg)`n==========================`n"
Get-ChildItem -Path $media_path -Filter *.mp4 -Recurse -ErrorAction SilentlyContinue -Force |
        Foreach-Object {
            $path = $_.Directory
            $file = [System.Io.Path]::GetFileNameWithoutExtension($_.Name);
            $input = "$path\$file.mp4"
            $output = "$path\$file.roq"
            if(![System.IO.File]::Exists($output)) {
                Write-Output "Processing: $file.mp4 > $file.roq"
                .\sscrap\ffmpeg.exe -v 8 -i $input -ar 22050 -framerate 30 -vf "scale=256:-2" -t 30 $output
                if (!$?) {
                    Write-Output "error: could not convert $input"
                } else {
                    Remove-Item $input
                }
            } else {
                Write-Output "skipped: $input (already converted)"
            }
        }

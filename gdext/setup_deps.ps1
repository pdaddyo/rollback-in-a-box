# Clone the two vendored dependencies at the pinned, known-good revisions.
# Run from anywhere: powershell -ExecutionPolicy Bypass -File gdext/setup_deps.ps1
$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot

$BOX3D_SHA = "d421e45c828f6f853a145f726f0b9425d31146eb"
$GODOT_CPP_TAG = "godot-4.5-stable"

if (-not (Test-Path "extern/box3d")) {
    git clone https://github.com/erincatto/box3d.git extern/box3d
    git -C extern/box3d checkout --detach $BOX3D_SHA
} else {
    Write-Host "extern/box3d already present"
}

if (-not (Test-Path "extern/godot-cpp")) {
    git clone --depth 1 --branch $GODOT_CPP_TAG https://github.com/godotengine/godot-cpp.git extern/godot-cpp
} else {
    Write-Host "extern/godot-cpp already present"
}

Write-Host "deps ready"

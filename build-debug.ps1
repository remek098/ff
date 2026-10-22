Write-Host "=== LAUNCHING DEBUG BUILD SCRIPT ===" -ForegroundColor Cyan

if (-not (Test-Path "build-Debug")) {
    Write-Host "build-Debug doesn't exist. Generating CMake configuration..." -ForegroundColor Yellow
    cmake -S . -B build-Debug -DCMAKE_BUILD_TYPE=Debug -G Ninja
}

# Kompilacja projektu
cmake --build "build-Debug" --config Debug --target all

if ($LASTEXITCODE -eq 0) {
    Write-Host "=== DEBUG BUILD COMPLETED SUCCESSFULLY ===" -ForegroundColor Green
} else {
    Write-Host "=== ERROR DURING DEBUG BUILD ===" -ForegroundColor Red
    exit $LASTEXITCODE
}
Write-Host "LAUNCHING RELEASE BUILD SCRIPT ===" -ForegroundColor Cyan

# Automatyczne tworzenie folderu i konfiguracja CMake, jeśli nie istnieje
if (-not (Test-Path "build-Release")) {
    Write-Host "build-Release doesn't exist. Generating CMake configuration..." -ForegroundColor Yellow
    cmake -S . -B build-Release -DCMAKE_BUILD_TYPE=Release -G Ninja
}

# Kompilacja projektu
cmake --build "build-Release" --config Release --target all

if ($LASTEXITCODE -eq 0) {
    Write-Host "=== RELEASE BUILD COMPLETED SUCCESSFULLY ===" -ForegroundColor Green
} else {
    Write-Host "=== ERROR DURING RELEASE BUILD ===" -ForegroundColor Red
    exit $LASTEXITCODE
}
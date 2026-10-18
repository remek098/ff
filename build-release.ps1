Write-Host "=== URUCHAMIANIE SKRYPTU: BUDOWANIE RELEASE ===" -ForegroundColor Cyan

# Automatyczne tworzenie folderu i konfiguracja CMake, jeśli nie istnieje
if (-not (Test-Path "build-Release")) {
    Write-Host "Folder build-Release nie istnieje. Generowanie konfiguracji CMake..." -ForegroundColor Yellow
    cmake -S . -B build-Release -DCMAKE_BUILD_TYPE=Release -G Ninja
}

# Kompilacja projektu
cmake --build "build-Release" --config Release --target all

if ($LASTEXITCODE -eq 0) {
    Write-Host "=== BUDOWANIE RELEASE ZAKONCZONE SUKCESEM ===" -ForegroundColor Green
} else {
    Write-Host "=== BLAD BUDOWANIA RELEASE ===" -ForegroundColor Red
    exit $LASTEXITCODE
}
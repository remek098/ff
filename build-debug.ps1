Write-Host "=== URUCHAMIANIE SKRYPTU: BUDOWANIE DEBUG ===" -ForegroundColor Cyan

# Automatyczne tworzenie folderu i konfiguracja CMake, jeśli nie istnieje
if (-not (Test-Path "build-Debug")) {
    Write-Host "Folder build-Debug nie istnieje. Generowanie konfiguracji CMake..." -ForegroundColor Yellow
    cmake -S . -B build-Debug -DCMAKE_BUILD_TYPE=Debug -G Ninja
}

# Kompilacja projektu
cmake --build "build-Debug" --config Debug --target all

if ($LASTEXITCODE -eq 0) {
    Write-Host "=== BUDOWANIE DEBUG ZAKONCZONE SUKCESEM ===" -ForegroundColor Green
} else {
    Write-Host "=== BLAD BUDOWANIA DEBUG ===" -ForegroundColor Red
    exit $LASTEXITCODE
}
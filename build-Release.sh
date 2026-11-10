#!/bin/bash

# ANSI Color Codes
CYAN='\033[0;36m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${CYAN}=== LAUNCHING RELEASE BUILD SCRIPT ===${NC}"

# Check for Clang and set environment variables if found
if command -v clang &> /dev/null && command -v clang++ &> /dev/null; then
    echo -e "${GREEN}Clang detected! Setting CC=clang and CXX=clang++${NC}"
    export CC=clang
    export CXX=clang++
else
    echo -e "${YELLOW}Clang not found. Falling back to default system compiler (GCC).${NC}"
fi

if [ ! -d "build-Release" ]; then
    echo -e "${YELLOW}build-Release doesn't exist. Generating CMake configuration...${NC}"
    cmake -S . -B build-Release -DCMAKE_BUILD_TYPE=Release -G Ninja
fi

# Kompilacja projektu
cmake --build "build-Release" --config Release --target all

BUILD_EXIT_CODE=$?

if [ $BUILD_EXIT_CODE -eq 0 ]; then
    echo -e "${GREEN}=== RELEASE BUILD COMPLETED SUCCESSFULLY ===${NC}"
else
    echo -e "${RED}=== ERROR DURING RELEASE BUILD ===${NC}"
    exit $BUILD_EXIT_CODE
fi
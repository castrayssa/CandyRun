@echo off
rem Atalho no Windows para compilar com o CMake (o mesmo que esta no README).
rem Uso:  build.bat                  compila o jogo (candyrun.exe na pasta do projeto)
rem       build.bat run              compila e ja abre o jogo
rem
rem Precisa de: CMake e um compilador C++ (o MinGW-w64 do winlibs ja traz os dois).
rem O SFML nao precisa instalar: na primeira vez o CMake baixa e compila sozinho.

setlocal
cd /d "%~dp0"

where cmake >nul 2>nul
if errorlevel 1 (
    echo Nao achei o CMake. Instale o CMake ^(ou o MinGW do winlibs, que ja traz^) e coloque no PATH.
    exit /b 1
)

set ALVO=candyrun
set RODAR=
if /i "%1"=="run" set RODAR=run

cmake -B build
if errorlevel 1 goto erro
cmake --build build --target %ALVO%
if errorlevel 1 goto erro

echo Pronto: %ALVO%.exe
if /i "%RODAR%"=="run" start "" "%ALVO%.exe"
exit /b 0

:erro
echo.
echo *** Erro na compilacao ***
exit /b 1

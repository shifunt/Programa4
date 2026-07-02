@echo off
REM ============================================================
REM Script de compilacion y empaquetado para Windows
REM Correr esto DESDE "Qt MinGW Command Prompt" (viene en el menu
REM inicio junto con Qt Creator) para que qmake y windeployqt
REM esten en el PATH automaticamente.
REM
REM Si tu Kit es MSVC en vez de MinGW, abri en cambio el
REM "Qt MSVC Command Prompt" o la consola x64 de Visual Studio.
REM ============================================================

set PROYECTO=SeguridadGolden
set BUILD_DIR=build-windows

echo Limpiando build anterior...
if exist %BUILD_DIR% rmdir /s /q %BUILD_DIR%
mkdir %BUILD_DIR%
cd %BUILD_DIR%

echo Corriendo qmake...
qmake ..\%PROYECTO%.pro CONFIG+=release
if errorlevel 1 goto error

echo Compilando...
mingw32-make -j4
if errorlevel 1 (
    echo Si el comando anterior fallo y usas MSVC en vez de MinGW, probar con: nmake
    goto error
)

echo.
echo Compilado. Buscando el ejecutable...
if not exist release\%PROYECTO%.exe (
    echo No se encontro release\%PROYECTO%.exe
    goto error
)

echo.
echo Empaquetando dependencias de Qt con windeployqt...
cd release
windeployqt %PROYECTO%.exe --release --no-translations

REM Copiar tambien el plugin de SQLite si windeployqt no lo agrega solo
REM (normalmente lo hace automaticamente al detectar QT += sql)

echo.
echo ============================================================
echo LISTO. La carpeta build-windows\release contiene el .exe
echo junto con todas las DLLs necesarias.
echo Comprimila en un .zip y ya es portable: se puede copiar a
echo cualquier PC Windows sin instalar Qt.
echo ============================================================
cd ..\..
goto fin

:error
echo.
echo Hubo un error. Revisa los mensajes de arriba.
cd ..

:fin
pause

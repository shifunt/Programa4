# SeguridadGolden

Sistema de control de acceso (entradas/salidas de vehículos) desarrollado en C++ con Qt Widgets y SQLite.

## Requisitos

- Qt 6.x (módulos: `core`, `gui`, `widgets`, `sql`)
- Compilador: MinGW o MSVC en Windows, GCC en Linux
- SQLite (ya viene incluido en Qt vía el plugin `QSQLITE`, no requiere instalación aparte)

## Estructura del proyecto

```
SeguridadGolden.pro
main.cpp
mainwindow.cpp
mainwindow.h
mainwindow.ui
deploy_linux.sh      -> compila y empaqueta en AppImage (Linux)
deploy_windows.bat   -> compila y empaqueta con windeployqt (Windows)
```

## Compilar en Linux

Opción A — Qt Creator: abrir `SeguridadGolden.pro` y darle a "Run" (Ctrl+R).

Opción B — Terminal, con el script:
```bash
chmod +x deploy_linux.sh
./deploy_linux.sh
```
Esto compila el proyecto y, si tenés `linuxdeployqt` instalado, genera un `.AppImage` portable en `build-linux/`.

Para instalar `linuxdeployqt`:
```bash
wget https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage
chmod +x linuxdeployqt-continuous-x86_64.AppImage
sudo mv linuxdeployqt-continuous-x86_64.AppImage /usr/local/bin/linuxdeployqt
```

## Compilar en Windows

1. Abrir el menú Inicio y buscar **"Qt <version> (MinGW 64-bit)"** — es una terminal que ya tiene qmake y windeployqt en el PATH. (Si tu Kit es MSVC, abrir la consola de Visual Studio en cambio).
2. Navegar hasta la carpeta del proyecto (`cd C:\ruta\a\SeguridadGolden`).
3. Correr:
   ```
   deploy_windows.bat
   ```
4. El resultado queda en `build-windows\release\`: el `.exe` + todas las DLLs de Qt necesarias.
5. Comprimir esa carpeta en un `.zip` — eso es lo portable que le pasás a cualquier PC Windows (no necesita tener Qt instalado).

## Notas importantes

- La base de datos SQLite se crea automáticamente en `Documentos/SeguridadGolden/seguridadGael.db` la primera vez que se abre la app (usa `QStandardPaths`, por eso es portable entre usuarios y sistemas operativos).
- No subir al repositorio ningún `.db` real con datos (ya está excluido en `.gitignore`).
- La contraseña de acceso está hardcodeada en `mainwindow.cpp` (`claveCorrecta = "1234"`) — considerar cambiarla o moverla a un archivo de configuración antes de distribuir la app a producción.

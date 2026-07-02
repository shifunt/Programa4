#!/bin/bash
# ============================================================
# Script de compilación y empaquetado para Linux (AppImage)
# Uso: ./deploy_linux.sh
# ============================================================
set -e  # corta el script si algo falla

PROYECTO="SeguridadGolden"
BUILD_DIR="build-linux"

echo "🔧 Limpiando build anterior..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "🔧 Corriendo qmake..."
qmake ../${PROYECTO}.pro CONFIG+=release

echo "🔨 Compilando (make)..."
make -j$(nproc)

# El ejecutable queda en build-linux/${PROYECTO}
if [ ! -f "$PROYECTO" ]; then
    echo "❌ No se generó el ejecutable. Revisá errores de compilación arriba."
    exit 1
fi

echo "✅ Compilado correctamente: $BUILD_DIR/$PROYECTO"

# ------------------------------------------------------------
# Empaquetado en AppImage (requiere linuxdeployqt)
# Descarga: https://github.com/probonopd/linuxdeployqt/releases
# ------------------------------------------------------------
if ! command -v linuxdeployqt &> /dev/null; then
    echo ""
    echo "⚠️  No se encontró 'linuxdeployqt' en el PATH."
    echo "    Descargalo con:"
    echo "    wget https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
    echo "    chmod +x linuxdeployqt-continuous-x86_64.AppImage"
    echo "    sudo mv linuxdeployqt-continuous-x86_64.AppImage /usr/local/bin/linuxdeployqt"
    echo ""
    echo "    Ya tenés el ejecutable compilado en: $BUILD_DIR/$PROYECTO"
    echo "    Podés correrlo directo, pero para distribuirlo portable te conviene el AppImage."
    exit 0
fi

echo "📦 Armando estructura AppDir..."
mkdir -p AppDir/usr/bin
cp "$PROYECTO" AppDir/usr/bin/

# Ícono placeholder si no hay uno (linuxdeployqt lo requiere)
if [ ! -f ../icon.png ]; then
    echo "⚠️  No se encontró icon.png en la raíz del proyecto, usando uno genérico."
    # Genera un ícono simple 256x256 si tenés ImageMagick, si no, avisá y creá uno manualmente
    convert -size 256x256 xc:#1b5e20 ../icon.png 2>/dev/null || echo "⚠️ Instalá imagemagick o poné tu icon.png manualmente."
fi
cp ../icon.png AppDir/ 2>/dev/null || true

cat > AppDir/${PROYECTO}.desktop <<EOF
[Desktop Entry]
Type=Application
Name=${PROYECTO}
Exec=${PROYECTO}
Icon=icon
Categories=Utility;
EOF

echo "📦 Generando AppImage..."
linuxdeployqt AppDir/${PROYECTO}.desktop -appimage

echo ""
echo "✅ Listo. Buscá el archivo *.AppImage en $BUILD_DIR/"
echo "   Para ejecutarlo en otra PC Linux: chmod +x *.AppImage && ./*.AppImage"

# LMN-3 DAW - Instalación y Compilación

## Instalación en Raspberry Pi

### 1. Copiar el archivo a la Raspberry Pi
```bash
scp LMN-3-aarch64-linux-gnu_0.6.2.zip pi@raspberrypi:~
```

### 2. En la Raspberry Pi, descomprimir
```bash
unzip LMN-3-aarch64-linux-gnu_0.6.2.zip
```

### 3. Dar permisos de ejecución
```bash
chmod +x LMN-3
```

### 4. Ejecutar la aplicación
```bash
./LMN-3
```

---

## Compilación Manual

### Opción 1: Usando el script (Recomendado)
```bash
bash docker-build.sh
```

Este script ejecuta automáticamente todos los pasos necesarios y genera el archivo ZIP.

### Opción 2: Compilación paso a paso con Docker

#### 1. Limpiar build anterior (opcional)
```bash
rm -rf build
```

#### 2. Inicializar submodules de Git
```bash
git submodule update --init --recursive
```

#### 3. Configurar el proyecto con CMake
```bash
export MSYS_NO_PATHCONV=1
docker run --rm -v "/d/DEV/LMN-3-DAW-iamdey-repo:/workspace" -w /workspace iamdey/lmn-3-daw-docker-compiler:arm64 bash -c "cmake -S . -B build -DCMAKE_BUILD_TYPE=Release"
```

#### 4. Compilar el proyecto
```bash
docker run --rm -v "/d/DEV/LMN-3-DAW-iamdey-repo:/workspace" -w /workspace iamdey/lmn-3-daw-docker-compiler:arm64 bash -c "cmake --build build -j8"
```

**Nota:** `-j8` usa 8 cores para compilar más rápido. Ajusta según tu CPU.

#### 5. Crear el archivo ZIP
```bash
cd build/LMN-3_artefacts/Release
zip -r ../../../LMN-3-aarch64-linux-gnu_0.6.2.zip LMN-3 ../../../LICENSE
cd ../../..
```

---

## Notas Técnicas

### Requisitos
- Docker instalado y corriendo
- Imagen Docker: `iamdey/lmn-3-daw-docker-compiler:arm64`
- Git con submodules inicializados

### Tiempos de Compilación
- Primera compilación: 10-15 minutos
- Compilaciones incrementales: 2-5 minutos

### Arquitectura
- Target: aarch64 (ARM 64-bit)
- Platform: Linux GNU
- Build Type: Release

### Solución de Problemas

#### Error: "Docker no está corriendo"
```bash
# Windows
# Abre Docker Desktop

# Linux
sudo systemctl start docker
```

#### Error: "Permission denied"
```bash
chmod +x docker-build.sh
```

#### Limpiar todo y empezar de cero
```bash
rm -rf build
git submodule update --init --recursive
bash docker-build.sh
```

---

## Cambios en esta versión (0.6.2)

### Arpeggiator para FourOsc
- Añadido arpeggiator con 5 modos: Off, Up, Down, Up-Down, Random
- Control de Rate (1-16 Hz), Octaves (1-4), Gate (0.05-1.0)
- Fix de notas colgadas al soltar teclas
- Accesible desde Tab 7 en FourOsc plugin

### Conocidos
- La configuración del arpeggiator no persiste entre sesiones (se perderá al cambiar de vista)
- Este es un problema conocido que se solucionará en una futura versión

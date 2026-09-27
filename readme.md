# Entorno de Trabajo

## Ubuntu:
```git clone https://github.com/curso-firmware-marvell/fw-10-2026```

```cd fw-10-2026```

```git submodule update --init```

### Actualizar
```sudo apt update && sudo apt upgrade -y```

### Instalar dependencias:
```sudo apt install -y verilator gtkwave python3 python3-pip```

### Buildear toolchain (riscv repo)
```sudo apt-get install autoconf automake autotools-dev curl python3-tomli libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex texinfo gperf libtool patchutils bc zlib1g-dev libexpat-dev ninja-build git cmake libglib2.0-dev libslirp-dev libncurses-dev```

```cd submodules/riscv-gnu-toolchain```

```git submodule update --init```

```./configure --prefix=/opt/riscv --enable-multilib```

#### Si usan WSL:

Crear un archivo .wslconfig (docs: https://learn.microsoft.com/es-mx/windows/wsl/wsl-config) en C:\Usuarios\\%usuario%\\.wslconfig con parámetros igual a la mitad de los disponibles en la computadora (por ejemplo, para 8 núcleos y 16GB de RAM), más al menos 8GB de swap (espacio en disco que se ocupará en C:).

```
[wsl2]
memory=8GB
processors=4
swap=16GB
```

Luego de crear el archivo, reiniciar la instancia WSL desde powershell (wsl --shutdown). Para el siguiente comando, modificar el "4" con la cantidad de procesadores asignados a WSL (también se puede checkear con el comando nproc).

```sudo make -j4```

```sudo make clean```

#### Si no usan WSL:

```sudo make -j8```

```sudo make clean```

## Compilación de FW:

Ir a directorio de FW.

```cd fw```

```make clean```

```make```

## Simulación de RTL:

Opciones:

```make```

```make clean```

```make fw```

```make sim```

```make wave```

```make build_project_config```

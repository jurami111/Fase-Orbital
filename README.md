# Fase Orbital

Proyecto 1A de Estructuras de Datos. Videojuego arcade hecho en C++ con Allegro 5.

## 1. Descripción

En el juego, una nave gira alrededor de un núcleo y dispara automáticamente a enemigos de tres colores: cian, magenta y ámbar. El jugador puede cambiar la dirección de la nave y escoger el color de los disparos.

Hay seis tipos de enemigos: Dron, Girador, Divisor, Mini, Tanque y Camaleón. También hay oleadas, combos, un pulso especial y partidas por turnos con hasta seis jugadores, que pueden ser personas o bots.

El juego guarda los resultados de las partidas y permite ver estadísticas y un ranking. Para organizar los datos se utilizaron listas, colas y otras estructuras hechas con nodos y punteros.

## 2. Ejecutar en Windows

Si el juego ya está compilado, solo hay que abrir `faseorbital.exe` desde la carpeta del proyecto. Es importante mantener las DLL de Allegro y la carpeta `sonidos` junto con los demás archivos necesarios.

Para compilarlo desde el código se necesita Windows de 64 bits, Visual Studio 2022 con C++ y Allegro 5.2.11.3 instalado mediante NuGet.

1. Abrir **x64 Native Tools Command Prompt for VS 2022**.
2. Entrar a la carpeta del proyecto:

```bat
cd /d C:\ruta\a\Fase-Orbital
```

3. Copiar y ejecutar este comando completo:

```bat
cl /nologo /EHsc /std:c++17 /I include /I include\Estructuras /I include\Motor /I include\Estadisticas /I include\Vista /I "%USERPROFILE%\.nuget\packages\allegro\5.2.11.3\build\native\include" src\main.cpp src\Vista\VistaAllegro.cpp src\Vista\Controlador.cpp src\Estructuras\*.cpp src\Motor\*.cpp src\Estadisticas\*.cpp /Fe:faseorbital.exe /link /LIBPATH:"%USERPROFILE%\.nuget\packages\allegro\5.2.11.3\build\native\v143\x64\lib" allegro.lib allegro_font.lib allegro_ttf.lib allegro_primitives.lib allegro_image.lib allegro_audio.lib allegro_acodec.lib
```

4. Abrir el juego:

```bat
faseorbital.exe
```

## 3. Controles

- **Flechas izquierda y derecha:** cambiar la dirección de la nave.
- **1, 2 y 3:** seleccionar el color cian, magenta o ámbar.
- **Espacio:** usar el pulso cuando esté disponible.
- **P:** pausar o continuar la partida.
- **Mouse:** escoger las opciones del menú.
- **Esc:** regresar al menú o salir, según la pantalla.

Desde el menú se puede jugar, ver las estadísticas, consultar el ranking o salir. Antes de empezar se pueden agregar hasta seis jugadores. Los nombres que comienzan con `BOT`, como `BOT1`, se controlan automáticamente.

## 4. Partidas automáticas

También se pueden hacer partidas con bots desde la consola, sin abrir la ventana del juego.

En Windows:

```bat
faseorbital.exe --lotes --jugadores BOT1,BOT2 --semilla 42 --oleadas 10
```

- `--jugadores`: nombres de los participantes separados por comas.
- `--semilla`: número que permite repetir una simulación.
- `--oleadas`: cantidad máxima de oleadas, entre 1 y 10.
- `--historial`: permite guardar los resultados en otro archivo CSV.

Al terminar, los resultados se guardan en `historial.csv`, a menos que se indique otro archivo.

## 5. Historial, estadísticas y ranking

El archivo `historial.csv` guarda los resultados de las partidas. Desde el menú se pueden consultar las estadísticas de los jugadores y el ranking.

También se pueden ver desde la consola de Windows:

```bat
faseorbital.exe --estadisticas historial.csv
faseorbital.exe --ranking historial.csv
```

Para abrir el juego usando el archivo de prueba:

```bat
faseorbital.exe --historial tests\historial_prueba.csv
```

El archivo `tests/historial_prueba.csv` contiene datos de ejemplo para realizar pruebas.

## 6. Pruebas

En Windows, desde la carpeta principal del proyecto, se pueden ejecutar:

```bat
Pruebas.exe historial_pruebas.csv
PruebasASan.exe historial_asan.csv
```

Estas pruebas permiten revisar el funcionamiento de las estructuras de datos y el guardado de información.

## 7. Compilar y ejecutar en macOS

Primero, instalar Allegro y las herramientas necesarias:

```sh
brew install allegro pkgconf
```

Después, desde la carpeta del proyecto:

```sh
make
./build/faseorbital
```

También se puede utilizar:

```sh
make ejecutar
```

Para ejecutar las pruebas:

```sh
make pruebas
```

Para simular una partida con bots:

```sh
./build/faseorbital --lotes --jugadores BOT1,BOT2 --semilla 42 --oleadas 10
```

Para consultar las estadísticas y el ranking:

```sh
./build/faseorbital --estadisticas historial.csv
./build/faseorbital --ranking historial.csv
```

## 8. Capturas del juego

El programa permite guardar imágenes de algunas pantallas del juego.

En Windows:

```bat
mkdir build\capturas
faseorbital.exe --capturas build\capturas --historial tests\historial_prueba.csv
```

En macOS:

```sh
mkdir -p build/capturas
./build/faseorbital --capturas build/capturas --historial tests/historial_prueba.csv
```

Se generan cuatro imágenes: `principal.png`, `jugar.png`, `estadisticas.png` y `ranking.png`.

## 9. Archivos del proyecto

El proyecto incluye el código en `src` e `include`, las pruebas, los sonidos, el archivo `Makefile`, el historial de partidas y este README.

Para ejecutar la versión de Windows sin compilar, se debe conservar `faseorbital.exe` junto con las DLL y los recursos que necesita el juego.

# Fase Orbital

Proyecto 1A de Estructuras de Datos — videojuego arcade desarrollado en C++ con Allegro.

## 1. Descripción

La nave gira alrededor de un núcleo y dispara automáticamente contra enemigos cian, magenta y ámbar. El jugador selecciona la dirección y el color de disparo. Incluye seis tipos de enemigos (Dron, Girador, Divisor, Mini, Tanque y Camaleón), oleadas deterministas, combos, pulso, partidas por turnos de uno a seis participantes humanos o bots, bitácora de eventos, historial, estadísticas y ranking.

Las estructuras de datos principales están implementadas con nodos y punteros propios: listas de enemigos y balas, cola de aparición, ronda circular de jugadores, bitácora, historial y ranking.

## 2. Compilación y ejecución en Windows (entorno verificado)

**Requisitos:** Windows x64, Visual Studio 2022 con compilador de C++ y Allegro 5.2.11.3 instalado mediante NuGet. La ruta usada a continuación corresponde a la instalación NuGet en `%USERPROFILE%`; si Allegro está en otra carpeta, hay que ajustarla.

1. Abrir **x64 Native Tools Command Prompt for VS 2022**.
2. Situarse en la carpeta raíz del proyecto, que contiene `src`, `include` y `README.md`:

```bat
cd /d C:\ruta\a\Fase-Orbital
```

3. Ejecutar el siguiente comando completo (es una sola instrucción; las líneas que se muestran aquí son solo ajuste visual del editor):

```bat
cl /nologo /EHsc /std:c++17 /I include /I include\Estructuras /I include\Motor /I include\Estadisticas /I include\Vista /I "%USERPROFILE%\.nuget\packages\allegro\5.2.11.3\build\native\include" src\main.cpp src\Vista\VistaAllegro.cpp src\Vista\Controlador.cpp src\Estructuras\*.cpp src\Motor\*.cpp src\Estadisticas\*.cpp /Fe:faseorbital.exe /link /LIBPATH:"%USERPROFILE%\.nuget\packages\allegro\5.2.11.3\build\native\v143\x64\lib" allegro.lib allegro_font.lib allegro_ttf.lib allegro_primitives.lib allegro_image.lib allegro_audio.lib allegro_acodec.lib
```

4. Ejecutar el juego:

```bat
faseorbital.exe
```

Las DLL de Allegro requeridas deben estar junto al ejecutable o en una carpeta incluida en `PATH`. Si aparece un error de DLL faltante, revisar la instalación de Allegro y la carpeta `build\native\v143\x64\bin` del paquete, según corresponda; no descargar DLL de sitios externos.

**Verificado en Windows:** el comando anterior compiló el juego y se abrió el menú principal. También se comprobó el modo por lotes.

## 3. Controles y modos de juego

- **Flechas izquierda/derecha:** cambiar la dirección de la nave.
- **1, 2, 3:** seleccionar cian, magenta o ámbar.
- **Espacio:** activar el pulso cuando esté cargado.
- **P:** pausar o reanudar.
- **Mouse:** seleccionar opciones del menú.

Desde el menú principal se puede **Jugar**, consultar **Ranking**, consultar **Estadísticas** o **Salir**. En el registro de jugadores se agregan hasta seis participantes. Los nombres que comienzan con `BOT` (por ejemplo, `BOT1`) corresponden a participantes automáticos.

## 4. Simulación automática (modo por lotes)

Permite simular partidas desde la consola sin abrir la ventana gráfica. Los participantes `BOT...` actúan automáticamente. Los jugadores humanos no reciben controles interactivos en este modo.

**Windows — comando verificado:**

```bat
faseorbital.exe --lotes --jugadores BOT1,BOT2 --semilla 42 --oleadas 10
```

**macOS — comando equivalente, pendiente de comprobar en una Mac:**

```sh
./build/faseorbital --lotes --jugadores BOT1,BOT2 --semilla 42 --oleadas 10
```

- `--jugadores`: alias separados por comas.
- `--semilla`: valor para reproducir una simulación; si se omite, se utiliza la hora actual.
- `--oleadas`: cantidad máxima de oleadas solicitadas, de 1 a 10. La partida puede terminar antes por derrota.
- `--historial`: permite indicar otro archivo CSV de historial; si se omite, se usa `historial.csv`.

La consola muestra resultado, oleada alcanzada, puntajes y línea de tiempo de los jugadores. Al terminar, guarda la partida en `historial.csv` (o en el archivo indicado).

**Verificado:** con semilla 42 y dos bots, la simulación terminó en derrota en la oleada 7 de 10 y confirmó `Partida guardada en historial.csv`.

## 5. Estadísticas, ranking e historial

Los resultados se almacenan en `historial.csv`. La interfaz permite consultar estadísticas por jugador, historial y clasificación histórica.

El programa también incluye reportes sin interfaz gráfica. **Estos comandos están implementados, pero falta comprobar su ejecución en el ejecutable final de Windows:**

```bat
faseorbital.exe --estadisticas historial.csv
faseorbital.exe --ranking historial.csv
```

Para consultar un historial distinto sin modificar el principal:

```bat
faseorbital.exe --historial tests\historial_prueba.csv
```

`tests/historial_prueba.csv` contiene datos sintéticos de prueba; no debe presentarse como evidencia de partidas reales.

## 6. Pruebas

En Windows, desde la raíz del proyecto:

```bat
Pruebas.exe historial_pruebas.csv
PruebasASan.exe historial_asan.csv
```

Ambos ejecutables de pruebas fueron utilizados y reportaron `Todas las pruebas ejecutadas pasaron`. La ejecución de AddressSanitizer no mostró un diagnóstico de error en la salida compartida. La comprobación mediante Valgrind no se ejecutó en Windows y figura como `SKIP`.

Para repetir las pruebas desde el código en macOS (o en un entorno con `make` y compilador compatible):

```sh
make pruebas
```

El objetivo `pruebas` del Makefile compila `build/pruebas` y lo ejecuta con un historial temporal.

## 7. Compilación y ejecución en macOS (instrucciones conservadas, no verificadas aquí)

Instalar las dependencias mediante Homebrew:

```sh
brew install allegro pkgconf
```

Desde la raíz del repositorio:

```sh
make
./build/faseorbital
```

Alternativamente:

```sh
make ejecutar
```

El Makefile utiliza C++17 y `pkg-config` para localizar Allegro. La disponibilidad de sus complementos y la compilación deben verificarse en el equipo macOS de destino.

Para los reportes de consola en macOS:

```sh
./build/faseorbital --estadisticas historial.csv
./build/faseorbital --ranking historial.csv
```

## 8. Persistencia y detalles técnicos

La partida registra resultados de todos los participantes, incluidos los eliminados, mediante `Persistencia::guardarPartidaCompleta`. El guardado valida los registros y utiliza un archivo temporal antes de reemplazar el CSV. Los reportes pueden consultar el historial sin reescribirlo.

El formato actual del CSV contiene 36 columnas: 21 originales y 15 adicionales para impactos, bajas por color y daño por tipo. El sistema admite CSV antiguos de 21 columnas y los migra al formato actual al guardar una partida nueva, conservando los registros anteriores y asignando cero a las métricas que no existían.

La bitácora registra bajas, impactos erróneos, daños, choques, pulsos, fin de oleada y eliminaciones. `Estadisticas::calcularResumenHistorico` devuelve el resumen por valor; la variante `generarResumenHistorico` devuelve un puntero que debe liberarse con `delete`. Las funciones de resumen y línea de tiempo evitan duplicar los eventos de daño.

## 9. Capturas automáticas de verificación (función disponible; pendiente de probar en Windows)

Desde la carpeta raíz, crear el directorio de salida y ejecutar:

```bat
mkdir build\capturas
faseorbital.exe --capturas build\capturas --historial tests\historial_prueba.csv
```

La función está diseñada para producir `principal.png`, `jugar.png`, `estadisticas.png` y `ranking.png`, sin alterar el historial de prueba. En macOS, el comando equivalente es:

```sh
mkdir -p build/capturas
./build/faseorbital --capturas build/capturas --historial tests/historial_prueba.csv
```

## 10. Archivos de entrega

Conservar `src/`, `include/`, `tests/`, `Makefile`, `README.md`, los recursos necesarios, `faseorbital.exe`, las DLL requeridas para Windows, el programa de pruebas, el historial solicitado, el manual de usuario, la documentación técnica y el video de defensa. Evitar entregar ejecutables duplicados de verificación como si fueran versiones principales.

**Nota:** los comandos de Windows indicados como verificados se probaron en el entorno de desarrollo; los reportes y las capturas automáticas requieren las comprobaciones finales descritas arriba. Las instrucciones de macOS se conservaron del proyecto, pero no se ejecutaron en una Mac durante esta revisión.

# Fase-Orbital

## Estado Actual

El juego en Allegro incluye registro de nombres unicos, partidas por turnos de
hasta seis jugadores, oleadas deterministas, seis tipos de enemigo, puntaje/combo,
pulso, daño al nucleo, bitacora, estadisticas historicas y ranking. Los alias
`BOT...` se marcan en el historial, y tienen control automatico.

## Ejecucion Por Lotes

Simula partidas sin ventana y guarda el resultado en el historial (`historial.csv`
por defecto, o el indicado con `--historial`):

```sh
./build/faseorbital --lotes --jugadores Ana,BOT1 --semilla 42 --oleadas 10
```

- `--jugadores` lista los alias separados por comas; los `BOT...` usan control automatico.
- `--semilla` fija la semilla; sin ella se usa la hora actual.
- `--oleadas` va de 1 a 10.
- Los jugadores humanos no disparan en lotes: solo los `BOT...` juegan.
- La partida se guarda con modo `Lotes`, igual que el guardado de la GUI.

Pendiente: scripts de acciones para jugadores humanos.

## Compilar Y Ejecutar En macOS

Instalar dependencias una vez:

```sh
brew install allegro pkgconf
```

Desde la raiz del repositorio:

```sh
make
./build/faseorbital
# O directamente:
make ejecutar
```

Los botones funcionan con mouse. Escape vuelve al menu desde el juego y los reportes,
o cierra la ventana desde el menu. Volver regresa sin recrear la ventana.
En estadisticas, las flechas cambian de jugador. Una fila del ranking abre sus estadisticas.
En la pantalla principal, las flechas mueven la nave como en el main original.
Si no existe la fuente de Windows, se usa la fuente incorporada de Allegro.

## Datos De Prueba

Este CSV contiene datos sinteticos para pruebas, no evidencia de partidas jugadas:

```sh
./build/faseorbital --historial tests/historial_prueba.csv
```

## Pruebas

```sh
make pruebas
```

Los reportes por consola no abren ventana:

```sh
./build/faseorbital --estadisticas historial.csv
./build/faseorbital --ranking historial.csv
```

Un archivo ausente o con encabezado incorrecto produce un aviso; las
filas invalidas se informan y omiten. Abrir reportes no escribe el historial.

## Partida Y Persistencia

La partida actual asigna posiciones y llama a `Persistencia::guardarPartidaCompleta`
con todos los jugadores, incluidos los eliminados. El guardado valida las filas,
escribe un archivo temporal y reemplaza el CSV solo cuando todas las escrituras
terminan bien. El historial y el ranking se actualizan despues de guardar.

`Estadisticas::calcularResumenHistorico` devuelve datos por valor, incluyendo
precision y bajas por color y dano por tipo. La variante
`generarResumenHistorico` devuelve un puntero que el llamador debe liberar con `delete`.
La precision historica es el promedio de precisiones por partida; una partida
sin impactos aporta cero. `calcularResumenOleada` e `imprimirLineaTiempo` usan
eventos BAJA cuyo valor es el puntaje ganado, y eventos de dano sin duplicarlos.

El CSV actual tiene 36 columnas: las 21 originales mas impactos y bajas por color
y daño recibido por tipo.
Los CSV antiguos de 21 columnas se pueden cargar; se migran al formato actual
cuando se guarda una partida nueva. La migracion mantiene sus registros antiguos
y deja en cero las metricas que ese formato no almacenaba.

La bitacora de eventos se mantiene durante la partida. Registra bajas, impactos
erroneos, dano al nucleo, choques, pulsos, fin de oleada y eliminaciones. Al final
se imprime la linea de tiempo de cada jugador; el historial y las columnas nuevas
guardan las metricas necesarias para los reportes posteriores.

## Capturas De Verificacion

```sh
mkdir -p build/capturas
./build/faseorbital --capturas build/capturas --historial tests/historial_prueba.csv
```

Genera principal.png (menu), jugar.png, estadisticas.png y ranking.png.
El recorrido automatizado registra un jugador, agrega y comienza una partida,
y cierra despues del primer tick sin modificar el historial proporcionado.
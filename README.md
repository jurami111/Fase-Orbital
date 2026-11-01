# Fase-Orbital

## Estado Actual

El main original conserva su dibujo, controles, fuente TTF y temporizador.
La pantalla inicial tiene cuatro botones: Jugar, Ranking, Estadisticas y Salir.
Jugar muestra el prototipo original; los reportes usan la misma ventana.
El prototipo de la orbita no es una partida completa y no guarda resultados.
El motor, bots, scripts, resumen final y pantallas ligadas a partidas aun necesitan integracion.

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

El main original usa Allegro; ya no acepta --estadisticas o --ranking por consola.
El objetivo de compilacion de consola necesita un punto de entrada separado antes de usarse.
Un archivo ausente o con encabezado incorrecto
produce un error; las filas invalidas se informan y omiten. La ventana muestra
un aviso si no pudo cargar el historial. Abrir reportes no escribe el archivo.

## Integracion Con El Motor

Al finalizar una partida, asignar posiciones de 1 a cantidad y llamar a
`Persistencia::guardarPartidaCompleta` con todos los jugadores, incluidos los eliminados,
el historial cargado, semilla y modo. Genera un id comun y actualiza el historial
despues de cada escritura exitosa. Reconstruir el ranking despues con
`Ranking::ConstruirDesdeHistorial`. Si falla una escritura, no reintentar a ciegas:
las filas anteriores pueden haberse guardado; no es una transaccion atomica.

`Estadisticas::calcularResumenHistorico` devuelve datos por valor. La variante
`generarResumenHistorico` devuelve un puntero que el llamador debe liberar con `delete`.
La precision historica es el promedio de precisiones por partida; una partida
sin impactos aporta cero. `calcularResumenOleada` e `imprimirLineaTiempo` usan
eventos BAJA cuyo valor es el puntaje ganado, y eventos de dano sin duplicarlos.

El CSV conserva las 21 columnas del enunciado. Precision por color y dano por tipo
no pueden reconstruirse historicamente con ese formato: falta acordar su persistencia.

## Capturas De Verificacion

```sh
mkdir -p build/capturas
./build/faseorbital --capturas build/capturas --historial tests/historial_prueba.csv
```

Genera principal.png (menu), jugar.png, estadisticas.png y ranking.png.
Comprueba los clics sobre los botones del main y el retorno, y cierra sin modificar el historial.
#include "Oleada.h"
#include "Generador.h"

#include <limits>
#include <stdexcept>

ColaSpawn Oleada::Generar(std::uint32_t semillaPartida, int numeroOleada) const {
    if (numeroOleada < 1 ||
        numeroOleada > (std::numeric_limits<int>::max() - 6) / 3) {
        throw std::invalid_argument("El número de oleada debe ser positivo y válido.");
    }

    TipoEnemigo tiposDisponibles[7] = {
        TipoEnemigo::Dron, TipoEnemigo::Dron, TipoEnemigo::Girador,
        TipoEnemigo::Divisor, TipoEnemigo::Tanque, TipoEnemigo::Camaleon,
        TipoEnemigo::Girador
    };
    int cantidadTipos = 2;
    if (numeroOleada >= 2) cantidadTipos++;
    if (numeroOleada >= 3) cantidadTipos++;
    if (numeroOleada >= 4) cantidadTipos++;
    if (numeroOleada >= 5) cantidadTipos += 2;

    const std::uint32_t semillaOleada =
        semillaPartida * 1000u + static_cast<std::uint32_t>(numeroOleada);
    Generador generador(semillaOleada);
    ColaSpawn cola;
    int tickAparicion = 60;
    const int cantidadApariciones = 6 + 3 * numeroOleada;
    const int intervaloBase = numeroOleada * 2 >= 26
        ? 9
        : (35 - 2 * numeroOleada < 9 ? 9 : 35 - 2 * numeroOleada);

    for (int i = 0; i < cantidadApariciones; i++) {
        DatosSpawn datos{};
        datos.tick = tickAparicion;
        datos.tipo = tiposDisponibles[generador.siguiente() % cantidadTipos];
        datos.color = static_cast<int>(generador.siguiente() % 3u);
        datos.angulo = static_cast<double>(generador.siguiente() % 360u);
        datos.sentido = false;
        if (datos.tipo == TipoEnemigo::Girador) {
            datos.sentido = generador.siguiente() % 2u == 0u;
        }

        SpawnPtr spawn = &datos;
        cola.Encolar(spawn);

        if (i + 1 < cantidadApariciones) {
            const std::uint32_t variacion = generador.siguiente();
            if (numeroOleada >= 7 && variacion % 4u == 0u) {
                continue;
            }
            tickAparicion += intervaloBase *
                static_cast<int>(60u + variacion % 81u) / 100;
        }
    }

    return cola;
}
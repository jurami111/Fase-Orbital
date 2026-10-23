#pragma once

#include <string>
#include "Historial.h"
#include "../Estructuras/RondaJugadores.h"

class Persistencia {
    public:
        static const char* const FORMATO_CSV;

        bool cargarHistorial(const std::string& ruta, Historial* historial);
        bool guardarPartida(const std::string& ruta, JugadorPtr jugador,
                            int idPartida, int semilla, const std::string& modo);
};
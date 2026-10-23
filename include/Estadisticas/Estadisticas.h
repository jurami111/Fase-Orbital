#pragma once

#include <string>
#include "BitacoraEventos.h"
#include "Historial.h"

class Estadisticas {
    public:
        double calcularMedia(Historial* historial, const std::string& alias);
        double calcularMediana(Historial* historial, const std::string& alias);
        double calcularDesviacionEstandar(Historial* historial, const std::string& alias);

        double calcularPrecisionPromedio(BitacoraEventos* bitacora, const std::string& alias);
        int obtenerColorMejorPrecision(BitacoraEventos* bitacora, const std::string& alias);
        TipoEnemigo obtenerEnemigoMasDanino(BitacoraEventos* bitacora, const std::string& alias);

        void imprimirHistogramaASCII(Historial* historial, const std::string& alias);
};
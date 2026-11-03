#pragma once

#include <string>
#include "BitacoraEventos.h"
#include "Historial.h"

typedef struct ResumenHistorico{
    char alias[25];
    int partidas_jugadas;
    int cantidad_victorias;
    int mejor_puntaje;
    int peor_puntaje;
    int oleada_maxima;
    double media;
    double mediana;
    double desviacion_estandar;
    double precision_promedio;
    int correctos_color[3]{};
    int erroneos_color[3]{};
    int bajas_color[3]{};
    int dano_por_tipo[6]{};
} *ResumenHistoricoPtr;

struct ResultadoHistograma {
    int minimo = 0;
    int maximo = 0;
    int cantidad = 0;
    int frecuencias[10]{};
};

struct ResumenOleada {
    int bajas = 0;
    int dano = 0;
    int puntos = 0;
    bool tiene_eventos = false;
};

class Estadisticas {
    public:

        

        void Histograma(Historial* historial, const std::string& alias);
        ResumenHistoricoPtr generarResumenHistorico(Historial* historial, const std::string& alias);
        ResumenHistorico calcularResumenHistorico(Historial* historial, const std::string& alias);
        double calcularPrecisionHistorica(Historial* historial, const std::string& alias);
        double calcularPrecisionColorHistorica(Historial* historial, const std::string& alias, int color);
        TipoEnemigo obtenerEnemigoMasDanino(Historial* historial, const std::string& alias);
        ResultadoHistograma calcularHistograma(Historial* historial, const std::string& alias);
        void imprimirReporteHistorico(Historial* historial, const std::string& alias);
        ResumenOleada calcularResumenOleada(BitacoraEventos* bitacora, JugadorPtr jugador, int oleada);
        void imprimirLineaTiempo(BitacoraEventos* bitacora, JugadorPtr jugador);
        double calcularMedia(Historial* historial, const std::string& alias);
        double calcularMediana(Historial* historial, const std::string& alias);
        double calcularDesviacionEstandar(Historial* historial, const std::string& alias);

        double calcularPrecisionPromedio(BitacoraEventos* bitacora, const std::string& alias);
        int obtenerColorMejorPrecision(BitacoraEventos* bitacora, const std::string& alias);
        TipoEnemigo obtenerEnemigoMasDanino(BitacoraEventos* bitacora, const std::string& alias);

        void imprimirHistogramaASCII(Historial* historial, const std::string& alias);
};
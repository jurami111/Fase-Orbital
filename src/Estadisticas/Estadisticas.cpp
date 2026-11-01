#include "Estadisticas.h"

#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>

namespace {


bool coincideAlias(const char* jugador, const std::string& alias) {
    return jugador != nullptr && alias == jugador;
}

void liberarNodos(NodoHistorialCSVPtr nodo) {
    while (nodo != nullptr) {
        NodoHistorialCSVPtr siguiente = nodo->siguiente;
        delete nodo;
        nodo = siguiente;
    }
}

int indiceEnemigo(TipoEnemigo tipo) {
    switch (tipo) {
        case TipoEnemigo::Dron: return 0;
        case TipoEnemigo::Girador: return 1;
        case TipoEnemigo::Divisor: return 2;
        case TipoEnemigo::Mini: return 3;
        case TipoEnemigo::Tanque: return 4;
        case TipoEnemigo::Camaleon: return 5;
    }
    return -1;
}
}


double Estadisticas::calcularMedia(Historial* historial, const std::string& alias) {
    if (historial == nullptr) return 0.0;
    double suma = 0.0;
    int cantidad = 0;
    for (NodoHistorialCSVPtr nodo = historial->primero();
        nodo != nullptr; nodo = nodo->siguiente) {
        if (nodo->dato != nullptr && alias == nodo->dato->jugador) {
            suma += nodo->dato->puntaje;
            cantidad++;
        }
    }
    return cantidad == 0 ? 0.0 : suma / cantidad;
}

double Estadisticas::calcularMediana(Historial* historial, const std::string& alias) {
    if (historial == nullptr) return 0.0;
    NodoHistorialCSVPtr copia = nullptr;
    int cantidad = 0;
    try {
        for (NodoHistorialCSVPtr nodo = historial->primero();
             nodo != nullptr; nodo = nodo->siguiente) {
            if (nodo->dato != nullptr && alias == nodo->dato->jugador) {
                copia = new NodoHistorialCSV{nullptr, nodo->dato->puntaje, copia};
                cantidad++;
            }
        }
    } catch (...) {
        liberarNodos(copia);
        throw;
    }
    if (cantidad == 0) return 0.0;
    historial->MergeSort(copia);
    NodoHistorialCSVPtr central = copia;
    for (int indice = 0; indice < (cantidad - 1) / 2; indice++) {
        central = central->siguiente;
    }
    double mediana = central->puntaje;
    if (cantidad % 2 == 0) {
        mediana = (mediana + central->siguiente->puntaje) / 2.0;
    }
    liberarNodos(copia);
    return mediana;
}

double Estadisticas::calcularDesviacionEstandar(Historial* historial, const std::string& alias) {
    if (historial == nullptr) return 0.0;
    double suma = 0.0;
    int cantidad = 0;
    //Usado para calcular la desviación estándar de los puntajes del alias
    for (NodoHistorialCSVPtr nodo = historial->primero();
        nodo != nullptr; nodo = nodo->siguiente) { // Calcular suma y cantidad de puntajes para el alias
        if (nodo->dato != nullptr && alias == nodo->dato->jugador) {
            suma += nodo->dato->puntaje; // Sumar puntaje para el alias
            cantidad++;
        }
        }

    if (cantidad == 0) return 0.0;
    const double media = suma / cantidad;
    double sumaCuadrados = 0.0;
    //Usando la media calculada previamente, se calcula la suma de los cuadrados de las diferencias respecto a la media
    for (NodoHistorialCSVPtr nodo = historial->primero();
        nodo != nullptr; nodo = nodo->siguiente) { // Calcular suma de cuadrados de diferencias respecto a la media
        if (nodo->dato != nullptr && alias == nodo->dato->jugador) {
            const double diferencia = nodo->dato->puntaje - media; // Diferencia respecto a la media
            sumaCuadrados += diferencia * diferencia; // Sumar el cuadrado de la diferencia respecto a la media
        }
    }
    return std::sqrt(sumaCuadrados / cantidad);
}

double Estadisticas::calcularPrecisionPromedio(BitacoraEventos* bitacora, const std::string& alias) {
    if (bitacora == nullptr) return 0.0;

    for (NodoEventoPtr nodo = bitacora->primero(); nodo != nullptr; nodo = nodo->siguiente) {
        if (nodo->dato == nullptr || nodo->dato->jugador == nullptr ||
            !coincideAlias(nodo->dato->jugador->Alias, alias)) continue;
        JugadorPtr jugador = nodo->dato->jugador;
        const double impactos = static_cast<double>(jugador->correctos) + jugador->erroneos;
        return impactos == 0.0 ? 0.0 : 100.0 * jugador->correctos / impactos;
    }
    return 0.0;
}

int Estadisticas::obtenerColorMejorPrecision(BitacoraEventos* bitacora, const std::string& alias) {
    if (bitacora == nullptr) return -1;
    for (NodoEventoPtr nodo = bitacora->primero(); nodo != nullptr; nodo = nodo->siguiente) {
        if (nodo->dato == nullptr || nodo->dato->jugador == nullptr ||
            !coincideAlias(nodo->dato->jugador->Alias, alias)) continue;
        JugadorPtr jugador = nodo->dato->jugador;
        int mejorColor = -1;
        double mejorPrecision = -1.0;
        for (int color = 0; color < 3; color++) {
            const double impactos = static_cast<double>(jugador->correctos_color[color]) +
                                    jugador->erroneos_color[color];
            if (impactos == 0.0) continue;
            const double precision = jugador->correctos_color[color] / impactos;
            if (precision > mejorPrecision) {
                mejorPrecision = precision;
                mejorColor = color;
            }
        }
        return mejorColor;
    }
    return -1;
}

TipoEnemigo Estadisticas::obtenerEnemigoMasDanino(BitacoraEventos* bitacora, const std::string& alias) {
    if (bitacora == nullptr) return TipoEnemigo::Dron;

    long long danoPorEnemigo[6] = {};
    for (NodoEventoPtr nodo = bitacora->primero(); nodo != nullptr; nodo = nodo->siguiente) {
        if (nodo->dato == nullptr || nodo->dato->jugador == nullptr ||
            !coincideAlias(nodo->dato->jugador->Alias, alias) ||
            (nodo->dato->tipo_evento != TipoEvento::DANO_NUCLEO &&
             nodo->dato->tipo_evento != TipoEvento::CHOQUE_NAVE)) continue;

        const int indice = indiceEnemigo(nodo->dato->tipo_enemigo);
        if (indice >= 0 && nodo->dato->valor > 0) {
            danoPorEnemigo[indice] += nodo->dato->valor;
        }
    }

    int mayorIndice = 0;
    for (int i = 1; i < 6; i++) {
        if (danoPorEnemigo[i] > danoPorEnemigo[mayorIndice]) mayorIndice = i;
    }
    return static_cast<TipoEnemigo>(mayorIndice);
}

ResultadoHistograma Estadisticas::calcularHistograma(Historial* historial, const std::string& alias) {
    ResultadoHistograma resultado;
    if (historial != nullptr) {
        for (NodoHistorialCSVPtr nodo = historial->primero();
             nodo != nullptr; nodo = nodo->siguiente) {
            if (nodo->dato == nullptr || alias != nodo->dato->jugador) continue;
            const int puntaje = nodo->dato->puntaje;
            if (resultado.cantidad == 0 || puntaje < resultado.minimo) resultado.minimo = puntaje;
            if (resultado.cantidad == 0 || puntaje > resultado.maximo) resultado.maximo = puntaje;
            resultado.cantidad++;
        }
    }
    if (resultado.cantidad == 0) return resultado;

    const double ancho = (static_cast<double>(resultado.maximo) - resultado.minimo) / 10.0;

    for (NodoHistorialCSVPtr nodo = historial->primero();
         nodo != nullptr; nodo = nodo->siguiente) {
        if (nodo->dato == nullptr || alias != nodo->dato->jugador) continue;
        int indice = ancho == 0.0 ? 0 : static_cast<int>(
            (static_cast<double>(nodo->dato->puntaje) - resultado.minimo) / ancho);
        if (indice >= 10) indice = 9;
        resultado.frecuencias[indice]++;
    }
    return resultado;
}

void Estadisticas::imprimirHistogramaASCII(Historial* historial, const std::string& alias) {
    const ResultadoHistograma resultado = calcularHistograma(historial, alias);
    if (resultado.cantidad == 0) {
        std::cout << "No hay puntajes para " << alias << ".\n";
        return;
    }
    const double ancho = (static_cast<double>(resultado.maximo) - resultado.minimo) / 10.0;
    std::cout << "Histograma de puntajes para " << alias << ":\n";
    for (int i = 0; i < 10; i++) {
        const double inicio = resultado.minimo + i * ancho;
        const double fin = i == 9 ? resultado.maximo : resultado.minimo + (i + 1) * ancho;
        std::cout << '[' << std::fixed << std::setprecision(1) << inicio << ", "
                  << fin << (i == 9 ? "] " : ") ") << std::string(resultado.frecuencias[i], '*')
                  << " (" << resultado.frecuencias[i] << ")\n";
    }
}

void Estadisticas::Histograma(Historial* historial, const std::string& alias) {
    imprimirHistogramaASCII(historial, alias);
}

double Estadisticas::calcularPrecisionHistorica(Historial* historial, const std::string& alias) {
    if (historial == nullptr) return 0.0;
    double suma = 0.0;
    int cantidad = 0;
    for (NodoHistorialCSVPtr nodo = historial->primero(); nodo != nullptr; nodo = nodo->siguiente) {
        if (alias != nodo->dato->jugador) continue;
        const double impactos = static_cast<double>(nodo->dato->correctos) + nodo->dato->erroneos;
        if (impactos > 0.0) suma += 100.0 * nodo->dato->correctos / impactos;
        cantidad++;
    }
    return cantidad == 0 ? 0.0 : suma / cantidad;
}

ResumenHistorico Estadisticas::calcularResumenHistorico(Historial* historial, const std::string& alias) {
    ResumenHistorico resumen{};
    if (historial == nullptr || alias.size() >= sizeof(resumen.alias)) return resumen;
    std::strcpy(resumen.alias, alias.c_str());
    for (NodoHistorialCSVPtr nodo = historial->primero(); nodo != nullptr; nodo = nodo->siguiente) {
        HistorialCSVPtr registro = nodo->dato;
        if (alias != registro->jugador) continue;
        if (resumen.partidas_jugadas == 0 || registro->puntaje > resumen.mejor_puntaje) resumen.mejor_puntaje = registro->puntaje;
        if (resumen.partidas_jugadas == 0 || registro->puntaje < resumen.peor_puntaje) resumen.peor_puntaje = registro->puntaje;
        resumen.partidas_jugadas++;
        if (registro->posicion == 1) resumen.cantidad_victorias++;
        if (registro->oleada > resumen.oleada_maxima) resumen.oleada_maxima = registro->oleada;
    }
    resumen.media = calcularMedia(historial, alias);
    resumen.mediana = calcularMediana(historial, alias);
    resumen.desviacion_estandar = calcularDesviacionEstandar(historial, alias);
    resumen.precision_promedio = calcularPrecisionHistorica(historial, alias);
    return resumen;
}

ResumenHistoricoPtr Estadisticas::generarResumenHistorico(Historial* historial, const std::string& alias) {
    return new ResumenHistorico(calcularResumenHistorico(historial, alias));
}

void Estadisticas::imprimirReporteHistorico(Historial* historial, const std::string& alias) {
    const ResumenHistorico resumen = calcularResumenHistorico(historial, alias);
    if (resumen.partidas_jugadas == 0) {
        std::cout << "No hay partidas para " << alias << ".\n";
        return;
    }
    std::cout << "\nJugador: " << resumen.alias
              << "\nPartidas: " << resumen.partidas_jugadas << "  Victorias: " << resumen.cantidad_victorias
              << "\nMejor: " << resumen.mejor_puntaje << "  Peor: " << resumen.peor_puntaje
              << "  Oleada maxima: " << resumen.oleada_maxima << std::fixed << std::setprecision(2)
              << "\nMedia: " << resumen.media << "  Mediana: " << resumen.mediana
              << "  Desviacion: " << resumen.desviacion_estandar
              << "\nPrecision promedio: " << resumen.precision_promedio << "%\n";
    imprimirHistogramaASCII(historial, alias);
}

ResumenOleada Estadisticas::calcularResumenOleada(BitacoraEventos* bitacora, JugadorPtr jugador, int oleada) {
    ResumenOleada resumen;
    if (bitacora == nullptr || jugador == nullptr) return resumen;
    for (NodoEventoPtr nodo = bitacora->primero(); nodo != nullptr; nodo = nodo->siguiente) {
        EventoPtr evento = nodo->dato;
        if (evento->jugador != jugador || evento->oleada != oleada) continue;
        resumen.tiene_eventos = true;
        if (evento->tipo_evento == TipoEvento::BAJA) {
            resumen.bajas++;
            resumen.puntos += evento->valor;
        }
        if (evento->tipo_evento == TipoEvento::DANO_NUCLEO || evento->tipo_evento == TipoEvento::CHOQUE_NAVE) resumen.dano += evento->valor;
    }
    return resumen;
}

void Estadisticas::imprimirLineaTiempo(BitacoraEventos* bitacora, JugadorPtr jugador) {
    if (bitacora == nullptr || jugador == nullptr) return;
    int ultimaOleada = 0;
    for (NodoEventoPtr nodo = bitacora->primero(); nodo != nullptr; nodo = nodo->siguiente) {
        if (nodo->dato->jugador == jugador && nodo->dato->oleada > ultimaOleada) ultimaOleada = nodo->dato->oleada;
    }
    std::cout << "Linea de tiempo de " << jugador->Alias << '\n';
    int acumulado = 0;
    for (int oleada = 1; oleada <= ultimaOleada; oleada++) {
        const ResumenOleada resumen = calcularResumenOleada(bitacora, jugador, oleada);
        acumulado += resumen.puntos;
        if (resumen.tiene_eventos) {
            std::cout << "Oleada " << oleada << ": bajas " << resumen.bajas
                      << ", dano " << resumen.dano << ", puntaje acumulado " << acumulado << '\n';
        }
    }
}
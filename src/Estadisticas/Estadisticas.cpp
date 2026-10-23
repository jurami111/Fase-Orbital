#include "Estadisticas.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace {
bool coincideAlias(const char* jugador, const std::string& alias) {
    return jugador != nullptr && alias == jugador;
}

std::vector<int> puntajesDe(Historial* historial, const std::string& alias) {
    std::vector<int> puntajes;
    if (historial == nullptr) return puntajes;

    NodoHistorialCSVPtr actual = historial->primero();
    while (actual != nullptr) {
        if (actual->dato != nullptr && coincideAlias(actual->dato->jugador, alias)) {
            puntajes.push_back(actual->dato->puntaje);
        }
        actual = actual->siguiente;
    }
    return puntajes;
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
    const std::vector<int> puntajes = puntajesDe(historial, alias);
    if (puntajes.empty()) return 0.0;

    double suma = 0.0;
    for (int puntaje : puntajes) suma += puntaje;
    return suma / puntajes.size();
}

double Estadisticas::calcularMediana(Historial* historial, const std::string& alias) {
    if (historial == nullptr) return 0.0;

    NodoHistorialCSVPtr copia = nullptr;
    NodoHistorialCSVPtr ultimo = nullptr;
    NodoHistorialCSVPtr actual = historial->primero();
    int cantidad = 0;

    while (actual != nullptr) {
        if (actual->dato != nullptr && coincideAlias(actual->dato->jugador, alias)) {
            NodoHistorialCSVPtr nuevo = new NodoHistorialCSV;
            nuevo->dato = actual->dato;
            nuevo->puntaje = actual->dato->puntaje;
            nuevo->siguiente = nullptr;
            if (copia == nullptr) {
                copia = nuevo;
            } else {
                ultimo->siguiente = nuevo;
            }
            ultimo = nuevo;
            cantidad++;
        }
        actual = actual->siguiente;
    }

    if (cantidad == 0) return 0.0;

    historial->MergeSort(copia);
    actual = copia;
    for (int i = 0; i < cantidad / 2; i++) actual = actual->siguiente;

    double mediana = actual->puntaje;
    if (cantidad % 2 == 0) {
        NodoHistorialCSVPtr anterior = copia;
        for (int i = 0; i < cantidad / 2 - 1; i++) anterior = anterior->siguiente;
        mediana = (static_cast<double>(anterior->puntaje) + actual->puntaje) / 2.0;
    }
    liberarNodos(copia);
    return mediana;
}

double Estadisticas::calcularDesviacionEstandar(Historial* historial, const std::string& alias) {
    const std::vector<int> puntajes = puntajesDe(historial, alias);
    if (puntajes.empty()) return 0.0;

    const double media = calcularMedia(historial, alias);
    double sumaCuadrados = 0.0;
    for (int puntaje : puntajes) {
        const double diferencia = puntaje - media;
        sumaCuadrados += diferencia * diferencia;
    }
    return std::sqrt(sumaCuadrados / puntajes.size());
}

double Estadisticas::calcularPrecisionPromedio(BitacoraEventos* bitacora, const std::string& alias) {
    if (bitacora == nullptr) return 0.0;

    int aciertos = 0;
    int disparos = 0;
    for (NodoEventoPtr nodo = bitacora->primero(); nodo != nullptr; nodo = nodo->siguiente) {
        if (nodo->dato == nullptr || nodo->dato->jugador == nullptr ||
            !coincideAlias(nodo->dato->jugador->Alias, alias)) continue;
        if (nodo->dato->tipo_evento == TipoEvento::BAJA) {
            aciertos++;
            disparos++;
        } else if (nodo->dato->tipo_evento == TipoEvento::IMPACTO_ERRONEO) {
            disparos++;
        }
    }
    return disparos == 0 ? 0.0 : 100.0 * aciertos / disparos;
}

int Estadisticas::obtenerColorMejorPrecision(BitacoraEventos* bitacora, const std::string& alias) {
    if (bitacora == nullptr) return -1;

    struct ConteoColor {
        int aciertos = 0;
        int disparos = 0;
    };
    std::map<int, ConteoColor> conteos;

    for (NodoEventoPtr nodo = bitacora->primero(); nodo != nullptr; nodo = nodo->siguiente) {
        if (nodo->dato == nullptr || nodo->dato->jugador == nullptr ||
            !coincideAlias(nodo->dato->jugador->Alias, alias)) continue;
        if (nodo->dato->tipo_evento != TipoEvento::BAJA &&
            nodo->dato->tipo_evento != TipoEvento::IMPACTO_ERRONEO) continue;

        ConteoColor& conteo = conteos[nodo->dato->color];
        conteo.disparos++;
        if (nodo->dato->tipo_evento == TipoEvento::BAJA) conteo.aciertos++;
    }

    int mejorColor = -1;
    double mejorPrecision = -1.0;
    for (const auto& entrada : conteos) {
        const double precision = static_cast<double>(entrada.second.aciertos) /
                                 entrada.second.disparos;
        if (precision > mejorPrecision) {
            mejorPrecision = precision;
            mejorColor = entrada.first;
        }
    }
    return mejorColor;
}

TipoEnemigo Estadisticas::obtenerEnemigoMasDanino(BitacoraEventos* bitacora, const std::string& alias) {
    if (bitacora == nullptr) return TipoEnemigo::Dron;

    long long danoPorEnemigo[6] = {};
    for (NodoEventoPtr nodo = bitacora->primero(); nodo != nullptr; nodo = nodo->siguiente) {
        if (nodo->dato == nullptr || nodo->dato->jugador == nullptr ||
            !coincideAlias(nodo->dato->jugador->Alias, alias) ||
            nodo->dato->tipo_evento != TipoEvento::DANO_NUCLEO) continue;

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

void Estadisticas::imprimirHistogramaASCII(Historial* historial, const std::string& alias) {
    const std::vector<int> puntajes = puntajesDe(historial, alias);
    if (puntajes.empty()) {
        std::cout << "No hay puntajes para " << alias << ".\n";
        return;
    }

    const auto extremos = std::minmax_element(puntajes.begin(), puntajes.end());
    const double minimo = *extremos.first;
    const double maximo = *extremos.second;
    const double ancho = (maximo - minimo) / 10.0;
    int bins[10] = {};

    for (int puntaje : puntajes) {
        int indice = ancho == 0.0 ? 0 : static_cast<int>((puntaje - minimo) / ancho);
        if (indice >= 10) indice = 9;
        bins[indice]++;
    }

    std::cout << "Histograma de puntajes para " << alias << ":\n";
    for (int i = 0; i < 10; i++) {
        const double inicio = minimo + i * ancho;
        const double fin = i == 9 ? maximo : minimo + (i + 1) * ancho;
        std::cout << '[' << std::fixed << std::setprecision(1) << inicio << ", "
                  << fin << (i == 9 ? "] " : ") ") << std::string(bins[i], '*')
                  << " (" << bins[i] << ")\n";
    }
}
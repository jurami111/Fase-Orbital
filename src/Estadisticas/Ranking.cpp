#include "Ranking.h"
#include "Estadisticas.h"
#include <cstring>
#include <iomanip>
#include <iostream>

namespace {
bool vaAntes(const RegistroRanking& primero, const RegistroRanking& segundo) {
    if (primero.puntaje != segundo.puntaje) return primero.puntaje > segundo.puntaje;
    if (primero.presicion != segundo.presicion) return primero.presicion > segundo.presicion;
    return std::strcmp(primero.Alias, segundo.Alias) < 0;
}
}

Ranking::Ranking(){
    primeroRanking = nullptr;
    cantidad = 0;
}

Ranking::~Ranking(){
    Limpiar();
}

void Ranking::Limpiar() {
    NodoRegistroPtr actual = primeroRanking;
    while (actual != nullptr) {
        NodoRegistroPtr siguiente = actual->siguiente;
        delete actual->dato;
        delete actual;
        actual = siguiente;
    }
    primeroRanking = nullptr;
    cantidad = 0;
}

NodoRegistroPtr Ranking::InsertarOrdenado(RegistroRankingPtr& registro) {
    if (registro == nullptr) return nullptr;
    NodoRegistroPtr nuevo = new NodoRegistro;
    try {
        nuevo->dato = new RegistroRanking(*registro);
    } catch (...) {
        delete nuevo;
        throw;
    }
    nuevo->siguiente = nullptr;
    NodoRegistroPtr* enlace = &primeroRanking;
    while (*enlace != nullptr && !vaAntes(*registro, *(*enlace)->dato)) {
        enlace = &(*enlace)->siguiente;
    }
    nuevo->siguiente = *enlace;
    *enlace = nuevo;
    cantidad++;
    return nuevo;
}

NodoRegistroPtr Ranking::Top(int n){
    if (n < 1) return nullptr;
    NodoRegistroPtr actual = primeroRanking;
    int contador = 1;
    while (actual != nullptr && contador < n) {
        actual = actual->siguiente;
        contador++;
    }
    return actual;
}

bool Ranking::isVacia() {
    return primeroRanking == nullptr;
}

NodoRegistroPtr Ranking::primero() {
    return primeroRanking;
}

NodoRegistroPtr Ranking::ultimo(){
    NodoRegistroPtr actual = primeroRanking;
    if (actual == nullptr) {
        return nullptr;
    }
    while (actual->siguiente != nullptr) {
        actual = actual->siguiente;
    }
    return actual;
}

NodoRegistroPtr Ranking::Eliminar(RegistroRankingPtr& registro) {
    NodoRegistroPtr* enlace = &primeroRanking;
    while (*enlace != nullptr && (*enlace)->dato != registro) enlace = &(*enlace)->siguiente;
    if (*enlace == nullptr) return nullptr;
    NodoRegistroPtr actual = *enlace;
    *enlace = actual->siguiente;
    delete actual->dato;
    delete actual;
    registro = nullptr;
    cantidad--;
    return primeroRanking;
}

void Ranking::ConstruirDesdeHistorial(Historial* historial) {
    Limpiar();
    if (historial == nullptr) return;
    Estadisticas estadisticas;
    for (NodoHistorialCSVPtr nodo = historial->primero(); nodo != nullptr; nodo = nodo->siguiente) {
        bool repetido = false;
        for (NodoHistorialCSVPtr anterior = historial->primero(); anterior != nodo; anterior = anterior->siguiente) {
            if (std::strcmp(anterior->dato->jugador, nodo->dato->jugador) == 0) {
                repetido = true;
                break;
            }
        }
        if (repetido) continue;
        const ResumenHistorico resumen = estadisticas.calcularResumenHistorico(historial, nodo->dato->jugador);
        RegistroRanking registro{};
        std::strcpy(registro.Alias, resumen.alias);
        registro.puntaje = resumen.mejor_puntaje;
        registro.presicion = resumen.precision_promedio;
        RegistroRankingPtr dato = &registro;
        InsertarOrdenado(dato);
    }
}

void Ranking::ImprimirTop(int limite) {
    if (isVacia()) {
        std::cout << "No hay jugadores en el ranking.\n";
        return;
    }
    std::cout << "Posicion  Jugador                   Mejor puntaje  Precision\n";
    NodoRegistroPtr nodo = primeroRanking;
    for (int posicion = 1; nodo != nullptr && posicion <= limite; posicion++, nodo = nodo->siguiente) {
        std::cout << std::setw(8) << posicion << "  " << std::left << std::setw(24) << nodo->dato->Alias
                  << std::right << std::setw(13) << nodo->dato->puntaje << "  "
                  << std::fixed << std::setprecision(2) << nodo->dato->presicion << "%\n";
    }
}
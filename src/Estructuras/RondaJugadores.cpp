#include "RondaJugadores.h"

RondaJugadores::RondaJugadores() {
    primero = nullptr;
    Actual = nullptr;
    ultimo = nullptr;
    cantidad = 0;
}

RondaJugadores::~RondaJugadores() {
    NodoJugadorPtr actual = primero;
    for (int i = 0; i < cantidad; i++) {
        NodoJugadorPtr siguiente = actual->siguiente;
        delete actual;
        actual = siguiente;
    }
    primero = nullptr;
    Actual = nullptr;
    ultimo = nullptr;
    cantidad = 0;
}

NodoJugadorPtr RondaJugadores::AgregarJugador(JugadorPtr& jugador) {
    NodoJugadorPtr nuevo = new NodoJugador;
    nuevo->dato = jugador;

    if (primero == nullptr) {
        nuevo->siguiente = nuevo;
        nuevo->anterior = nuevo;
        primero = nuevo;
        ultimo = nuevo;
        Actual = nuevo;
    } else {
        nuevo->siguiente = primero;
        nuevo->anterior = ultimo;
        ultimo->siguiente = nuevo;
        primero->anterior = nuevo;
        ultimo = nuevo;
    }
    cantidad++;

    return nuevo;
}

NodoJugadorPtr RondaJugadores::EliminarJugador(NodoJugadorPtr indice) {
    if (indice == nullptr) return nullptr;

    NodoJugadorPtr actual = primero;
    int visitados = 0;
    while (visitados < cantidad && actual != indice) {
        actual = actual->siguiente;
        visitados++;
    }
    if (visitados == cantidad) return nullptr;

    if (cantidad == 1) {
        primero = nullptr;
        ultimo = nullptr;
        Actual = nullptr;
    } else {
        indice->anterior->siguiente = indice->siguiente;
        indice->siguiente->anterior = indice->anterior;
        if (primero == indice) primero = indice->siguiente;
        if (ultimo == indice) ultimo = indice->anterior;
        if (Actual == indice) Actual = indice->siguiente;
    }

    delete indice;
    cantidad--;

    return Actual;
}

JugadorPtr RondaJugadores::EliminarActual(bool horario) {
    if (Actual == nullptr) return nullptr;

    JugadorPtr eliminado = Actual->dato;
    if (cantidad == 1) {
        EliminarJugador(Actual);
        return eliminado;
    }

    NodoJugadorPtr nuevoActual = horario ? Actual->siguiente : Actual->anterior;
    NodoJugadorPtr eliminadoNodo = Actual;
    Actual = nuevoActual;
    EliminarJugador(eliminadoNodo);
    Actual = nuevoActual;
    return eliminado;
}

NodoJugadorPtr RondaJugadores::AvanzarJugador() {
    return AvanzarJugador(true);
}

NodoJugadorPtr RondaJugadores::AvanzarJugador(bool horario) {
    if (Actual == nullptr) Actual = primero;
    else Actual = horario ? Actual->siguiente : Actual->anterior;
    return Actual;
}

NodoJugadorPtr RondaJugadores::VerActual(){
    return Actual;
}

int RondaJugadores::tamano(){
    return cantidad;
}
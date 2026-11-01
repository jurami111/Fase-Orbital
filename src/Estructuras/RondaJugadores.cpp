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

    NodoJugadorPtr nodo = Actual;
    JugadorPtr eliminado = nodo->dato;
    if (cantidad == 1) {
        primero = nullptr;
        ultimo = nullptr;
        Actual = nullptr;
    } else {
        nodo->anterior->siguiente = nodo->siguiente;
        nodo->siguiente->anterior = nodo->anterior;
        if (primero == nodo) primero = nodo->siguiente;
        if (ultimo == nodo) ultimo = nodo->anterior;
        Actual = horario ? nodo->siguiente : nodo->anterior;
    }
    delete nodo;
    cantidad--;
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
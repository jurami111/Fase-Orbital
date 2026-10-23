#include "BitacoraEventos.h"

BitacoraEventos::BitacoraEventos() {
    primerNodo = nullptr;
    ultimoNodo = nullptr;
    cantidad = 0;
}

BitacoraEventos::~BitacoraEventos(){
    NodoEventoPtr actual = primerNodo;
    while (actual != nullptr) {
        NodoEventoPtr siguiente = actual->siguiente;
        delete actual;
        actual = siguiente;
    }
    primerNodo = nullptr;
    ultimoNodo = nullptr;
    cantidad = 0;
}

NodoEventoPtr BitacoraEventos::AgregarFinal(EventoPtr& evento){
    NodoEventoPtr nuevoNodo = new NodoEvento;
    nuevoNodo->dato = evento;
    nuevoNodo->siguiente = nullptr;

    if (primerNodo == nullptr) {
        primerNodo = nuevoNodo;
        ultimoNodo = nuevoNodo;
    } else {
        ultimoNodo->siguiente = nuevoNodo;
        ultimoNodo = nuevoNodo;
    }
    cantidad++;
    return nuevoNodo;
}

NodoEventoPtr BitacoraEventos::Eliminar(NodoEventoPtr indice) {
    if (indice == nullptr) return nullptr;

    if (indice == primerNodo) {
        primerNodo = primerNodo->siguiente;
        if (indice == ultimoNodo) {
            ultimoNodo = nullptr;
        }
        delete indice;
        cantidad--;
        return primerNodo;
    }

    NodoEventoPtr actual = primerNodo;
    while (actual != nullptr && actual->siguiente != indice) {
        actual = actual->siguiente;
    }

    if (actual != nullptr) {
        actual->siguiente = indice->siguiente;
        if (indice == ultimoNodo) {
            ultimoNodo = actual;
        }
        delete indice;
        cantidad--;
        return actual->siguiente;
    }

    return nullptr;
}

NodoEventoPtr BitacoraEventos::BuscarPorJugadorYOleada(JugadorPtr jugador, int oleada) {
    NodoEventoPtr actual = primerNodo;
    while (actual != nullptr) {
        if (actual->dato->jugador == jugador && actual->dato->oleada == oleada) {
            return actual;
        }
        actual = actual->siguiente;
    }
    return nullptr;
}

NodoEventoPtr BitacoraEventos::primero() {
    return primerNodo;
}

NodoEventoPtr BitacoraEventos::ultimo() {
    return ultimoNodo;
}

int BitacoraEventos::tamano() {
    return cantidad;
}
#include <iostream>
#include "ListaEnemigos.h"

// Implementación del constructor
ListaEnemigos::ListaEnemigos() {
    primerNodo = nullptr;
    ultimoNodo = nullptr;
    cantidad = 0;
}

// Implementación del destructor
ListaEnemigos::~ListaEnemigos() {
    NodoEnemigo* actual = primerNodo;
    while (actual != nullptr) {
        NodoEnemigo* siguiente = actual->siguiente;
        delete actual->dato;
        delete actual;
        actual = siguiente;
    }
}

// Implementación de insertarFinal
NodoEnemigo* ListaEnemigos::InsertarFinal(EnemigoPtr& enemigo) {
    if (enemigo == nullptr) return nullptr;
    NodoEnemigo* nuevo = new NodoEnemigo; // Reservas memoria
    try {
        nuevo->dato = new Enemigo(*enemigo);
    } catch (...) {
        delete nuevo;
        throw;
    }
    nuevo->siguiente = nullptr;
    nuevo->anterior = ultimoNodo;
    
    if (vacia()) {
        primerNodo = nuevo;
    } else {
        ultimoNodo->siguiente = nuevo;
    }
    ultimoNodo = nuevo;
    cantidad++;
    
    return nuevo;
}

// Implementación de insertarDespues
NodoEnemigoPtr ListaEnemigos::InsertarDespues(NodoEnemigoPtr indice, const EnemigoPtr& enemigo) {
    if (indice == nullptr || enemigo == nullptr) return nullptr;

    NodoEnemigoPtr nuevo = new NodoEnemigo;
    try {
        nuevo->dato = new Enemigo(*enemigo);
    } catch (...) {
        delete nuevo;
        throw;
    }
    nuevo->siguiente = indice->siguiente;
    nuevo->anterior = indice;

    if (indice->siguiente != nullptr) {
        indice->siguiente->anterior = nuevo;
    } else {
        ultimoNodo = nuevo;
    }
    indice->siguiente = nuevo;
    cantidad++;

    return nuevo;
}

NodoEnemigoPtr ListaEnemigos::Eliminar(NodoEnemigoPtr indice) {
    if (indice == nullptr) return nullptr;

    if (indice->anterior != nullptr) {
        indice->anterior->siguiente = indice->siguiente;
    } else {
        primerNodo = indice->siguiente;
    }

    if (indice->siguiente != nullptr) {
        indice->siguiente->anterior = indice->anterior;
    } else {
        ultimoNodo = indice->anterior;
    }

    NodoEnemigoPtr siguiente = indice->siguiente;
    delete indice->dato;
    delete indice;
    cantidad--;

    return siguiente;
}

NodoEnemigoPtr ListaEnemigos::Buscar(int id){
    NodoEnemigoPtr actual = primerNodo;
    while (actual != nullptr) {
        if (actual->dato->id == id) {
            return actual;
        }
        actual = actual->siguiente;
    }
    return nullptr;
}

NodoEnemigoPtr ListaEnemigos::primero() {
    return primerNodo;
}

NodoEnemigoPtr ListaEnemigos::ultimo() {
    return ultimoNodo;
}

int ListaEnemigos::tamano() {
    return cantidad;
}

bool ListaEnemigos::vacia() {
    return cantidad == 0;
}

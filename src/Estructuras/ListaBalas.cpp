#include "ListaBalas.h"

// Implementación del constructor
ListaBalas::ListaBalas() {
    primerNodo = nullptr;
    ultimoNodo = nullptr;
    cantidad = 0;
}

// Implementación del destructor
ListaBalas::~ListaBalas() {
    NodoBalaPtr actual = primerNodo;
    while (actual != nullptr) {
        NodoBalaPtr siguiente = actual->siguiente;
        delete actual;
        actual = siguiente;
    }
    primerNodo = nullptr;
    ultimoNodo = nullptr;
    cantidad = 0;
}

NodoBalaPtr ListaBalas::InsertarInicio(BalaPtr& bala){
    NodoBalaPtr nuevo = new NodoBala;
    nuevo->dato = bala;
    nuevo->siguiente = primerNodo;

    if (primerNodo == nullptr) ultimoNodo = nuevo;
    primerNodo = nuevo;
    cantidad++;

    return nuevo;
}

NodoBalaPtr ListaBalas::InsertarFinal(BalaPtr& bala){
    NodoBalaPtr nuevo = new NodoBala;
    nuevo->dato = bala;
    nuevo->siguiente = nullptr;

    if (ultimoNodo != nullptr) {
        ultimoNodo->siguiente = nuevo;
    } else {
        primerNodo = nuevo;
    }
    ultimoNodo = nuevo;
    cantidad++;

    return nuevo;
};

NodoBalaPtr ListaBalas::Eliminar(NodoBalaPtr& bala) {
    if (bala == nullptr) return nullptr;

    NodoBalaPtr anterior = nullptr;
    NodoBalaPtr* enlace = &primerNodo;
    while (*enlace != nullptr && *enlace != bala) {
        anterior = *enlace;
        enlace = &(*enlace)->siguiente;
    }
    if (*enlace == nullptr) return nullptr;

    NodoBalaPtr siguiente = bala->siguiente;
    *enlace = siguiente;
    if (bala == ultimoNodo) ultimoNodo = anterior;
    delete bala;
    bala = siguiente;
    cantidad--;
    return siguiente;
}

NodoBalaPtr ListaBalas::Buscar(int id){
    NodoBalaPtr actual = primerNodo;
    while (actual != nullptr) {
        if (actual->dato != nullptr && actual->dato->duenio == id) {
            return actual;
        }
        actual = actual->siguiente;
    }
    return nullptr;
}

NodoBalaPtr ListaBalas::primero(){
    return primerNodo;
}

NodoBalaPtr ListaBalas::ultimo(){
    return ultimoNodo;
}

int ListaBalas::tamano(){
    return cantidad;
}

bool ListaBalas::vacia(){
    return cantidad == 0;
}
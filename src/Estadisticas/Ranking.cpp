#include "Ranking.h"

Ranking::Ranking(){
    primeroRanking = nullptr;
    cantidad = 0;
}

Ranking::~Ranking(){
    NodoRegistroPtr actual = primeroRanking;
    while (actual != nullptr) {
        NodoRegistroPtr siguiente = actual->siguiente;
        delete actual;
        actual = siguiente;
    }
}

NodoRegistroPtr Ranking::InsertarOrdenado(RegistroRankingPtr& registro) {
    NodoRegistroPtr nuevo = new NodoRegistro;
    nuevo->dato = registro;
    nuevo->siguiente = nullptr;
    nuevo->anterior = nullptr;

    if (primeroRanking == nullptr) {
        primeroRanking = nuevo;
    } else {
        NodoRegistroPtr actual = primeroRanking;
        while (actual != nullptr && actual->dato->puntaje >= registro->puntaje) {
            actual = actual->siguiente;
        }
        if (actual == nullptr) {
            NodoRegistroPtr ultimo = primeroRanking;
            while (ultimo->siguiente != nullptr) {
                ultimo = ultimo->siguiente;
            }
            ultimo->siguiente = nuevo;
            nuevo->anterior = ultimo;
        } else {
            nuevo->siguiente = actual;
            nuevo->anterior = actual->anterior;
            if (actual->anterior != nullptr) {
                actual->anterior->siguiente = nuevo;
            } else {
                primeroRanking = nuevo;
            }
            actual->anterior = nuevo;
        }
    }
    cantidad++;
    return nuevo;
}

NodoRegistroPtr Ranking::Top(int n){
    NodoRegistroPtr actual = primeroRanking;
    int contador = 0;
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
    NodoRegistroPtr actual = primeroRanking;
    while (actual != nullptr && actual->dato != registro) {
        actual = actual->siguiente;
    }
    if (actual == nullptr) {
        return nullptr;
    }
    if (actual->anterior != nullptr) {
        actual->anterior->siguiente = actual->siguiente;
    } else {
        primeroRanking = actual->siguiente;
    }
    if (actual->siguiente != nullptr) {
        actual->siguiente->anterior = actual->anterior;
    }
    delete actual;
    cantidad--;
    return primeroRanking;
}
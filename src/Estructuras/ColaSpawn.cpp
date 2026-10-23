#include "ColaSpawn.h"
#include <memory>

ColaSpawn::ColaSpawn() {
    primerNodo = nullptr;
    ultimoNodo = nullptr;
    cantidad = 0;
}

ColaSpawn::~ColaSpawn() {
    Limpiar();
}

ColaSpawn::ColaSpawn(const ColaSpawn& otra) : primerNodo(nullptr), ultimoNodo(nullptr), cantidad(0) {
    try {
        for (NodoSpawnPtr actual = otra.primerNodo; actual != nullptr; actual = actual->siguiente) {
            EncolarCopia(*actual->dato);
        }
    } catch (...) {
        Limpiar();
        throw;
    }
}

ColaSpawn& ColaSpawn::operator=(const ColaSpawn& otra) {
    if (this != &otra) {
        ColaSpawn copia(otra);
        Intercambiar(copia);
    }
    return *this;
}

void ColaSpawn::Limpiar() {
    NodoSpawnPtr actual = primerNodo;
    while (actual != nullptr) {
        NodoSpawnPtr siguiente = actual->siguiente;
        delete actual->dato;
        delete actual;
        actual = siguiente;
    }
    primerNodo = nullptr;
    ultimoNodo = nullptr;
    cantidad = 0;
}

void ColaSpawn::Intercambiar(ColaSpawn& otra) {
    NodoSpawnPtr nodo = primerNodo;
    primerNodo = otra.primerNodo;
    otra.primerNodo = nodo;

    nodo = ultimoNodo;
    ultimoNodo = otra.ultimoNodo;
    otra.ultimoNodo = nodo;

    int total = cantidad;
    cantidad = otra.cantidad;
    otra.cantidad = total;
}

NodoSpawnPtr ColaSpawn::EncolarCopia(const DatosSpawn& spawn) {
    std::unique_ptr<DatosSpawn> dato(new DatosSpawn(spawn));
    std::unique_ptr<NodoSpawn> nuevo(new NodoSpawn);
    nuevo->dato = dato.get();
    nuevo->siguiente = nullptr;
    nuevo->anterior = ultimoNodo;

    if (ultimoNodo != nullptr) {
        ultimoNodo->siguiente = nuevo.get();
    } else {
        primerNodo = nuevo.get();
    }
    ultimoNodo = nuevo.get();
    cantidad++;

    dato.release();
    return nuevo.release();
}

NodoSpawnPtr ColaSpawn::Encolar(SpawnPtr& spawn) {
    if (spawn == nullptr) return nullptr;
    return EncolarCopia(*spawn);
}

NodoSpawnPtr ColaSpawn::Desencolar() {
    if (primerNodo == nullptr) return nullptr;

    NodoSpawnPtr frente = primerNodo;
    NodoSpawnPtr siguiente = frente->siguiente;
    primerNodo = siguiente;
    if (primerNodo == nullptr) {
        ultimoNodo = nullptr;
    } else {
        primerNodo->anterior = nullptr;
    }
    delete frente->dato;
    delete frente;
    cantidad--;

    return primerNodo;
}

bool ColaSpawn::Desencolar(DatosSpawn& salida) {
    if (primerNodo == nullptr) return false;
    salida = *primerNodo->dato;
    Desencolar();
    return true;
}

NodoSpawnPtr ColaSpawn::primero() {
    return primerNodo;
}

const DatosSpawn* ColaSpawn::VerFrente() const {
    return primerNodo == nullptr ? nullptr : primerNodo->dato;
}

bool ColaSpawn::isVacia() const {
    return primerNodo == nullptr;
}

int ColaSpawn::tamano() const {
    return cantidad;
}

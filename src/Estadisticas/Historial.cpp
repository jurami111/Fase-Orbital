#include "Historial.h"
#include <cmath>
#include <cstring>
#include <limits>
Historial::Historial() {
    primerNodo = nullptr;
    ultimoNodo = nullptr;
    cantidad = 0;
}

Historial::~Historial() {
    Limpiar();
}

void Historial::Limpiar() {
    NodoHistorialCSVPtr actual = primerNodo;
    while (actual != nullptr) {
        NodoHistorialCSVPtr siguiente = actual->siguiente;
        delete actual->dato;
        delete actual;
        actual = siguiente;
    }
    primerNodo = nullptr;
    ultimoNodo = nullptr;
    cantidad = 0;
}

NodoHistorialCSVPtr Historial::InsertarFinal(HistorialCSVPtr& registro){
    if (registro == nullptr) return nullptr;
    NodoHistorialCSVPtr nuevo = new NodoHistorialCSV;
    nuevo->dato = registro;
    nuevo->puntaje = registro->puntaje;
    nuevo->siguiente = nullptr;

    if (primerNodo == nullptr) {
        primerNodo = nuevo;
    } else {
        ultimoNodo->siguiente = nuevo;
    }
    ultimoNodo = nuevo;
    cantidad++;
    return nuevo;
}

NodoHistorialCSVPtr Historial::CopiarAListaAuxiliar(NodoHistorialCSVPtr& lista){
    NodoHistorialCSVPtr copia = nullptr;
    NodoHistorialCSVPtr actual = lista;
    NodoHistorialCSVPtr ultimo = nullptr;
    while (actual != nullptr) {
        NodoHistorialCSVPtr nuevo = new NodoHistorialCSV;
        nuevo->dato = actual->dato;
        nuevo->puntaje = actual->puntaje;
        nuevo->siguiente = nullptr;
        if (copia == nullptr) {
            copia = nuevo;
        } else {
            ultimo->siguiente = nuevo;
        }
        ultimo = nuevo;
        actual = actual->siguiente;
    }
    return copia;
}

NodoHistorialCSVPtr Historial::mezclar(NodoHistorialCSVPtr a, NodoHistorialCSVPtr b) {
    if (a == nullptr) return b;
    if (b == nullptr) return a;

    NodoHistorialCSVPtr resultado = nullptr;
    
    if (a->puntaje <= b->puntaje) {
        resultado = a;
        resultado->siguiente = mezclar(a->siguiente, b);
    } else {
        resultado = b;
        resultado->siguiente = mezclar(a, b->siguiente);
    }
    return resultado;
}

void Historial::dividir(NodoHistorialCSVPtr fuente, NodoHistorialCSVPtr* frente, NodoHistorialCSVPtr* atras) {
    NodoHistorialCSVPtr lento = fuente;
    NodoHistorialCSVPtr rapido = fuente->siguiente;

    // El rápido avanza dos nodos por cada nodo que avanza el lento
    while (rapido != nullptr) {
        rapido = rapido->siguiente;
        if (rapido != nullptr) {
            lento = lento->siguiente;
            rapido = rapido->siguiente;
        }
    }
    *frente = fuente;
    *atras = lento->siguiente;
    lento->siguiente = nullptr; // Desconecta la primera mitad de la segunda
}

void Historial::MergeSort(NodoHistorialCSVPtr& lista){
NodoHistorialCSVPtr primero = lista;
    NodoHistorialCSVPtr a;
    NodoHistorialCSVPtr b;

    // Caso base: lista vacía o de un solo nodo
    if (primero == nullptr || primero->siguiente == nullptr) {
        return;
    }

    dividir(primero, &a, &b);
    
    MergeSort(a);
    MergeSort(b);

    lista = mezclar(a, b);
}

int Historial::calcularMedia(NodoHistorialCSVPtr& lista) {
    if (lista == nullptr) return 0;
    int suma = 0;
    int contador = 0;
    NodoHistorialCSVPtr actual = lista;
    while (actual != nullptr) {
        suma += actual->dato->puntaje;
        contador++;
        actual = actual->siguiente;
    }
    return (contador == 0) ? 0 : suma / contador;
}

int Historial::calcularDesviacionEstandar(NodoHistorialCSVPtr& lista) {
    if (lista == nullptr) return 0;
    int media = calcularMedia(lista);
    int sumaCuadrados = 0;
    int contador = 0;
    NodoHistorialCSVPtr actual = lista;
    while (actual != nullptr) {
        int diff = actual->dato->puntaje - media;
        sumaCuadrados += diff * diff;
        contador++;
        actual = actual->siguiente;
    }
    return (contador == 0) ? 0 : static_cast<int>(sqrt(static_cast<double>(sumaCuadrados) / contador));
}

bool Historial::isVacia() {
    return cantidad == 0;
}

NodoHistorialCSVPtr Historial::Top(){
    return primerNodo;
}

int Historial::tamano() {
    return cantidad;
}

NodoHistorialCSVPtr Historial::primero() {
    return primerNodo;
}

NodoHistorialCSVPtr Historial::ultimo() {
    return ultimoNodo;
}

bool Historial::Contiene(HistorialCSVPtr& registro) {
    if (registro == nullptr) return false;
    NodoHistorialCSVPtr actual = primerNodo;
    while (actual != nullptr) {
        if (std::strcmp(actual->dato->jugador, registro->jugador) == 0) {
            return true;
        }
        actual = actual->siguiente;
    }
    return false;
}

NodoHistorialCSVPtr Historial::SiguientePartida(HistorialCSVPtr& registro){
    if (registro == nullptr) return nullptr;
    NodoHistorialCSVPtr actual = primerNodo;
    while (actual != nullptr) {
        if (actual->dato == registro) {
            return actual->siguiente;
        }
        actual = actual->siguiente;
    }
    return nullptr;
}

bool Historial::ExisteJugador(const std::string& alias) {
    for (NodoHistorialCSVPtr nodo = primerNodo; nodo != nullptr; nodo = nodo->siguiente) {
        if (alias == nodo->dato->jugador) return true;
    }
    return false;
}

int Historial::SiguienteIdPartida() {
    int mayor = 0;
    for (NodoHistorialCSVPtr nodo = primerNodo; nodo != nullptr; nodo = nodo->siguiente) {
        if (nodo->dato->id_partida > mayor) mayor = nodo->dato->id_partida;
    }
    return mayor == std::numeric_limits<int>::max() ? 0 : mayor + 1;
}
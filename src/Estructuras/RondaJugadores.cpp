#include "RondaJugadores.h"

// constructor: se ejecuta cuando se crea una ronda para dejar la lista vacía
RondaJugadores::RondaJugadores() {
    primero = nullptr;
    Actual = nullptr;
    ultimo = nullptr;
    cantidad = 0;
}

// destructor: libera la memoria usada para crear los nodos, esto se necesita porque los nodos se crean
// usando new
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
    NodoJugadorPtr nuevo = new NodoJugador; // agrega nuevo jugador a la lista
    nuevo->dato = jugador; // le asigna el jugador al nodo

    // si la lista esta vacia, el nuevo nodo se convierte en el primero, ultimo y actual.
    // Sus enlaces siguiente y anterior apuntan a él mismo
    if (primero == nullptr) {
        nuevo->siguiente = nuevo;
        nuevo->anterior = nuevo;
        primero = nuevo;
        ultimo = nuevo;
        Actual = nuevo;
    } 

    //Si ya existen jugadore entonces conecta el nuevo despues del último y antes del primero
    else {  
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
    if (indice == nullptr) return nullptr;  //verifica si se recibe unpuntero que sí existe, sino termina la función

    NodoJugadorPtr actual = primero; // creamos un ptr actual y lo colocamos en primer nodo de la lista
    int visitados = 0; // contador nodos visitados

    // Recorre la lista hasta encontrar el nodo
    while (visitados < cantidad && actual != indice) {
        actual = actual->siguiente;
        visitados++;
    }

    if (visitados == cantidad) return nullptr; // Revisa si se encontro 

    if (cantidad == 1) { // revisamos si solo queda 1 jugador y si lo eliminamos la lista queda vacia
        primero = nullptr;
        ultimo = nullptr;
        Actual = nullptr;
    } else {
        indice->anterior->siguiente = indice->siguiente; // pone como siguiente el siguiente del que se eliminó
        indice->siguiente->anterior = indice->anterior; // pone como anterior el anterior del que se eliminó
        if (primero == indice) primero = indice->siguiente; // Si eliminamos al primer jugador, el siguiente pasa a ser el primero
        if (ultimo == indice) ultimo = indice->anterior; //Si eliminamos al último jugador, el anterior pasa a ser el último
        if (Actual == indice) Actual = indice->siguiente; // Si eliminamos al jugador que tenía el turno, el turno pasa al siguiente
    }

    delete indice;
    cantidad--; // resta 1 a la cantidad tot de jugadores

    return Actual; // retorna el nodo que quedo como Actual
}

// elimina al jugador que tiene el turno en ese momento y decide quién sigue jugando
JugadorPtr RondaJugadores::EliminarActual(bool horario) {
    if (Actual == nullptr) return nullptr; // revisa si hay un jugador actual

    NodoJugadorPtr nodo = Actual;
    JugadorPtr eliminado = nodo->dato; // eliminado apunta a los datos del jugador, porque después vamos a borrar el nodo, pero queremos poder devolver los datos de ese jugador
    
    // revisa si es el único jugador
    if (cantidad == 1) {
        primero = nullptr;
        ultimo = nullptr;
        Actual = nullptr;
    } else { // si hay varios reconecta la lista
        nodo->anterior->siguiente = nodo->siguiente;
        nodo->siguiente->anterior = nodo->anterior;

        if (primero == nodo) primero = nodo->siguiente; // Si eliminamos al primero, el siguiente ocupa su lugar
        if (ultimo == nodo) ultimo = nodo->anterior; // Si eliminamos al último, el anterior pasa a ser el último
        if (horario) {  // se decide quien va a tener el siguiente turno
            Actual = nodo->siguiente;
        } else {
            Actual = nodo->anterior;
        } 
    }
    delete nodo;
    cantidad--;
    return eliminado; // retorna los datos del jugador que se eliminó
}

// pasa al siguiente jugador sin tener que indicar la direccion
NodoJugadorPtr RondaJugadores::AvanzarJugador() {
    return AvanzarJugador(true); // llama a la otra versión de AvanzarJugador
}

// permite cambiar el turno hacia adelante o hacia atrás
NodoJugadorPtr RondaJugadores::AvanzarJugador(bool horario) {
    if (Actual == nullptr) { // revisa si existe un jugador actual
        Actual = primero; // Si Actual es nullptr, hacemos que apunte al primer jugador de la lista
    } 
    // si hay un jugador actual revisa la dirección si es horario (true)->siguiente nodo, horario(false)->nodo anterior
    else {
        if (horario) {
            Actual = Actual->siguiente;
        } else {
            Actual = Actual->anterior;
        }
    }
    return Actual; // retorna al jugador que le toca jugar
}

// consulta quién tiene el turno
NodoJugadorPtr RondaJugadores::VerActual(){
    return Actual;
}

// retorna la cantidad de jugadores en la ronda
int RondaJugadores::tamano(){
    return cantidad;
}

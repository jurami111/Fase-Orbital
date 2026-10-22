#pragma once

typedef struct Jugador {
    char Alias[25];
    bool isBot;
    int puntaje, combo, carga_pulso, vida_nucleo, numero_oleada;
} *JugadorPtr;

struct NodoJugador;
using NodoJugadorPtr = NodoJugador*;

struct NodoJugador {
    JugadorPtr dato;
    NodoJugadorPtr siguiente;
    NodoJugadorPtr anterior;
};

class RondaJugadores {
    private:
        NodoJugadorPtr primero;
        NodoJugadorPtr Actual;
        NodoJugadorPtr ultimo;
        int cantidad;

    public:
        RondaJugadores(); //Constructor
        ~RondaJugadores();//Destructor

        // Prohibir copia (Regla de tres)
        RondaJugadores(const RondaJugadores&) = delete;
        RondaJugadores& operator=(const RondaJugadores&) = delete;

        NodoJugadorPtr AgregarJugador(JugadorPtr& jugador); //Le pasamos un puntero a jugador y lo insertamos al final de la lista
        NodoJugadorPtr EliminarJugador(NodoJugadorPtr indice); //Eliminamos un nodo
        NodoJugadorPtr AvanzarJugador(); //Avanzamos al siguiente jugador en la ronda
        NodoJugadorPtr VerActual();
        int tamano(); //Cantidad de jugadores en la ronda


};
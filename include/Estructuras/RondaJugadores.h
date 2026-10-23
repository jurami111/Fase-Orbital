#pragma once

typedef struct Jugador {
    char Alias[25];
    bool isBot;
    int puntaje, combo, carga_pulso, vida_nucleo, numero_oleada;
    int posicion, disparos, correctos, erroneos, combo_max, pulsos;
    int dano_total, ticks;
    int bajas_dron, bajas_girador, bajas_divisor;
    int bajas_mini, bajas_tanque, bajas_camaleon;

    Jugador()
        : Alias{}, isBot(false), puntaje(0), combo(0), carga_pulso(0),
          vida_nucleo(0), numero_oleada(0), posicion(0), disparos(0),
          correctos(0), erroneos(0), combo_max(0), pulsos(0), dano_total(0),
          ticks(0), bajas_dron(0), bajas_girador(0), bajas_divisor(0),
          bajas_mini(0), bajas_tanque(0), bajas_camaleon(0) {}
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
        JugadorPtr EliminarActual(bool horario);
        NodoJugadorPtr AvanzarJugador(); //Avanzamos al siguiente jugador en la ronda
        NodoJugadorPtr AvanzarJugador(bool horario);
        NodoJugadorPtr VerActual();
        int tamano(); //Cantidad de jugadores en la ronda


};
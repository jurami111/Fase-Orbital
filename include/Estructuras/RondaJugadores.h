#pragma once

typedef struct Jugador {
    char Alias[25]; //guarda el nombre del jugador 
    bool isBot; // si es un bot (true) o una persona(false)
    int puntaje, combo, carga_pulso, vida_nucleo, numero_oleada;
    int posicion, disparos, correctos, erroneos, combo_max, pulsos;
    int dano_total, ticks;
    int bajas_dron, bajas_girador, bajas_divisor;
    int bajas_mini, bajas_tanque, bajas_camaleon;
    int correctos_color[3]{};
    int erroneos_color[3]{};
    int bajas_color[3]{};
    int dano_por_tipo[6]{};

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
    JugadorPtr dato; //apunta a la informacion del jugador
    NodoJugadorPtr siguiente;  //apunta al siguiente nodo(jugador)
    NodoJugadorPtr anterior;  // apunta al anterior(jugador)
}; 

// controlar la lista de jugadores y los turnos
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
        JugadorPtr EliminarActual(bool horario); // elimina jugador actual
        NodoJugadorPtr AvanzarJugador(); //Avanzamos al siguiente jugador en la ronda
        NodoJugadorPtr AvanzarJugador(bool horario); //avanza en la direccion de los turnos
        NodoJugadorPtr VerActual(); // devuelve el nodo del jugador que le toca el turno
        int tamano(); //Cantidad de jugadores en la ronda

};
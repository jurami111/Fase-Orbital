#pragma once

#include "../Estructuras/ListaEnemigos.h"
#include "../Estructuras/RondaJugadores.h"

enum class TipoEvento {
    BAJA,
    IMPACTO_ERRONEO,
    DANO_NUCLEO,
    CHOQUE_NAVE,
    PULSO,
    FIN_OLEADA,
    ELIMINACION
};

typedef struct Evento {
    int tick, oleada, color, valor;
    JugadorPtr jugador;
    TipoEvento tipo_evento;
    TipoEnemigo tipo_enemigo;
} *EventoPtr;

struct NodoEvento;
using NodoEventoPtr = NodoEvento*;

struct NodoEvento {
    EventoPtr dato;
    NodoEventoPtr siguiente;
};

class BitacoraEventos {
    private:
        NodoEventoPtr primerNodo;
        NodoEventoPtr ultimoNodo;
        int cantidad;

    public:
        BitacoraEventos(); //Constructor
        ~BitacoraEventos();//Destructor

        // Prohibir copia (Regla de tres)
        BitacoraEventos(const BitacoraEventos&) = delete;
        BitacoraEventos& operator=(const BitacoraEventos&) = delete;
        
        NodoEventoPtr AgregarFinal(EventoPtr& evento); //Le pasamos un puntero a evento y lo insertamos al final de la lista
        NodoEventoPtr Eliminar(NodoEventoPtr indice); //Eliminamos un nodo de la lista
        NodoEventoPtr BuscarPorJugadorYOleada(JugadorPtr jugador, int oleada); //Buscamos un evento por su jugador y oleada
        NodoEventoPtr primero(); //Primer evento de la lista
        NodoEventoPtr ultimo(); //Ultimo evento de la lista
        int tamano(); //Cantidad de eventos en la lista
};
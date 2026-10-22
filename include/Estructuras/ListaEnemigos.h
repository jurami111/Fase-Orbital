#pragma once

enum class TipoEnemigo {
    Dron,
    Girador,
    Divisor,
    Mini,
    Tanque,
    Camaleon
};

typedef struct Enemigo {
int id;
TipoEnemigo tipo;
double r, theta, omega;
int color, vida, radio, ticksColor;
}*EnemigoPtr;

struct NodoEnemigo;
using NodoEnemigoPtr = NodoEnemigo*;

struct NodoEnemigo {
    EnemigoPtr dato;
    NodoEnemigoPtr siguiente;
    NodoEnemigoPtr anterior;
};

class  ListaEnemigos {
    private:
        NodoEnemigoPtr primerNodo;
        NodoEnemigoPtr ultimoNodo;
        int cantidad;

    public:
        ListaEnemigos(); //Constructor
        ~ListaEnemigos();//Destructor

        // Prohibir copia (Regla de tres)
        ListaEnemigos(const ListaEnemigos&) = delete;
        ListaEnemigos& operator=(const ListaEnemigos&) = delete;

        NodoEnemigoPtr InsertarFinal(EnemigoPtr& enemigo); //Le pasamos un puntero a enemigo y lo insertamos al final de la lista
        NodoEnemigoPtr insertarDespues(NodoEnemigoPtr indice, const EnemigoPtr& enemigo); //Insertamos un enemigo despues de un nodo dado
        NodoEnemigoPtr Eliminar(NodoEnemigoPtr indice); //Eliminamos un nodo de la lista
        NodoEnemigoPtr Buscar(int id); //Buscamos un enemigo por su id
        NodoEnemigoPtr primero(); //Primer enemigo de la lista
        NodoEnemigoPtr ultimo(); //Ultimo enemigo de la lista
        int tamano(); //Cantidad de enemigos en la lista
        bool vacia(); //Verificamos si la lista esta vacia
};
#pragma once

typedef struct Bala {
    double x, y, vx, vy;
    int color, duenio;
} *BalaPtr;

struct NodoBala;
using NodoBalaPtr = NodoBala*;

struct NodoBala {
    BalaPtr dato;
    NodoBalaPtr siguiente;
};

class ListaBalas {
    private:
        NodoBalaPtr primerNodo;
        NodoBalaPtr ultimoNodo;
        int cantidad;
    
    public:
        ListaBalas(); //Constructor
        ~ListaBalas();//Destructor

        // Prohibir copia (Regla de tres)
        ListaBalas(const ListaBalas&) = delete;
        ListaBalas& operator=(const ListaBalas&) = delete;

        NodoBalaPtr InsertarInicio(BalaPtr& bala); //Le pasamos un puntero a bala y lo insertamos al inicio de la lista
        NodoBalaPtr InsertarFinal(BalaPtr& bala); //Le pasamos un puntero a bala y lo insertamos al final de la lista 
        NodoBalaPtr Eliminar(NodoBalaPtr& bala); //Eliminamos un nodo y devolvemos el siguiente
        template <class Predicado>
        int EliminarSi(Predicado debeEliminarse) {
            int eliminadas = 0;
            NodoBalaPtr anterior = nullptr;
            NodoBalaPtr* enlace = &primerNodo;

            while (*enlace != nullptr) {
                NodoBalaPtr actual = *enlace;
                if (debeEliminarse(actual->dato)) {
                    *enlace = actual->siguiente;
                    if (actual == ultimoNodo) ultimoNodo = anterior;
                    delete actual;
                    cantidad--;
                    eliminadas++;
                } else {
                    anterior = actual;
                    enlace = &actual->siguiente;
                }
            }
            return eliminadas;
        }
        NodoBalaPtr Buscar(int id); //Buscamos una bala por su id
        NodoBalaPtr primero(); //Primer bala de la lista
        NodoBalaPtr ultimo(); //Ultima bala de la lista
        int tamano(); //Cantidad de balas en la lista
        bool vacia(); //Verificamos si la lista esta vacia
};
#pragma once

#include "ListaEnemigos.h"

typedef struct DatosSpawn{
    int tick, color;
    TipoEnemigo tipo;
    double angulo;
    bool sentido; //True = horario, False = antihorario
} *SpawnPtr;

struct NodoSpawn;
using NodoSpawnPtr = NodoSpawn*;

struct NodoSpawn {
    SpawnPtr dato;
    NodoSpawnPtr siguiente;
};

class Spawn{
    private:
        NodoSpawnPtr primerNodo;
        NodoSpawnPtr ultimoNodo;
        int cantidad;

    public:
        Spawn(); //Constructor
        ~Spawn();//Destructor

        // Prohibir copia (Regla de tres)
        Spawn(const Spawn&) = delete;
        Spawn& operator=(const Spawn&) = delete;

        NodoSpawnPtr Encolar(SpawnPtr& spawn); //Le pasamos un puntero a spawn y lo insertamos al final de la lista
        NodoSpawnPtr Desencolar(); //Eliminamos el primer spawn de la lista
        NodoSpawnPtr primero(); //Primer spawn de la lista
        bool isVacia(); //Verificamos si la lista esta vacia

};
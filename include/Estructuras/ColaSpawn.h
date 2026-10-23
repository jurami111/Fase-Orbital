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
    NodoSpawnPtr anterior;
};

class ColaSpawn {
    private:
        NodoSpawnPtr primerNodo;
        NodoSpawnPtr ultimoNodo;
        int cantidad;

    public:
        ColaSpawn(); //Constructor
        ~ColaSpawn();//Destructor

        // Prohibir copia (Regla de tres)
        ColaSpawn(const ColaSpawn& otra);
        ColaSpawn& operator=(const ColaSpawn& otra);

        NodoSpawnPtr Encolar(SpawnPtr& spawn); //Inserta una copia del dato al final
        NodoSpawnPtr Desencolar(); //Elimina el frente y devuelve el nuevo frente
        bool Desencolar(DatosSpawn& salida); //Retira el frente y copia su dato en salida
        NodoSpawnPtr primero(); //Primer spawn de la lista
        const DatosSpawn* VerFrente() const;
        bool isVacia() const; //Verificamos si la lista esta vacia
        int tamano() const;

    private:
        NodoSpawnPtr EncolarCopia(const DatosSpawn& spawn);
        void Limpiar();
        void Intercambiar(ColaSpawn& otra);
};

using Spawn = ColaSpawn;
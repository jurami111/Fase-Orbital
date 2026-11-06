#pragma once

#include <string>

typedef struct HistorialCSV{
    int id_partida, semilla;
    char modo[25];
    char jugador[25];
    int es_bot, posicion, puntaje, oleada, disparos, correctos, erroneos;
    int combo_max, pulsos, dano_total, ticks;
    int bajas_dron, bajas_girador, bajas_divisor;
    int bajas_mini, bajas_tanque, bajas_camaleon;
    int correctos_color[3]{};
    int erroneos_color[3]{};
    int bajas_color[3]{};
    int dano_por_tipo[6]{};
}*HistorialCSVPtr;

struct NodoHistorialCSV;
using NodoHistorialCSVPtr = NodoHistorialCSV*;

struct NodoHistorialCSV {
    HistorialCSVPtr dato;
    int puntaje;
    NodoHistorialCSVPtr siguiente;
};

class Historial {
    private:
        NodoHistorialCSVPtr primerNodo;
        NodoHistorialCSVPtr ultimoNodo;
        int cantidad;

    public:
        Historial(); //Constructor
        ~Historial();//Destructor

        // Prohibir copia (Regla de tres)
        Historial(const Historial&) = delete;
        Historial& operator=(const Historial&) = delete;

        NodoHistorialCSVPtr InsertarFinal(HistorialCSVPtr& registro); //Le pasamos un puntero a registro y lo insertamos al final de la lista
        NodoHistorialCSVPtr CopiarAListaAuxiliar(NodoHistorialCSVPtr& lista); //Copiamos los registros de la lista principal a una lista auxiliar
        void dividir(NodoHistorialCSVPtr fuente, NodoHistorialCSVPtr* frente, NodoHistorialCSVPtr* atras); //Dividimos la lista auxiliar en dos mitades
        NodoHistorialCSVPtr mezclar(NodoHistorialCSVPtr a, NodoHistorialCSVPtr b); //Mezclamos dos listas auxiliares ordenadas
        void MergeSort(NodoHistorialCSVPtr& lista); //Ordenamos la lista auxiliar de manera descendente por puntaje
        int calcularMedia(NodoHistorialCSVPtr& lista); //Calculamos la media de puntajes de la lista auxiliar
        int calcularDesviacionEstandar(NodoHistorialCSVPtr& lista); //Calculamos la desviación estándar de puntajes de la lista auxiliar
        NodoHistorialCSVPtr Top(); //Primer registro de la lista
        bool isVacia(); //Verificamos si la lista esta vacia    
        int tamano();
        NodoHistorialCSVPtr primero();
        NodoHistorialCSVPtr ultimo();
        void Limpiar();
        bool ExisteJugador(const std::string& alias);
        int SiguienteIdPartida();
        bool Contiene(HistorialCSVPtr& registro); //Verificamos si la lista contiene un registro específico
        NodoHistorialCSVPtr SiguientePartida(HistorialCSVPtr& registro); //Obtenemos el siguiente registro de la lista

};
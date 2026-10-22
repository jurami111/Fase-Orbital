typedef struct HistorialCSV{
    int id_partida, semilla, puntaje; 
    char jugador[25];
}*HistorialCSVPtr;

struct NodoHistorialCSV;
using NodoHistorialCSVPtr = NodoHistorialCSV*;

typedef struct NodoHistorialCSV {
    HistorialCSVPtr dato;
    NodoHistorialCSVPtr siguiente;
};

class Historial {
    private:
        NodoHistorialCSVPtr primero;
        int cantidad;

    public:
        Historial(); //Constructor
        ~Historial();//Destructor

        // Prohibir copia (Regla de tres)
        Historial(const Historial&) = delete;
        Historial& operator=(const Historial&) = delete;

        NodoHistorialCSVPtr InsertarFinal(HistorialCSVPtr& registro); //Le pasamos un puntero a registro y lo insertamos al final de la lista
        NodoHistorialCSVPtr CopiarAListaAuxiliar(NodoHistorialCSVPtr& lista); //Copiamos los registros de la lista principal a una lista auxiliar
        NodoHistorialCSVPtr MergeSort(NodoHistorialCSVPtr& lista); //Ordenamos la lista auxiliar de manera descendente por puntaje
        int calcularMedia(NodoHistorialCSVPtr& lista); //Calculamos la media de puntajes de la lista auxiliar
        int calcularMediana(NodoHistorialCSVPtr& lista); //Calculamos la mediana de puntajes de la lista auxiliar
        int calcularDesviacionEstandar(NodoHistorialCSVPtr& lista); //Calculamos la desviación estándar de puntajes de la lista auxiliar
        NodoHistorialCSVPtr Top(); //Primer registro de la lista
};
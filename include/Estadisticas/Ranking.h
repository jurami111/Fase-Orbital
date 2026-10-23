typedef struct RegistroRanking {
    char Alias[25];
    int puntaje;
    double presicion;
}*RegistroRankingPtr;

struct NodoRegistro;
using NodoRegistroPtr = NodoRegistro*;

struct NodoRegistro {
    RegistroRankingPtr dato;
    NodoRegistroPtr siguiente;
    NodoRegistroPtr anterior;
};

class Ranking {
    private:
        NodoRegistroPtr primeroRanking;
        int cantidad;

    public:
        Ranking(); //Constructor
        ~Ranking();//Destructor

        // Prohibir copia (Regla de tres)
        Ranking(const Ranking&) = delete;
        Ranking& operator=(const Ranking&) = delete;

        NodoRegistroPtr InsertarOrdenado(RegistroRankingPtr& registro); //Le pasamos un puntero a registro y lo insertamos en la lista de manera ordenada
        NodoRegistroPtr Top(int n); //Top n registros de la lista
        bool isVacia(); //Verificamos si la lista esta vacia
        NodoRegistroPtr primero(); //Primer registro de la lista
        NodoRegistroPtr ultimo(); //Ultimo registro de la lista
        NodoRegistroPtr Eliminar(RegistroRankingPtr& registro); //Eliminar un registro de la lista por su puntero
        
};
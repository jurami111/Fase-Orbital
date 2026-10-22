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
};

class Ranking {
    private:
        NodoRegistroPtr primero;
        int cantidad;

    public:
        Ranking(); //Constructor
        ~Ranking();//Destructor

        // Prohibir copia (Regla de tres)
        Ranking(const Ranking&) = delete;
        Ranking& operator=(const Ranking&) = delete;

        NodoRegistroPtr InsertarOrdenado(RegistroRankingPtr& registro); //Le pasamos un puntero a registro y lo insertamos en la lista de manera ordenada
        NodoRegistroPtr Top(int n); //Top n registros de la lista
};
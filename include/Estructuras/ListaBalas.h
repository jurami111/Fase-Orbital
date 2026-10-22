typedef struct Bala {
    double x, y, vx, vy;
    int color, duenio;
} *BalaPtr;


typedef struct NodoEnemigo {
    BalaPtr dato;
    BalaPtr siguiente;
    BalaPtr anterior;
}*NodoBalaPtr;

class  ListaBalas {
    private:
        NodoBalaPtr primero;
        int cantidad;
    
    public:
        ListaBalas(); //Constructor
        ~ListaBalas();//Destructor

        NodoBalaPtr InsertarInicio(BalaPtr& bala); //Le pasamos un puntero a bala y lo insertamos al inicio de la lista, 
        NodoBalaPtr EliminarSi(BalaPtr& bala); //Eliminamos una bala de la lista si existe
};
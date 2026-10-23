#include "../include/Estructuras/ListaEnemigos.h"
#include "../include/Estructuras/ListaBalas.h"
#include "../include/Estructuras/ColaSpawn.h"
#include "../include/Estructuras/RondaJugadores.h"
#include "../include/Motor/Generador.h"
#include "../include/Motor/Oleada.h"

#include <cstdint>
#include <iostream>

namespace {
bool verificar(bool condicion, const char* mensaje) {
    if (!condicion) std::cerr << "  Falló: " << mensaje << '\n';
    return condicion;
}

bool pruebaP01() {
    Enemigo enemigos[1000]{};
    ListaEnemigos lista;
    for (int i = 0; i < 1000; i++) {
        enemigos[i].id = i;
        EnemigoPtr dato = &enemigos[i];
        lista.InsertarFinal(dato);
    }

    for (NodoEnemigoPtr nodo = lista.primero(); nodo != nullptr;) {
        nodo = nodo->dato->id % 2 == 0 ? lista.Eliminar(nodo) : nodo->siguiente;
    }

    bool valido = verificar(lista.tamano() == 500, "quedan 500 enemigos");
    int esperado = 1;
    for (NodoEnemigoPtr nodo = lista.primero(); nodo != nullptr; nodo = nodo->siguiente) {
        valido &= verificar(nodo->dato->id == esperado, "recorrido hacia adelante en orden");
        esperado += 2;
    }

    esperado = 999;
    for (NodoEnemigoPtr nodo = lista.ultimo(); nodo != nullptr; nodo = nodo->anterior) {
        valido &= verificar(nodo->dato->id == esperado, "recorrido hacia atrás en orden");
        esperado -= 2;
    }
    return valido && esperado == -1;
}

bool pruebaP02() {
    Enemigo datos[3]{};
    ListaEnemigos lista;
    for (int i = 0; i < 3; i++) {
        datos[i].id = i;
        EnemigoPtr dato = &datos[i];
        lista.InsertarFinal(dato);
    }

    lista.Eliminar(lista.primero());
    bool valido = verificar(lista.primero()->dato->id == 1 &&
                            lista.primero()->anterior == nullptr, "eliminación de cabeza");
    lista.Eliminar(lista.ultimo());
    valido &= verificar(lista.primero() == lista.ultimo() &&
                        lista.tamano() == 1, "eliminación de cola");
    lista.Eliminar(lista.primero());
    valido &= verificar(lista.vacia() && lista.primero() == nullptr &&
                        lista.ultimo() == nullptr, "eliminación del nodo único");
    return valido;
}

bool pruebaP03() {
    Enemigo datos[3]{};
    ListaEnemigos lista;
    for (int i = 0; i < 2; i++) {
        datos[i].id = i;
        EnemigoPtr dato = &datos[i];
        lista.InsertarFinal(dato);
    }
    datos[2].id = 2;
    EnemigoPtr nuevoDato = &datos[2];
    NodoEnemigoPtr nuevo = lista.InsertarDespues(lista.ultimo(), nuevoDato);
    return verificar(nuevo != nullptr && lista.ultimo() == nuevo &&
                     nuevo->dato->id == 2 && nuevo->anterior->dato->id == 1 &&
                     nuevo->siguiente == nullptr, "la cola se actualiza al insertar después del último");
}

bool pruebaP04() {
    Bala datos[6]{};
    ListaBalas lista;
    for (int i = 0; i < 6; i++) {
        datos[i].color = (i < 2 || (i >= 3 && i <= 4)) ? 1 : 0;
        BalaPtr dato = &datos[i];
        lista.InsertarFinal(dato);
    }

    const int eliminadas = lista.EliminarSi([](BalaPtr bala) {
        return bala->color == 1;
    });
    bool valido = verificar(eliminadas == 4 && lista.tamano() == 2,
                            "se eliminan todas las balas consecutivas marcadas");
    NodoBalaPtr actual = lista.primero();
    valido &= verificar(actual != nullptr && actual->dato == &datos[2],
                        "se conserva la bala válida después de la cabeza");
    actual = actual == nullptr ? nullptr : actual->siguiente;
    valido &= verificar(actual != nullptr && actual->dato == &datos[5] &&
                        actual->siguiente == nullptr && lista.ultimo() == actual,
                        "se conserva la bala válida final y la cola es correcta");
    return valido;
}

bool pruebaP05() {
    ColaSpawn cola;
    DatosSpawn datos[4]{};
    for (int i = 0; i < 4; i++) {
        datos[i].tick = i + 1;
        SpawnPtr dato = &datos[i];
        cola.Encolar(dato);
    }

    bool valido = true;
    DatosSpawn salida{};
    for (int i = 1; i <= 4; i++) {
        valido &= verificar(cola.Desencolar(salida) && salida.tick == i,
                            "los spawns salen en orden FIFO");
    }
    valido &= verificar(cola.isVacia() && cola.primero() == nullptr &&
                        cola.VerFrente() == nullptr && !cola.Desencolar(salida),
                        "desencolar una cola vacía devuelve false");
    return valido;
}

bool pruebaP06() {
    ColaSpawn original;
    DatosSpawn datos[3]{};
    for (int i = 0; i < 3; i++) {
        datos[i].tick = 10 + i;
        datos[i].color = i;
        SpawnPtr dato = &datos[i];
        original.Encolar(dato);
    }

    ColaSpawn copia(original);
    ColaSpawn asignada;
    asignada = original;
    DatosSpawn salida{};
    bool valido = verificar(copia.Desencolar(salida) && salida.tick == 10 &&
                            copia.Desencolar(salida) && salida.tick == 11,
                            "se consume la copia independiente");
    valido &= verificar(original.tamano() == 3 && original.VerFrente() != nullptr &&
                        original.VerFrente()->tick == 10,
                        "la cola original permanece intacta");
    valido &= verificar(asignada.tamano() == 3 && asignada.VerFrente() != nullptr &&
                        asignada.VerFrente()->tick == 10,
                        "la asignación también conserva una copia independiente");
    return valido;
}

bool pruebaP07() {
    Jugador jugadores[3]{};
    RondaJugadores ronda;
    for (Jugador& jugador : jugadores) {
        JugadorPtr dato = &jugador;
        ronda.AgregarJugador(dato);
    }

    bool valido = verificar(ronda.VerActual() != nullptr &&
                            ronda.VerActual()->dato == &jugadores[0],
                            "el primer jugador queda como actual");
    ronda.AvanzarJugador(true);
    valido &= verificar(ronda.VerActual()->dato == &jugadores[1],
                        "rotación horaria");
    ronda.AvanzarJugador(false);
    valido &= verificar(ronda.VerActual()->dato == &jugadores[0],
                        "rotación antihoraria");

    JugadorPtr eliminado = ronda.EliminarActual(false);
    valido &= verificar(eliminado == &jugadores[0] && ronda.tamano() == 2 &&
                        ronda.VerActual()->dato == &jugadores[2],
                        "eliminar actual con avance antihorario");
    NodoJugadorPtr actual = ronda.VerActual();
    valido &= verificar(actual->siguiente->anterior == actual &&
                        actual->anterior->siguiente == actual,
                        "enlaces circulares tras eliminación");

    ronda.AvanzarJugador(false);
    eliminado = ronda.EliminarActual(true);
    valido &= verificar(eliminado == &jugadores[1] && ronda.tamano() == 1 &&
                        ronda.VerActual()->dato == &jugadores[2],
                        "eliminar actual con avance horario");
    eliminado = ronda.EliminarActual(true);
    valido &= verificar(eliminado == &jugadores[2] && ronda.tamano() == 0 &&
                        ronda.VerActual() == nullptr,
                        "eliminar el jugador único");
    return valido;
}

bool pruebaGeneradorYOleada() {
    Generador generador(1);
    bool valido = verificar(generador.siguiente() == 1103527590u &&
                            generador.siguiente() == 377401575u &&
                            generador.siguiente() == 662824084u,
                            "secuencia LCG estable para la semilla 1");

    Oleada generadorOleada;
    ColaSpawn primera = generadorOleada.Generar(24, 1);
    ColaSpawn segunda = generadorOleada.Generar(24, 1);
    valido &= verificar(primera.tamano() == 9 && segunda.tamano() == 9,
                        "cantidad de apariciones para oleada 1");

    DatosSpawn a{};
    DatosSpawn b{};
    int apariciones = 0;
    while (primera.Desencolar(a)) {
        valido &= verificar(segunda.Desencolar(b) && a.tick == b.tick &&
                            a.color == b.color && a.tipo == b.tipo &&
                            a.angulo == b.angulo && a.sentido == b.sentido,
                            "misma semilla produce una oleada idéntica");
        if (apariciones == 0) {
            valido &= verificar(a.tick == 60, "la primera aparición ocurre en el tick 60");
        }
        apariciones++;
    }
    valido &= verificar(apariciones == 9 && segunda.isVacia(),
                        "ambas colas se consumen en paralelo");
    return valido;
}

void reportar(const char* nombre, bool resultado, int& fallidas) {
    std::cout << nombre << (resultado ? " PASS\n" : " FAIL\n");
    if (!resultado) fallidas++;
}
}

int main() {
    int fallidas = 0;
    reportar("P01", pruebaP01(), fallidas);
    reportar("P02", pruebaP02(), fallidas);
    reportar("P03", pruebaP03(), fallidas);
    reportar("P04", pruebaP04(), fallidas);
    reportar("P05", pruebaP05(), fallidas);
    reportar("P06", pruebaP06(), fallidas);
    reportar("P07", pruebaP07(), fallidas);
    reportar("G01", pruebaGeneradorYOleada(), fallidas);
    std::cout << (fallidas == 0 ? "Todas las pruebas pasaron.\n" : "Hay pruebas fallidas.\n");
    return fallidas == 0 ? 0 : 1;
}

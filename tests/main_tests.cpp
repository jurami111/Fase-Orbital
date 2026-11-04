#include "../include/Estructuras/ListaEnemigos.h"
#include "../include/Estructuras/ListaBalas.h"
#include "../include/Estructuras/ColaSpawn.h"
#include "../include/Estructuras/RondaJugadores.h"
#include "../include/Motor/Generador.h"
#include "../include/Motor/Oleada.h"
#include "../include/Estadisticas/Estadisticas.h"
#include "../include/Estadisticas/Persistencia.h"
#include "../include/Estadisticas/Ranking.h"
#include "../include/Vista/Controlador.h"

#include <array>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

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
        datos[i].duenio = i;
        BalaPtr dato = &datos[i];
        lista.InsertarFinal(dato);
    }

    const int eliminadas = lista.EliminarSi([](BalaPtr bala) {
        return bala->color == 1;
    });
    bool valido = verificar(eliminadas == 4 && lista.tamano() == 2,
                            "se eliminan todas las balas consecutivas marcadas");
    NodoBalaPtr actual = lista.primero();
    valido &= verificar(actual != nullptr && actual->dato->duenio == 2 &&
                        actual->dato != &datos[2],
                        "se conserva la bala válida después de la cabeza");
    actual = actual == nullptr ? nullptr : actual->siguiente;
    valido &= verificar(actual != nullptr && actual->dato->duenio == 5 &&
                        actual->dato != &datos[5] &&
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

void agregarPuntaje(Historial& historial, int puntaje) {
    HistorialCSVPtr registro = new HistorialCSV{};
    std::strcpy(registro->jugador, "Ana");
    registro->puntaje = puntaje;
    historial.InsertarFinal(registro);
}

bool pruebaEstadisticas() {
    Estadisticas estadisticas;
    Historial vacio;
    bool valido = verificar(estadisticas.calcularMediana(&vacio, "Ana") == 0.0 &&
                            estadisticas.calcularMedia(&vacio, "Ana") == 0.0 &&
                            estadisticas.calcularDesviacionEstandar(&vacio, "Ana") == 0.0,
                            "estadisticas de lista vacia");
    Historial historial;
    agregarPuntaje(historial, 40);
    valido &= verificar(estadisticas.calcularMediana(&historial, "Ana") == 40.0,
                        "mediana de un registro");
    agregarPuntaje(historial, 10);
    valido &= verificar(estadisticas.calcularMediana(&historial, "Ana") == 25.0,
                        "mediana de dos registros inversos");
    agregarPuntaje(historial, 30);
    valido &= verificar(estadisticas.calcularMediana(&historial, "Ana") == 30.0,
                        "mediana impar");
    agregarPuntaje(historial, 30);
    valido &= verificar(estadisticas.calcularMediana(&historial, "Ana") == 30.0 &&
                        estadisticas.calcularMedia(&historial, "Ana") == 27.5 &&
                        std::fabs(estadisticas.calcularDesviacionEstandar(&historial, "Ana") -
                                  std::sqrt(118.75)) < 0.000001,
                        "mediana par con repetidos y desviacion poblacional");
    valido &= verificar(historial.primero()->dato->puntaje == 40 && historial.tamano() == 4,
                        "la mediana no modifica el historial");
    std::ostringstream salida;
    std::streambuf* anterior = std::cout.rdbuf(salida.rdbuf());
    estadisticas.imprimirHistogramaASCII(&historial, "Ana");
    std::cout.rdbuf(anterior);
    valido &= verificar(salida.str().find("[37.0, 40.0] * (1)") != std::string::npos,
                        "el maximo pertenece al ultimo intervalo");
    Historial iguales;
    agregarPuntaje(iguales, 0);
    agregarPuntaje(iguales, 0);
    salida.str("");
    anterior = std::cout.rdbuf(salida.rdbuf());
    estadisticas.imprimirHistogramaASCII(&iguales, "Ana");
    std::cout.rdbuf(anterior);
    valido &= verificar(salida.str().find("** (2)") != std::string::npos,
                        "histograma de puntajes iguales sin division por cero");
    return valido;
}

bool pruebaPrecisionYDano() {
    Jugador jugador;
    std::strcpy(jugador.Alias, "Ana");
    jugador.correctos = 3;
    jugador.erroneos = 1;
    jugador.correctos_color[0] = 1;
    jugador.erroneos_color[0] = 1;
    jugador.correctos_color[2] = 2;
    Evento evento{};
    evento.jugador = &jugador;
    evento.tipo_evento = TipoEvento::CHOQUE_NAVE;
    evento.tipo_enemigo = TipoEnemigo::Tanque;
    evento.valor = 12;
    EventoPtr dato = &evento;
    BitacoraEventos bitacora;
    bitacora.AgregarFinal(dato);
    Estadisticas estadisticas;
    return verificar(estadisticas.calcularPrecisionPromedio(&bitacora, "Ana") == 75.0 &&
                     estadisticas.obtenerColorMejorPrecision(&bitacora, "Ana") == 2 &&
                     estadisticas.obtenerEnemigoMasDanino(&bitacora, "Ana") == TipoEnemigo::Tanque,
                     "precision usa impactos reales y dano incluye choque con nave");
}

bool pruebaPropiedadDeDatos() {
    ListaEnemigos enemigos;
    EnemigoPtr original = new Enemigo{};
    original->id = 7;
    NodoEnemigoPtr enemigo = enemigos.InsertarFinal(original);
    bool valido = verificar(enemigo->dato != original, "enemigo almacenado como copia");
    delete original;
    NodoEnemigoPtr segundoEnemigo = enemigos.InsertarDespues(enemigo, enemigo->dato);
    segundoEnemigo->dato->id = 8;
    valido &= verificar(enemigo->dato->id == 7 && segundoEnemigo->dato->id == 8,
                        "copias de enemigos independientes del original y entre si");
    valido &= verificar(enemigos.Eliminar(enemigo) == segundoEnemigo,
                        "eliminar enemigo retorna el siguiente");

    Bala bala{};
    bala.duenio = 3;
    BalaPtr datoBala = &bala;
    ListaBalas balas;
    NodoBalaPtr primeraBala = balas.InsertarInicio(datoBala);
    bala.duenio = 4;
    NodoBalaPtr segundaBala = balas.InsertarFinal(datoBala);
    bala.duenio = 99;
    valido &= verificar(primeraBala->dato->duenio == 3 && segundaBala->dato->duenio == 4,
                        "las balas no cambian al modificar el dato local");
    valido &= verificar(balas.Eliminar(primeraBala) == segundaBala &&
                        primeraBala == segundaBala && bala.duenio == 99,
                        "eliminar bala conserva el original y actualiza el puntero");

    Jugador jugador;
    Evento evento{};
    evento.jugador = &jugador;
    evento.valor = 12;
    EventoPtr datoEvento = &evento;
    BitacoraEventos bitacora;
    NodoEventoPtr primerEvento = bitacora.AgregarFinal(datoEvento);
    evento.valor = 22;
    NodoEventoPtr segundoEvento = bitacora.AgregarFinal(datoEvento);
    evento.valor = 32;
    NodoEventoPtr tercerEvento = bitacora.AgregarFinal(datoEvento);
    evento.valor = 99;
    valido &= verificar(primerEvento->dato->valor == 12 && segundoEvento->dato->valor == 22 &&
                        tercerEvento->dato->valor == 32 && primerEvento->dato->jugador == &jugador,
                        "eventos copiados pero jugador compartido sin transferir propiedad");
    bitacora.Eliminar(tercerEvento);
    bitacora.Eliminar(primerEvento);
    valido &= verificar(bitacora.primero() == segundoEvento &&
                        bitacora.ultimo() == segundoEvento && bitacora.tamano() == 1,
                        "eliminar cabeza y cola de eventos libera solo sus copias");

    RegistroRanking registro{};
    registro.puntaje = 10;
    RegistroRankingPtr datoRegistro = &registro;
    Ranking ranking;
    NodoRegistroPtr primerRegistro = ranking.InsertarOrdenado(datoRegistro);
    registro.puntaje = 20;
    NodoRegistroPtr segundoRegistro = ranking.InsertarOrdenado(datoRegistro);
    registro.puntaje = 99;
    valido &= verificar(primerRegistro->dato->puntaje == 10 && segundoRegistro->dato->puntaje == 20 &&
                        ranking.primero() == segundoRegistro,
                        "ranking almacena copias independientes");
    RegistroRankingPtr copiaRegistro = primerRegistro->dato;
    ranking.Eliminar(copiaRegistro);
    valido &= verificar(ranking.primero() == segundoRegistro &&
                        ranking.ultimo() == segundoRegistro && registro.puntaje == 99,
                        "eliminar ranking con puntero a la copia conserva el original");
    return valido;
}

bool pruebaHistorial() {
    Historial historial;
    bool valido = verificar(historial.ultimo() == nullptr && historial.SiguienteIdPartida() == 1,
                            "historial vacio inicializado");
    agregarPuntaje(historial, 10);
    agregarPuntaje(historial, 20);
    historial.ultimo()->dato->id_partida = 7;
    HistorialCSV consulta{};
    std::strcpy(consulta.jugador, "Ana");
    HistorialCSVPtr dato = &consulta;
    valido &= verificar(historial.ultimo()->dato->puntaje == 20 && historial.Contiene(dato) &&
                        historial.ExisteJugador("Ana") && !historial.ExisteJugador("Luis") &&
                        historial.SiguienteIdPartida() == 8,
                        "cola, comparacion de alias y siguiente identificador");
    HistorialCSVPtr primero = historial.primero()->dato;
    valido &= verificar(historial.SiguientePartida(primero) == historial.ultimo(),
                        "siguiente registro por identidad");
    historial.Limpiar();
    valido &= verificar(historial.primero() == nullptr && historial.ultimo() == nullptr &&
                        historial.tamano() == 0, "limpieza reinicia cabeza, cola y cantidad");
    agregarPuntaje(historial, 30);
    return valido && verificar(historial.primero() == historial.ultimo(), "historial reutilizable");
}

bool pruebaReportesYRanking() {
    Historial historial;
    agregarPuntaje(historial, 100);
    historial.ultimo()->dato->posicion = 1;
    historial.ultimo()->dato->correctos = 3;
    historial.ultimo()->dato->erroneos = 1;
    historial.ultimo()->dato->correctos_color[0] = 2;
    historial.ultimo()->dato->erroneos_color[0] = 1;
    historial.ultimo()->dato->bajas_color[0] = 1;
    historial.ultimo()->dato->dano_por_tipo[4] = 12;
    agregarPuntaje(historial, 300);
    historial.ultimo()->dato->posicion = 2;
    historial.ultimo()->dato->correctos = 1;
    historial.ultimo()->dato->erroneos = 3;
    agregarPuntaje(historial, 300);
    std::strcpy(historial.ultimo()->dato->jugador, "Luis");
    historial.ultimo()->dato->correctos = 3;
    historial.ultimo()->dato->erroneos = 1;
    agregarPuntaje(historial, 300);
    std::strcpy(historial.ultimo()->dato->jugador, "Zoe");
    historial.ultimo()->dato->correctos = 3;
    historial.ultimo()->dato->erroneos = 1;
    Estadisticas estadisticas;
    const ResumenHistorico resumen = estadisticas.calcularResumenHistorico(&historial, "Ana");
    bool valido = verificar(resumen.partidas_jugadas == 2 && resumen.cantidad_victorias == 1 &&
                            resumen.mejor_puntaje == 300 && resumen.peor_puntaje == 100 &&
                            resumen.media == 200.0 && resumen.mediana == 200.0 &&
                            resumen.desviacion_estandar == 100.0 && resumen.precision_promedio == 50.0,
                            "resumen historico y promedio por partida");
    valido &= verificar(resumen.correctos_color[0] == 2 && resumen.erroneos_color[0] == 1 &&
                        resumen.bajas_color[0] == 1 && resumen.dano_por_tipo[4] == 12 &&
                        std::fabs(estadisticas.calcularPrecisionColorHistorica(&historial, "Ana", 0) -
                                  200.0 / 3.0) < 0.000001 &&
                        estadisticas.obtenerEnemigoMasDanino(&historial, "Ana") == TipoEnemigo::Tanque,
                        "resumen agrega precision por color, bajas y dano por tipo");
    Ranking ranking;
    ranking.ConstruirDesdeHistorial(&historial);
    valido &= verificar(std::strcmp(ranking.Top(1)->dato->Alias, "Luis") == 0 &&
                        std::strcmp(ranking.Top(2)->dato->Alias, "Zoe") == 0 &&
                        std::strcmp(ranking.Top(3)->dato->Alias, "Ana") == 0 &&
                        ranking.Top(4) == nullptr, "alias unicos y desempates de precision y nombre");
    ranking.ConstruirDesdeHistorial(&historial);
    valido &= verificar(ranking.Top(4) == nullptr, "reconstruir no duplica ranking");
    Jugador jugador;
    Evento baja{};
    baja.jugador = &jugador;
    baja.oleada = 1;
    baja.tipo_evento = TipoEvento::BAJA;
    baja.valor = 100;
    EventoPtr dato = &baja;
    BitacoraEventos bitacora;
    bitacora.AgregarFinal(dato);
    baja.tipo_evento = TipoEvento::CHOQUE_NAVE;
    baja.valor = 12;
    bitacora.AgregarFinal(dato);
    const ResumenOleada oleada = estadisticas.calcularResumenOleada(&bitacora, &jugador, 1);
    return valido && verificar(oleada.bajas == 1 && oleada.puntos == 100 && oleada.dano == 12,
                              "resumen de oleada desde bitacora");
}

bool pruebaGuardarParticipantes(const std::string& ruta) {
    Jugador ana, luis;
    std::strcpy(ana.Alias, "Ana");
    std::strcpy(luis.Alias, "Luis");
    ana.posicion = 1;
    ana.puntaje = 300;
    luis.posicion = 2;
    luis.puntaje = 100;
    JugadorPtr jugadores[] = {&ana, &luis};
    Historial historial;
    Persistencia persistencia;
    bool valido = persistencia.guardarPartidaCompleta(ruta, jugadores, 2, 42, "turnos", &historial);
    valido &= verificar(historial.tamano() == 2 && historial.primero()->dato->id_partida == 1 &&
                        historial.ultimo()->dato->id_partida == 1,
                        "todos los participantes se guardan con id comun");
    Historial recargado;
    valido &= persistencia.cargarHistorial(ruta, &recargado);
    valido &= verificar(recargado.tamano() == 2, "archivo coincide con historial en memoria");
    luis.posicion = 1;
    valido &= verificar(!persistencia.guardarPartidaCompleta(ruta, jugadores, 2, 43, "turnos", &historial) &&
                        historial.tamano() == 2, "datos invalidos no modifican historial");
    return valido;
}

bool pruebaPersistencia(const std::string& ruta) {
    Persistencia persistencia;
    Jugador jugador;
    std::strcpy(jugador.Alias, "Ana,\"Luz\"");
    jugador.posicion = 1;
    jugador.puntaje = 10;
    jugador.correctos_color[0] = 2;
    jugador.erroneos_color[0] = 1;
    jugador.bajas_color[2] = 1;
    jugador.dano_por_tipo[4] = 12;
    bool valido = persistencia.guardarPartida(ruta, &jugador, 1, 42, "turnos");
    jugador.puntaje = 20;
    valido &= persistencia.guardarPartida(ruta, &jugador, 2, 42, "turnos");
    Historial historial;
    valido &= persistencia.cargarHistorial(ruta, &historial);
    Estadisticas estadisticas;
    valido &= verificar(historial.tamano() == 2 &&
                        estadisticas.calcularMediana(&historial, jugador.Alias) == 15.0 &&
                        historial.primero()->dato->correctos_color[0] == 2 &&
                        historial.primero()->dato->erroneos_color[0] == 1 &&
                        historial.primero()->dato->bajas_color[2] == 1 &&
                        historial.primero()->dato->dano_por_tipo[4] == 12,
                        "CSV conserva alias y contadores nuevos");
    std::ifstream lectura(ruta);
    std::string encabezado, primera, segunda;
    std::getline(lectura, encabezado);
    std::getline(lectura, primera);
    std::getline(lectura, segunda);
    lectura.close();
    std::ofstream archivo(ruta);
    archivo << encabezado << "\r\n" << primera << "\r\n" << segunda << "\r\n";
    archivo << primera << ",extra\n";
    archivo << primera.substr(0, primera.rfind(',')) << '\n';
    archivo << "x" << primera.substr(primera.find(',')) << '\n';
    archivo << "999999999999999999999" << primera.substr(primera.find(',')) << '\n';
    archivo << "1,42,\"turnos\"x,Ana,0,0,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0\n";
    archivo << "1,42,tu\"rnos,Ana,0,0,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0\n";
    archivo.close();
    Historial recargado;
    std::ostringstream errores;
    std::streambuf* anterior = std::cerr.rdbuf(errores.rdbuf());
    valido &= persistencia.cargarHistorial(ruta, &recargado);
    std::cerr.rdbuf(anterior);
    valido &= verificar(recargado.tamano() == 2 &&
                        errores.str().find("4") != std::string::npos &&
                        errores.str().find("7") != std::string::npos &&
                        errores.str().find("8") != std::string::npos &&
                        errores.str().find("9") != std::string::npos,
                        "CRLF valido y filas con campos, numeros o comillas invalidos omitidos");
    archivo.open(ruta);
    archivo << "encabezado incorrecto\n";
    archivo.close();
    anterior = std::cerr.rdbuf(errores.rdbuf());
    valido &= verificar(!persistencia.cargarHistorial(ruta, &recargado) &&
                        !persistencia.guardarPartida(ruta, &jugador, 3, 42, "turnos"),
                        "se rechaza encabezado incorrecto al cargar y guardar");
    std::cerr.rdbuf(anterior);
    return valido;
}

bool pruebaMigracionCSVAntiguo(const std::string& ruta) {
    const std::string encabezadoAntiguo =
        "id_partida,semilla,modo,jugador,es_bot,posicion,puntaje,oleada,disparos,"
        "correctos,erroneos,combo_max,pulsos,dano_total,ticks,bajas_dron,"
        "bajas_girador,bajas_divisor,bajas_mini,bajas_tanque,bajas_camaleon";
    {
        std::ofstream archivo(ruta);
        archivo << encabezadoAntiguo << '\n'
                << "1,42,turnos,Ana,0,1,100,2,4,3,1,1,0,10,30,1,0,0,0,0,0\n";
    }
    Persistencia persistencia;
    Historial leido;
    bool valido = persistencia.cargarHistorial(ruta, &leido);
    Jugador nuevo;
    std::strcpy(nuevo.Alias, "Luis");
    nuevo.posicion = 2;
    nuevo.puntaje = 80;
    nuevo.correctos_color[1] = 2;
    nuevo.dano_por_tipo[0] = 10;
    valido &= persistencia.guardarPartida(ruta, &nuevo, 2, 43, "turnos");

    std::ifstream archivo(ruta);
    std::string encabezado;
    std::getline(archivo, encabezado);
    archivo.close();
    Historial migrado;
    valido &= persistencia.cargarHistorial(ruta, &migrado);
    valido &= verificar(encabezado == Persistencia::FORMATO_CSV && migrado.tamano() == 2 &&
                        migrado.primero()->dato->correctos_color[0] == 0 &&
                        migrado.ultimo()->dato->correctos_color[1] == 2 &&
                        migrado.ultimo()->dato->dano_por_tipo[0] == 10,
                        "CSV antiguo se migra y conserva sus filas con nuevos campos en cero");
    return valido;
}

class CapturaFlujo {
    std::ostream& flujo;
    std::ostringstream salida;
    std::streambuf* anterior;
    std::ios::fmtflags formato;
    std::streamsize precision;
    char relleno;
public:
    explicit CapturaFlujo(std::ostream& flujo)
        : flujo(flujo), anterior(flujo.rdbuf(salida.rdbuf())),
          formato(flujo.flags()), precision(flujo.precision()), relleno(flujo.fill()) {}
    ~CapturaFlujo() {
        flujo.rdbuf(anterior);
        flujo.flags(formato);
        flujo.precision(precision);
        flujo.fill(relleno);
    }
    std::string texto() const { return salida.str(); }
};

bool iniciarMotor(Motor& motor, int cantidad = 1, int semilla = 42) {
    for (int i = 0; i < cantidad; i++) {
        std::string nombre = "BOT" + std::to_string(i + 1), aviso;
        if (!verificar(motor.RegistrarNombre(nombre, aviso), "registro del bot")) return false;
    }
    motor.Semilla = semilla;
    motor.IniciarMultijugador();
    motor.EnTurno = false;
    return true;
}

void prepararEscena(Motor& motor) {
    motor.EnemigosPendientes = ColaSpawn{};
    DatosSpawn futuro{};
    futuro.tick = 1000000; // Evita terminar el turno al eliminar el ultimo enemigo de la escena.
    SpawnPtr dato = &futuro;
    motor.EnemigosPendientes.Encolar(dato);
    motor.TickOleada = 0;
}

NodoEnemigoPtr agregarEnemigo(Motor& motor, double radio, double angulo, int color,
                             TipoEnemigo tipo = TipoEnemigo::Dron) {
    Enemigo enemigo{};
    enemigo.id = motor.SiguienteId++;
    enemigo.tipo = tipo;
    enemigo.r = radio;
    enemigo.theta = angulo;
    enemigo.color = color;
    enemigo.vida = tipo == TipoEnemigo::Tanque ? 3 : 1;
    enemigo.radio = tipo == TipoEnemigo::Mini ? 4 : tipo == TipoEnemigo::Divisor ? 8 : 6;
    EnemigoPtr dato = &enemigo;
    return motor.Enemigos.InsertarFinal(dato);
}

void agregarBala(Motor& motor, double radio, double angulo, int color) {
    Bala bala{};
    bala.x = 400 + std::cos(angulo) * radio;
    bala.y = 310 - std::sin(angulo) * radio;
    bala.color = color;
    bala.duenio = 1;
    BalaPtr dato = &bala;
    motor.Balas.InsertarFinal(dato);
}

int contarEventos(Motor& motor, TipoEvento tipo) {
    int cantidad = 0;
    for (NodoEventoPtr nodo = motor.Bitacora.primero(); nodo; nodo = nodo->siguiente)
        if (nodo->dato->tipo_evento == tipo) cantidad++;
    return cantidad;
}

std::string reportesHistorial(Historial& historial) {
    std::ostringstream datos;
    datos << std::setprecision(17);
    Estadisticas estadisticas;
    Ranking ranking;
    ranking.ConstruirDesdeHistorial(&historial);
    for (NodoHistorialCSVPtr nodo = historial.primero(); nodo; nodo = nodo->siguiente) {
        const HistorialCSV& r = *nodo->dato;
        datos << r.id_partida << ' ' << r.semilla << ' ' << std::quoted(r.modo) << ' '
              << std::quoted(r.jugador) << ' ' << r.es_bot << ' ' << r.posicion << ' '
              << r.puntaje << ' ' << r.oleada << ' ' << r.disparos << ' ' << r.correctos << ' '
              << r.erroneos << ' ' << r.combo_max << ' ' << r.pulsos << ' ' << r.dano_total << ' '
              << r.ticks << ' ' << r.bajas_dron << ' ' << r.bajas_girador << ' '
              << r.bajas_divisor << ' ' << r.bajas_mini << ' ' << r.bajas_tanque << ' '
              << r.bajas_camaleon;
        for (int valor : r.correctos_color) datos << ' ' << valor;
        for (int valor : r.erroneos_color) datos << ' ' << valor;
        for (int valor : r.bajas_color) datos << ' ' << valor;
        for (int valor : r.dano_por_tipo) datos << ' ' << valor;
        datos << '\n';
        const ResumenHistorico resumen = estadisticas.calcularResumenHistorico(&historial, r.jugador);
        datos << resumen.media << ' ' << resumen.mediana << ' ' << resumen.desviacion_estandar
              << ' ' << resumen.precision_promedio << '\n';
    }
    for (NodoRegistroPtr nodo = ranking.primero(); nodo; nodo = nodo->siguiente)
        datos << nodo->dato->Alias << ' ' << nodo->dato->puntaje << ' ' << nodo->dato->presicion << '\n';
    {
        CapturaFlujo captura(std::cout);
        for (NodoRegistroPtr nodo = ranking.primero(); nodo; nodo = nodo->siguiente)
            estadisticas.imprimirReporteHistorico(&historial, nodo->dato->Alias);
        ranking.ImprimirTop();
        datos << captura.texto();
    }
    return datos.str();
}

std::string reporteMotor(Motor& motor) {
    motor.GuardarJugador();
    Historial historial;
    Persistencia persistencia;
    for (int i = 0; i < motor.Registrados; i++) {
        HistorialCSVPtr registro = new HistorialCSV{};
        if (!persistencia.crearRegistro(&motor.DatosJugadores[i], 1, motor.Semilla, "script", *registro)) {
            delete registro;
            std::cerr << "No se pudo construir el reporte del motor.\n";
            return {};
        }
        historial.InsertarFinal(registro);
    }
    std::ostringstream salida;
    salida << std::setprecision(17) << reportesHistorial(historial)
           << motor.Oleada << ' ' << motor.TickOleada << ' ' << motor.VidaNucleo << ' '
           << motor.Puntaje << ' ' << motor.Combo << ' ' << motor.Angulo << ' '
           << motor.CargaPulso << ' ' << motor.PulsoActivo << ' ' << motor.RadioPulso << '\n';
    for (NodoEnemigoPtr nodo = motor.Enemigos.primero(); nodo; nodo = nodo->siguiente) {
        const Enemigo& e = *nodo->dato;
        salida << e.id << ' ' << static_cast<int>(e.tipo) << ' ' << e.r << ' ' << e.theta
               << ' ' << e.omega << ' ' << e.color << ' ' << e.vida << ' ' << e.radio
               << ' ' << e.ticksColor << '\n';
    }
    for (NodoBalaPtr nodo = motor.Balas.primero(); nodo; nodo = nodo->siguiente) {
        const Bala& b = *nodo->dato;
        salida << b.x << ' ' << b.y << ' ' << b.vx << ' ' << b.vy << ' '
               << b.color << ' ' << b.duenio << '\n';
    }
    for (NodoEventoPtr nodo = motor.Bitacora.primero(); nodo; nodo = nodo->siguiente) {
        const Evento& e = *nodo->dato;
        salida << e.tick << ' ' << e.oleada << ' ' << e.jugador->Alias << ' '
               << static_cast<int>(e.tipo_evento) << ' ' << static_cast<int>(e.tipo_enemigo)
               << ' ' << e.color << ' ' << e.valor << '\n';
    }
    {
        CapturaFlujo captura(std::cout);
        Estadisticas estadisticas;
        estadisticas.imprimirLineaTiempo(&motor.Bitacora, &motor.DatosJugadores[0]);
        salida << captura.texto();
    }
    return salida.str();
}

bool pruebaP08() {
    std::vector<AccionesTick> script;
    for (int tick = 0; tick < 900; tick++) {
        AccionesTick accion{};
        accion.Izquierda = tick % 40 < 20;
        accion.Derecha = !accion.Izquierda;
        accion.color = (tick / 13) % 3;
        accion.angulo = (tick % 360) * Pi / 180;
        accion.Radio = 20;
        script.push_back(accion);
    }
    Motor primero, segundo, otraSemilla;
    if (!iniciarMotor(primero) || !iniciarMotor(segundo) || !iniciarMotor(otraSemilla, 1, 43)) return false;
    primero.DatosPartida.posicion = segundo.DatosPartida.posicion = otraSemilla.DatosPartida.posicion = 1;
    bool valido = true;
    for (const AccionesTick& accion : script) {
        primero.EnTurno = segundo.EnTurno = otraSemilla.EnTurno = false;
        primero.AvanzarTick(accion);
        segundo.AvanzarTick(accion);
        otraSemilla.AvanzarTick(accion);
        const std::string reporte = reporteMotor(primero);
        if (!verificar(!reporte.empty() && reporte == reporteMotor(segundo),
                       "P08: reportes identicos despues de cada accion con semilla y script iguales")) {
            valido = false;
            break;
        }
    }
    valido &= verificar(primero.DatosPartida.disparos > 0 && primero.Bitacora.tamano() > 0,
                        "P08: ejecucion no trivial con disparos y eventos");
    otraSemilla.Semilla = primero.Semilla; // Compara resultados, no solo el campo semilla del reporte.
    valido &= verificar(reporteMotor(primero) != reporteMotor(otraSemilla),
                        "P08: control con otra semilla produce otro reporte");
    return valido;
}

bool pruebaP09() {
    Motor motor, control;
    if (!iniciarMotor(motor) || !iniciarMotor(control)) return false;
    prepararEscena(motor);
    prepararEscena(control);
    NodoEnemigoPtr objetivo = agregarEnemigo(motor, 200, 0, 0, TipoEnemigo::Tanque);
    NodoEnemigoPtr referencia = agregarEnemigo(control, 200, 0, 0, TipoEnemigo::Tanque);
    agregarBala(motor, 200, 0, 1);
    motor.Combo = 5;
    motor.AvanzarTick();
    control.AvanzarTick();
    return verificar(motor.Enemigos.tamano() == 1 && motor.Balas.vacia() &&
                     std::fabs(objetivo->dato->r - referencia->dato->r - 9) < 0.000001 &&
                     objetivo->dato->vida == 3 && motor.VidaNucleo == 100 && motor.Combo == 0 &&
                     motor.DatosPartida.erroneos == 1 && motor.DatosPartida.erroneos_color[1] == 1 &&
                     contarEventos(motor, TipoEvento::IMPACTO_ERRONEO) == 1,
                     "P09: retroceso neto de 9, combo cero, vidas intactas e impacto registrado");
}

bool pruebaP10() {
    bool valido = true;
    for (int color = 0; color < 3; color++) {
        Motor motor;
        if (!iniciarMotor(motor)) return false;
        prepararEscena(motor);
        NodoEnemigoPtr anterior = agregarEnemigo(motor, 250, Pi, 0);
        const int idPadre = agregarEnemigo(motor, 200, 0, color, TipoEnemigo::Divisor)->dato->id;
        NodoEnemigoPtr siguiente = agregarEnemigo(motor, 250, Pi / 2, 1);
        const int primerIdMini = motor.SiguienteId;
        agregarBala(motor, 200, 0, color);
        motor.AvanzarTick();
        valido &= verificar(motor.Enemigos.Buscar(idPadre) == nullptr &&
                            motor.Enemigos.tamano() == 4 && motor.DatosPartida.bajas_divisor == 1,
                            "P10: elimina al Divisor e inserta exactamente dos enemigos");
        NodoEnemigoPtr mini = anterior->siguiente;
        for (int i = 0; i < 2; i++) {
            if (!verificar(mini != nullptr && mini != siguiente, "P10: Mini en la posicion de lista del padre"))
                return false;
            valido &= verificar(mini->dato->tipo == TipoEnemigo::Mini &&
                                mini->dato->id == primerIdMini + i &&
                                std::fabs(mini->dato->r - (200 - 0.88 * 0.8)) < 0.000001 &&
                                std::fabs(mini->dato->theta - (i == 0 ? -4 : 4) * Pi / 180) < 0.000001 &&
                                mini->dato->color == (color + i + 1) % 3 &&
                                mini->dato->vida == 1,
                                "P10: Minis al radio del padre, con apertura angular y colores diferentes");
            mini = mini->siguiente;
        }
        valido &= verificar(mini == siguiente && siguiente->anterior->anterior->anterior == anterior,
                            "P10: conserva vecinos y enlaces de la lista");
    }
    return valido;
}

bool pruebaP11() {
    bool valido = true;
    for (int carga : {0, 99, 100}) {
        Motor motor;
        if (!iniciarMotor(motor)) return false;
        prepararEscena(motor);
        agregarEnemigo(motor, 32.88, 0, 0);
        agregarEnemigo(motor, 32.88, Pi, 1);
        NodoEnemigoPtr fuera = agregarEnemigo(motor, 60, Pi / 2, 2);
        motor.CargaPulso = carga;
        motor.ActivarPulso();
        valido &= verificar(motor.PulsoActivo == (carga == 100) &&
                            motor.CargaPulso == (carga == 100 ? 0 : carga) &&
                            motor.DatosPartida.pulsos == (carga == 100 ? 1 : 0) &&
                            contarEventos(motor, TipoEvento::PULSO) == (carga == 100 ? 1 : 0),
                            "P11: solo carga 100 activa, consume y registra el pulso");
        motor.AvanzarTick();
        valido &= verificar(motor.Enemigos.tamano() == (carga == 100 ? 1 : 3) &&
                            motor.Enemigos.Buscar(fuera->dato->id) == fuera &&
                            motor.DatosPartida.bajas_dron == (carga == 100 ? 2 : 0) &&
                            motor.Puntaje == (carga == 100 ? 200 : 0),
                            "P11: bajas solo en la banda de la onda, independientemente del color");
    }
    return valido;
}

bool pruebaP12() {
    struct Caso {
        const char* nombre;
        std::vector<int> entrada, orden;
        double mediana;
    };
    const Caso casos[] = {
        {"vacia", {}, {}, 0},
        {"uno", {40}, {40}, 40},
        {"dos ordenados", {10, 40}, {10, 40}, 25},
        {"dos inversos", {40, 10}, {10, 40}, 25},
        {"repetidos", {30, 10, 30, 40, 30}, {10, 30, 30, 30, 40}, 30},
        {"repetidos par", {40, 30, 10, 30}, {10, 30, 30, 40}, 30}
    };
    bool valido = true;
    for (const Caso& caso : casos) {
        Historial historial;
        for (int puntaje : caso.entrada) agregarPuntaje(historial, puntaje);
        Estadisticas estadisticas;
        const std::string mensaje = std::string("P12: orden, mediana e historial intacto: ") + caso.nombre;
        valido &= verificar(estadisticas.calcularMediana(&historial, "Ana") == caso.mediana,
                            mensaje.c_str());
        NodoHistorialCSVPtr original = historial.primero();
        for (int puntaje : caso.entrada) {
            if (!verificar(original && original->dato->puntaje == puntaje, mensaje.c_str())) return false;
            original = original->siguiente;
        }
        valido &= verificar(original == nullptr, mensaje.c_str());
        NodoHistorialCSVPtr fuente = historial.primero();
        NodoHistorialCSVPtr copia = historial.CopiarAListaAuxiliar(fuente);
        historial.MergeSort(copia);
        NodoHistorialCSVPtr nodo = copia;
        for (int puntaje : caso.orden) {
            valido &= verificar(nodo && nodo->puntaje == puntaje, mensaje.c_str());
            if (nodo) nodo = nodo->siguiente;
        }
        valido &= verificar(nodo == nullptr, mensaje.c_str());
        while (copia) {
            NodoHistorialCSVPtr siguiente = copia->siguiente;
            delete copia;
            copia = siguiente;
        }
    }
    return valido;
}

bool faseGuardarP13(const std::string& ruta) {
    Jugador ana, luis;
    std::strcpy(ana.Alias, "Ana");
    std::strcpy(luis.Alias, "BOT Luis");
    ana.posicion = 1;
    luis.posicion = 2;
    luis.isBot = true;
    ana.puntaje = 300;
    luis.puntaje = 100;
    ana.numero_oleada = luis.numero_oleada = 10;
    ana.correctos = 3;
    ana.erroneos = 1;
    ana.disparos = 8;
    ana.combo_max = 3;
    ana.pulsos = 1;
    ana.ticks = 150;
    ana.bajas_dron = ana.bajas_divisor = ana.bajas_mini = 1;
    ana.bajas_girador = ana.bajas_tanque = ana.bajas_camaleon = 1;
    for (int color = 0; color < 3; color++) {
        ana.correctos_color[color] = 1;
        ana.erroneos_color[color] = color == 0 ? 1 : 0;
        ana.bajas_color[color] = 2;
    }
    for (int tipo = 0; tipo < 6; tipo++) {
        ana.dano_por_tipo[tipo] = 10 + tipo;
        ana.dano_total += 10 + tipo;
    }
    luis.correctos = 1;
    luis.erroneos = 1;
    JugadorPtr jugadores[] = {&ana, &luis};
    Historial historial;
    Persistencia persistencia;
    if (!verificar(persistencia.guardarPartidaCompleta(ruta, jugadores, 2, 42, "turnos", &historial),
                   "P13: guarda primera partida")) return false;
    ana.puntaje = 100;
    luis.puntaje = 300;
    ana.posicion = 2;
    luis.posicion = 1;
    if (!verificar(persistencia.guardarPartidaCompleta(ruta, jugadores, 2, 43, "turnos", &historial),
                   "P13: guarda segunda partida")) return false;
    std::ofstream esperado(ruta + ".esperado");
    esperado << reportesHistorial(historial);
    esperado.close();
    std::ofstream archivo(ruta, std::ios::app);
    archivo << "fila,invalida\n";
    archivo << "999999999999999999999,42,turnos,Ana\n";
    archivo.close();
    return verificar(static_cast<bool>(esperado) && static_cast<bool>(archivo), "P13: escribe fixtures");
}

bool faseCargarP13(const std::string& ruta) {
    Historial historial;
    Persistencia persistencia;
    bool cargado;
    std::string errores;
    {
        CapturaFlujo captura(std::cerr);
        cargado = persistencia.cargarHistorial(ruta, &historial);
        errores = captura.texto();
    }
    std::ifstream archivo(ruta + ".esperado");
    const std::string esperado{std::istreambuf_iterator<char>(archivo), std::istreambuf_iterator<char>()};
    return verificar(cargado && historial.tamano() == 4 && archivo.is_open() && !archivo.bad() &&
                     !esperado.empty() && reportesHistorial(historial) == esperado,
                     "P13: proceso nuevo conserva todos los campos, estadisticas y ranking") &&
           verificar(errores.find("Fila mal formada en línea 6; se omite.") != std::string::npos &&
                     errores.find("Fila mal formada en línea 7; se omite.") != std::string::npos,
                     "P13: identifica las dos filas invalidas tras reiniciar");
}

bool ejecutarProceso(const char* ejecutable, const char* fase, const std::string& ruta) {
    std::cout.flush();
    std::cerr.flush();
    const pid_t hijo = fork();
    if (hijo < 0) {
        std::perror("P13 fork");
        return false;
    }
    if (hijo == 0) {
        execlp(ejecutable, ejecutable, fase, ruta.c_str(), static_cast<char*>(nullptr));
        std::perror("P13 exec");
        _exit(127);
    }
    int estado = 0;
    pid_t resultado;
    do {
        resultado = waitpid(hijo, &estado, 0);
    } while (resultado == -1 && errno == EINTR);
    if (resultado == -1) {
        std::perror("P13 waitpid");
        return false;
    }
    return verificar(WIFEXITED(estado) && WEXITSTATUS(estado) == 0, "P13: subproceso termina correctamente");
}

bool pruebaP13(const char* ejecutable, const std::string& ruta) {
    const bool valido = ejecutarProceso(ejecutable, "--p13-guardar", ruta) &&
                        ejecutarProceso(ejecutable, "--p13-cargar", ruta);
    bool limpieza = true;
    for (const std::string& archivo : {ruta, ruta + ".esperado"}) {
        if (std::remove(archivo.c_str()) != 0 && errno != ENOENT) {
            std::perror("P13 limpieza");
            limpieza = false;
        }
    }
    return valido && limpieza;
}

bool pruebaP14() {
    Motor motor;
    if (!iniciarMotor(motor, 4)) return false;
    std::array<std::array<bool, 10>, 4> jugadas{};
    int ticks = 0;
    while (!motor.GameOver && !motor.Victoria && ticks < 200000) {
        motor.EnTurno = false;
        if (motor.Oleada >= 1 && motor.Oleada <= 10)
            jugadas[motor.IndiceActual][motor.Oleada - 1] = true;
        motor.AvanzarTick(Controlador::AccionesBot(motor));
        ticks++;
    }
    int turnos = 0;
    for (const auto& jugador : jugadas)
        for (bool jugada : jugador) if (jugada) turnos++;
    std::cout << "P14 escenario: " << ticks << " ticks, " << turnos
              << "/40 turnos, " << contarEventos(motor, TipoEvento::FIN_OLEADA)
              << "/10 oleadas completadas.\n";
    return verificar(ticks < 200000 && motor.Victoria && motor.Oleada == 10 && turnos == 40 &&
                     contarEventos(motor, TipoEvento::FIN_OLEADA) == 10 &&
                     contarEventos(motor, TipoEvento::ELIMINACION) == 0,
                     "P14: cuatro bots completan las diez oleadas sin alterar vidas ni reglas");
}

bool pruebaBotSinObjetivo() {
    Motor motor;
    if (!iniciarMotor(motor)) return false;
    motor.ColorSeleccionado = 2;
    motor.Angulo = 1.25f;
    bool valido = true;
    for (bool conEnemigos : {false, true}) {
        if (conEnemigos) {
            agregarEnemigo(motor, 100, 0, 0);
            agregarEnemigo(motor, 99, Pi, 1);
        }
        const AccionesTick accion = Controlador::AccionesBot(motor);
        valido &= verificar(accion.color == 2 && !accion.Izquierda && !accion.Derecha &&
                            accion.angulo == motor.Angulo && motor.Angulo == 1.25f,
                            "BOT01: sin elegibles conserva color y angulo, sin giro");
    }
    return valido;
}

bool pruebaBotObjetivo(bool empate) {
    Motor motor;
    if (!iniciarMotor(motor)) return false;
    motor.Angulo = 0;
    agregarEnemigo(motor, empate ? 150 : 220, Pi, 0);
    const double angulo = empate ? Pi : Pi / 2;
    if (!empate) agregarEnemigo(motor, 150, angulo, 1);
    agregarEnemigo(motor, 150, 3 * Pi / 2, 2);
    agregarEnemigo(motor, 100, 0, 2);
    agregarEnemigo(motor, 99, 0, 2);
    const AccionesTick accion = Controlador::AccionesBot(motor);
    bool valido = verificar(accion.color == (empate ? 0 : 1) && accion.Izquierda && !accion.Derecha,
                            "BOT02/BOT03: menor radio elegible; empate conserva el primero");
    valido &= verificar(std::fabs(accion.angulo - angulo) < 0.000001,
                        "BOT02/BOT03: devuelve el angulo del objetivo seleccionado");
    return valido;
}

bool pruebaBotGiro(bool acumulado) {
    bool valido = true;
    for (bool inverso : {false, true}) {
        Motor motor;
        if (!iniciarMotor(motor)) return false;
        motor.Angulo = static_cast<float>((inverso ? 1 : 359) * Pi / 180);
        const double equivalente = (inverso ? 359 : 1) * Pi / 180;
        NodoEnemigoPtr objetivo = agregarEnemigo(motor, 200, equivalente, 1);
        const AccionesTick base = Controlador::AccionesBot(motor);
        valido &= verificar(base.Izquierda == !inverso && base.Derecha == inverso,
                            "BOT04/BOT05: giro corto 359 -> 1 izquierda, inverso derecha");
        if (acumulado) {
            for (int vueltas : {3, 7}) {
                objetivo->dato->theta = equivalente + vueltas * 2 * Pi;
                const AccionesTick accion = Controlador::AccionesBot(motor);
                valido &= verificar(accion.Izquierda == base.Izquierda && accion.Derecha == base.Derecha,
                                    "BOT05: varias vueltas conservan el giro del angulo equivalente");
            }
        }
    }
    return valido;
}

bool pruebaBotPulso(int cantidad, int carga) {
    Motor motor;
    if (!iniciarMotor(motor)) return false;
    prepararEscena(motor);
    for (int i = 0; i < cantidad; i++) agregarEnemigo(motor, 150, i * Pi / 3, i % 3);
    agregarEnemigo(motor, 160, Pi, 0);
    agregarEnemigo(motor, 200, Pi, 1);
    motor.CargaPulso = carga;
    const AccionesTick accion = Controlador::AccionesBot(motor);
    const bool esperado = cantidad >= 5 && carga == 100;
    bool valido = verificar(motor.PulsoActivo == esperado &&
                            motor.CargaPulso == (esperado ? 0 : carga) &&
                            motor.DatosPartida.pulsos == (esperado ? 1 : 0) &&
                            contarEventos(motor, TipoEvento::PULSO) == (esperado ? 1 : 0),
                            "BOT06/BOT07/BOT08: umbral estricto r < 160 y carga completa");
    if (esperado) {
        motor.AvanzarTick(accion);
        Controlador::AccionesBot(motor);
        valido &= verificar(motor.DatosPartida.pulsos == 1 && contarEventos(motor, TipoEvento::PULSO) == 1,
                            "BOT07: no reactiva ni duplica el registro del pulso activo");
    }
    return valido;
}

bool pruebaBotOndaPersistente() {
    Motor motor;
    if (!iniciarMotor(motor)) return false;
    prepararEscena(motor);
    motor.CargaPulso = 100;
    motor.ActivarPulso();
    AccionesTick accion = Controlador::AccionesBot(motor);
    bool valido = verificar(!accion.Pulso && motor.PulsoActivo, "BOT09: pulso activo con orden falsa");
    float anterior = motor.RadioPulso;
    for (int tick = 0; tick < 3; tick++) {
        accion = Controlador::AccionesBot(motor);
        accion.Pulso = false;
        motor.AvanzarTick(accion);
        valido &= verificar(motor.PulsoActivo && motor.RadioPulso == anterior + 12 &&
                            motor.DatosPartida.pulsos == 1 && motor.CargaPulso == 0,
                            "BOT09: orden falsa no cancela ni reinicia la onda y avanza 12 por tick");
        anterior = motor.RadioPulso;
    }
    return valido;
}

void reportar(const char* nombre, bool resultado, int& fallidas) {
    std::cout << nombre << (resultado ? " PASS\n" : " FAIL\n");
    if (!resultado) fallidas++;
}
}

int main(int argc, char* argv[]) {
    if (argc == 3 && std::string(argv[1]) == "--p13-guardar")
        return faseGuardarP13(argv[2]) ? 0 : 1;
    if (argc == 3 && std::string(argv[1]) == "--p13-cargar")
        return faseCargarP13(argv[2]) ? 0 : 1;
    if (argc == 2 && std::string(argv[1]) == "--p14")
        return pruebaP14() ? 0 : 1;
    int fallidas = 0;
    reportar("P01", pruebaP01(), fallidas);
    reportar("P02", pruebaP02(), fallidas);
    reportar("P03", pruebaP03(), fallidas);
    reportar("P04", pruebaP04(), fallidas);
    reportar("P05", pruebaP05(), fallidas);
    reportar("P06", pruebaP06(), fallidas);
    reportar("P07", pruebaP07(), fallidas);
    reportar("P08", pruebaP08(), fallidas);
    reportar("P09", pruebaP09(), fallidas);
    reportar("P10", pruebaP10(), fallidas);
    reportar("P11", pruebaP11(), fallidas);
    reportar("P12", pruebaP12(), fallidas);
    if (argc > 1) reportar("P13", pruebaP13(argv[0], std::string(argv[1]) + ".reinicio"), fallidas);
    else std::cout << "P13 SKIP: requiere una ruta temporal de historial (make pruebas).\n";
    reportar("P14/escenario", pruebaP14(), fallidas);
    std::cout << "P14/Valgrind SKIP: ejecutar sh tests/p14_valgrind.sh para verificar memoria.\n";
    reportar("BOT01", pruebaBotSinObjetivo(), fallidas);
    reportar("BOT02", pruebaBotObjetivo(false), fallidas);
    reportar("BOT03", pruebaBotObjetivo(true), fallidas);
    reportar("BOT04", pruebaBotGiro(false), fallidas);
    reportar("BOT05", pruebaBotGiro(true), fallidas);
    reportar("BOT06", pruebaBotPulso(4, 100), fallidas);
    reportar("BOT07", pruebaBotPulso(5, 100), fallidas);
    reportar("BOT08", pruebaBotPulso(5, 99), fallidas);
    reportar("BOT09", pruebaBotOndaPersistente(), fallidas);
    reportar("G01", pruebaGeneradorYOleada(), fallidas);
    reportar("E01", pruebaEstadisticas(), fallidas);
    reportar("E02", pruebaPrecisionYDano(), fallidas);
    reportar("M01", pruebaPropiedadDeDatos(), fallidas);
    reportar("H01", pruebaHistorial(), fallidas);
    reportar("R01", pruebaReportesYRanking(), fallidas);
    if (argc > 1) reportar("CSV02", pruebaGuardarParticipantes(std::string(argv[1]) + ".partida"), fallidas);
    if (argc > 1) reportar("CSV01", pruebaPersistencia(argv[1]), fallidas);
    if (argc > 1) reportar("CSV03", pruebaMigracionCSVAntiguo(std::string(argv[1]) + ".legacy"), fallidas);
    std::cout << (fallidas == 0 ? "Todas las pruebas ejecutadas pasaron.\n" : "Hay pruebas fallidas.\n");
    return fallidas == 0 ? 0 : 1;
}

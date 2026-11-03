#include "../include/Estructuras/ListaEnemigos.h"
#include "../include/Estructuras/ListaBalas.h"
#include "../include/Estructuras/ColaSpawn.h"
#include "../include/Estructuras/RondaJugadores.h"
#include "../include/Motor/Generador.h"
#include "../include/Motor/Oleada.h"
#include "../include/Estadisticas/Estadisticas.h"
#include "../include/Estadisticas/Persistencia.h"
#include "../include/Estadisticas/Ranking.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

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

void reportar(const char* nombre, bool resultado, int& fallidas) {
    std::cout << nombre << (resultado ? " PASS\n" : " FAIL\n");
    if (!resultado) fallidas++;
}
}

int main(int argc, char* argv[]) {
    int fallidas = 0;
    reportar("P01", pruebaP01(), fallidas);
    reportar("P02", pruebaP02(), fallidas);
    reportar("P03", pruebaP03(), fallidas);
    reportar("P04", pruebaP04(), fallidas);
    reportar("P05", pruebaP05(), fallidas);
    reportar("P06", pruebaP06(), fallidas);
    reportar("P07", pruebaP07(), fallidas);
    reportar("G01", pruebaGeneradorYOleada(), fallidas);
    reportar("E01", pruebaEstadisticas(), fallidas);
    reportar("E02", pruebaPrecisionYDano(), fallidas);
    reportar("M01", pruebaPropiedadDeDatos(), fallidas);
    reportar("H01", pruebaHistorial(), fallidas);
    reportar("R01", pruebaReportesYRanking(), fallidas);
    if (argc > 1) reportar("CSV02", pruebaGuardarParticipantes(std::string(argv[1]) + ".partida"), fallidas);
    if (argc > 1) reportar("CSV01", pruebaPersistencia(argv[1]), fallidas);
    if (argc > 1) reportar("CSV03", pruebaMigracionCSVAntiguo(std::string(argv[1]) + ".legacy"), fallidas);
    std::cout << (fallidas == 0 ? "Todas las pruebas pasaron.\n" : "Hay pruebas fallidas.\n");
    return fallidas == 0 ? 0 : 1;
}

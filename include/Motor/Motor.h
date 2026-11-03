#pragma once

#include <string>

#include "Estructuras/ListaBalas.h"
#include "Estructuras/ListaEnemigos.h"
#include "Motor/Oleada.h"
#include "Estructuras/ColaSpawn.h"
#include "Estructuras/RondaJugadores.h"
#include "Estadisticas/BitacoraEventos.h"

const double Pi = 3.14159265358979323846;

class Motor
{
public:
    bool EnTurno = false; // para mostrar el siguiente turno
    int Registrados = 0;
    int IndiceActual = 0;
    Jugador DatosJugadores[6];
    RondaJugadores Ronda;
    ColaSpawn ColaOriginal; // guarda la oleada para los demas jugadores
    int JugadoresTurno = 0;
    int TurnosJugados = 0;
    bool Horario = true;
    bool EnPausa = false;
    bool Izquierda = false;
    bool Derecha = false;

    // Posicion de la nave
    float Angulo = static_cast<float>(3.0 * Pi / 2.0); // 270 grados iniciales
    const float Radio = 100; // Distancia de la nave al nucleo

    // listas que guardan las balas y los enemigos
    ListaBalas Balas;
    ListaEnemigos Enemigos;
    BitacoraEventos Bitacora;

    // esto es para los enemigos de cada oleada
    ::Oleada GeneradorOleadas;
    ColaSpawn EnemigosPendientes;
    int TickOleada = -1;

    int CargaPulso = 0; // carga de 0 a 100
    bool PulsoActivo = false;
    float RadioPulso = 20; // radio de la onda
    int Puntaje = 0; // puntos de la partida
    int ColorSeleccionado = 0; // Se empieza utilizando cian
    int Combo = 0; // aumenta al eliminar enemigos
    Jugador DatosPartida; // estadisticas de la partida actual
    int Semilla = 0; // Semilla de la partida actual

    // variables nuevas para controlar las oleadas
    int Oleada = 1; // la partida empieza en la oleada uno
    int SiguienteId = 1; // Numero que se asigna al proximo enemigo
    int EsperaOleada = 0; // Tiempo antes de comenzar otra oleada
    bool OleadaTerminada = false; // Indica si ya eliminamos todos
    bool GameOver = false; // indica si perdimos la partida
    bool Victoria = false; // indica si se gana la partida
    int VidaNucleo = 100; // Empieza con 100
    const int TotalOleadas = 5; // Hay que superar cinco oleadas

    Motor();
    void ReiniciarPartida(Jugador& Datos, int& Semilla);
    void GuardarJugador();
    void IniciarMultijugador();
    void AvanzarTick();
    void ActivarPulso();
    void CalcularPosiciones();
    bool RegistrarNombre(std::string& NombreJugador, std::string& AvisoRegistro);
    void ReiniciarRegistro();
    void ReiniciarDesdeVictoria();
    void ReiniciarDesdeMenu();

private:
    void RegistrarEvento(TipoEvento tipo, TipoEnemigo enemigo, int color, int valor);
    void RegistrarBaja(TipoEnemigo tipo, int color, int puntos);
    void RegistrarDano(int dano, TipoEnemigo tipo, TipoEvento evento);
    void PrepararTurno();
    void TerminarTurno(bool Eliminado);
};

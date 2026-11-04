#pragma once
#include "Motor.h"
class Motor;

enum class Accion
{
    Ninguna, ConfirmarTurno, Pausa, Izquierda, Derecha, Cian, Magenta, Ambar, Pulso
};

class Controlador
{
public:
    static void Presionar(Accion accion, Motor& Juego, bool EnMenu, bool EnNombre);
    static void Soltar(Accion accion, Motor& Juego);
    static AccionesTick AccionesBot(Motor& estadoMotor);
};

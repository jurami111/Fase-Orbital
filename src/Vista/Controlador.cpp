#include "Vista/Controlador.h"
#include "Motor/Motor.h"
#include <iostream>


void Controlador::Presionar(Accion accion, Motor& Juego, bool EnMenu, bool EnNombre)
{
    if (Juego.EnTurno && accion == Accion::ConfirmarTurno) Juego.EnTurno = false;

    // La tecla P pone y quita la pausa
    if (!EnMenu && !EnNombre && !Juego.EnTurno && !Juego.GameOver && !Juego.Victoria && accion == Accion::Pausa)
    {
        Juego.EnPausa = !Juego.EnPausa;
    }

    // la flecha izquierda mueve la nave en una direccion
    if (!EnMenu && !EnNombre && !Juego.EnTurno && !Juego.EnPausa && accion == Accion::Izquierda)
    {
        Juego.Izquierda = true;
    }

    // La flecha derecha la mueve en la otra direccion
    if (!EnMenu && !EnNombre && !Juego.EnTurno && !Juego.EnPausa && accion == Accion::Derecha)
    {
        Juego.Derecha = true;
    }

    // se selecciona el color cian con 1 o J
    if (!EnMenu && !EnNombre && !Juego.EnTurno && !Juego.EnPausa && accion == Accion::Cian)
    {
        Juego.ColorSeleccionado = 0;
    }

    // Se selecciona magenta con 2 o K
    if (!EnMenu && !EnNombre && !Juego.EnTurno && !Juego.EnPausa && accion == Accion::Magenta)
    {
        Juego.ColorSeleccionado = 1;
    }

    // se selecciona ambar con 3 o L
    if (!EnMenu && !EnNombre && !Juego.EnTurno && !Juego.EnPausa && accion == Accion::Ambar)
    {
        Juego.ColorSeleccionado = 2;
    }
    // el pulso se activa solo cuando la carga esta completa
    if (!EnMenu && !EnNombre && !Juego.EnTurno && !Juego.EnPausa && !Juego.GameOver && !Juego.Victoria && accion == Accion::Pulso)
    {
        Juego.ActivarPulso();
    }

}

// se revisa cuando se sueltan las flechas
void Controlador::Soltar(Accion accion, Motor& Juego)
{
    if (accion == Accion::Izquierda)
    {
        Juego.Izquierda = false; // se deja de girar a la izquierda
    }

    if (accion == Accion::Derecha)
    {
        Juego.Derecha = false; // se deja de girar a la derecha
    }
}

AccionesTick Controlador::AccionesBot(Motor& estadoMotor)
{
    AccionesTick Acciones;
    Acciones.Izquierda = false;
    Acciones.Derecha = false;
    Acciones.Pulso = false;
    bool hayObjetivo = false;
    Acciones.color = estadoMotor.ColorSeleccionado;
    Acciones.angulo = estadoMotor.Angulo;
    Acciones.Radio = estadoMotor.RadioPulso;
    NodoEnemigoPtr actual = estadoMotor.Enemigos.primero();
    double EnemigoaAtacar = 99999999;
    int EnemigoColor = -1;
    double EnemigoAngulo = -1000;
    int cantidadEnemigos = 0;
    double diferencia = 0;

    while (actual != nullptr)
    {
        Enemigo* enemigo = actual->dato;
        double enemigoAngulo = enemigo->theta; // se puede usar para decidir las acciones del bot
        int enemigoColor = enemigo->color; // se puede usar para decidir las acciones del bot
        if (enemigo->r > 100 && enemigo->r < EnemigoaAtacar){
            EnemigoaAtacar = enemigo->r;
            Acciones.angulo = enemigoAngulo;
            EnemigoColor = enemigoColor;
            hayObjetivo = true;
        }

        if (enemigo->r < 160) cantidadEnemigos++;

        actual = actual->siguiente;
    }
    if(!hayObjetivo){
        // Si no hay enemigos a atacar, el bot no gira
        Acciones.Izquierda = false;
        Acciones.Derecha = false;
    }

    if (EnemigoColor != -1) Acciones.color = EnemigoColor;

    if (cantidadEnemigos >= 5)
    {
        if (estadoMotor.CanActivarPulso())
            estadoMotor.ActivarPulso();
    }

    return Acciones;
}
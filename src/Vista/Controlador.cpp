#include "Vista/Controlador.h"
#include "Motor/Motor.h"

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

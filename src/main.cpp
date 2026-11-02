#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <allegro5/allegro_image.h>
#include "Estadisticas/Persistencia.h"
#include "Vista/VistaAllegro.h"
#include "Estructuras/ListaBalas.h"
#include <iostream>
#include <string>

#include "Estructuras/ListaEnemigos.h"


int main(int argc, char* argv[])
{
    std::string ArchivoHistorial = "historial.csv";
    std::string CarpetaCapturas;
    for (int indice = 1; indice < argc; indice++) {
        const std::string argumento = argv[indice];
        if (argumento == "--historial" && indice + 1 < argc) ArchivoHistorial = argv[++indice];
        else if (argumento == "--capturas" && indice + 1 < argc) CarpetaCapturas = argv[++indice];
        else {
            std::cerr << "Uso: faseorbital [--historial archivo.csv] [--capturas carpeta]\n";
            return 1;
        }
    }
    Historial HistorialJugadores;
    Persistencia Archivos;
    const bool HistorialCargado = Archivos.cargarHistorial(ArchivoHistorial, &HistorialJugadores);
    Ranking TablaRanking;
    TablaRanking.ConstruirDesdeHistorial(&HistorialJugadores);
    VistaAllegro Reportes;
    int PasoCaptura = 0;

    // Iniciar allegro y lo que vamos a usar
    al_init();
    al_install_keyboard();
    if (!al_install_mouse()) {
        std::cerr << "No se pudo instalar el mouse.\n";
        return 1;
    }
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_ttf_addon();
    if (!CarpetaCapturas.empty() && !al_init_image_addon()) {
        std::cerr << "No se pudo inicializar el addon de imagen.\n";
        return 1;
    }

    // Crear la ventana del juego
    ALLEGRO_DISPLAY* Ventana = al_create_display(800, 600);
    ALLEGRO_EVENT_QUEUE* Cola = al_create_event_queue();
    ALLEGRO_TIMER* Timer = al_create_timer(1.0 / 30);

    // Fuente para los textos
    ALLEGRO_FONT* Fuente = al_load_ttf_font(
        "C:/Windows/Fonts/consola.ttf", 22, 0
    );

    if (Fuente == NULL)
        Fuente = al_create_builtin_font();

    // Revisar que todo se haya creado
    if (Ventana == NULL || Cola == NULL || Timer == NULL || Fuente == NULL)
        return 1;

    al_set_window_title(Ventana, "Fase Orbital");

    // Registrar los eventos
    al_register_event_source(Cola, al_get_display_event_source(Ventana));
    al_register_event_source(Cola, al_get_keyboard_event_source());
    al_register_event_source(Cola, al_get_timer_event_source(Timer));
    al_register_event_source(Cola, al_get_mouse_event_source());

    // Colores de la interfaz
    ALLEGRO_COLOR Fondo = al_map_rgb(20, 29, 43);
    ALLEGRO_COLOR Crema = al_map_rgb(235, 242, 246);
    ALLEGRO_COLOR Morado = al_map_rgb(103, 129, 157);
    ALLEGRO_COLOR Coral = al_map_rgb(123, 212, 195);

    // Colores que pueden tener los enemigos
    ALLEGRO_COLOR Colores[3] = {
        al_map_rgb(68, 210, 225),   // Cian
        al_map_rgb(238, 91, 176),   // Magenta
        al_map_rgb(245, 183, 73)    // Ambar
    };

    // Para saber si se presionan las teclas
    bool Salir = false;
    bool EnMenu = true;
    bool Izquierda = false;
    bool Derecha = false;

    // Posicion de la nave en la orbita
    float Angulo = 0;
    float Radio = 150;

    // Guardar las balas que dispara la nave
    ListaBalas Balas;
    ListaEnemigos Enemigos;
    int TiempoDisparo = 0;
    int ColorSeleccionado = 0;
    int EnemigosEliminados = 0;
    int Combo = 0;

    // Los enemigos se guardan en la lista doblemente enlazada
    srand((unsigned int)time(NULL));

    // Crear el primer dron
    Enemigo DronNuevo{};

    DronNuevo.id = 1;
    DronNuevo.tipo = TipoEnemigo::Dron;
    DronNuevo.r = 300;
    DronNuevo.theta = 0;
    DronNuevo.omega = 0;
    DronNuevo.color = 0;
    DronNuevo.vida = 1;
    DronNuevo.radio = 11;
    DronNuevo.ticksColor = 0;

    EnemigoPtr Nuevo = &DronNuevo;
    Enemigos.InsertarFinal(Nuevo);

    // Crear el primer tanque
    Enemigo TanqueNuevo{};

    TanqueNuevo.id = 2;
    TanqueNuevo.tipo = TipoEnemigo::Tanque;
    TanqueNuevo.r = 300;
    TanqueNuevo.theta = 3.14;
    TanqueNuevo.omega = 0;
    TanqueNuevo.color = 1;
    TanqueNuevo.vida = 3;
    TanqueNuevo.radio = 16;
    TanqueNuevo.ticksColor = 0;

    Nuevo = &TanqueNuevo;
    Enemigos.InsertarFinal(Nuevo);


    al_start_timer(Timer);

    // Ciclo principal
    while (!Salir)
    {
        ALLEGRO_EVENT Evento;
        al_wait_for_event(Cola, &Evento);

        if (!CarpetaCapturas.empty() && Evento.type == ALLEGRO_EVENT_TIMER &&
            PasoCaptura >= 1 && PasoCaptura <= 3) {
            Evento = ALLEGRO_EVENT{};
            Evento.type = ALLEGRO_EVENT_MOUSE_BUTTON_DOWN;
            Evento.mouse.button = 1;
            Evento.mouse.x = 400;
            Evento.mouse.y = PasoCaptura == 1 ? 250 : (PasoCaptura == 2 ? 310 : 190);
            PasoCaptura++;
        }

        if (Evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && Evento.mouse.button == 1) {
            const float x = Evento.mouse.x;
            const float y = Evento.mouse.y;
            if (EnMenu && x >= 280 && x <= 520) {
                if (y >= 180 && y <= 220) EnMenu = false;
                else if ((y >= 240 && y <= 280) || (y >= 300 && y <= 340)) {
                    const bool EsRanking = y <= 280;
                    const std::string Captura = CarpetaCapturas.empty() ? "" : CarpetaCapturas +
                        (EsRanking ? "/ranking.png" : "/estadisticas.png");
                    Salir = !Reportes.MostrarReporte(Ventana, Cola, HistorialJugadores, TablaRanking,
                        EsRanking ? Pantalla::Ranking : Pantalla::Estadisticas,
                        HistorialCargado ? "" : "No se pudo cargar el historial", Captura);
                } else if (y >= 360 && y <= 400) Salir = true;
            } else if (!EnMenu && x >= 20 && x <= 140 && y >= 95 && y <= 131) {
                EnMenu = true;
            }
            Izquierda = false;
            Derecha = false;
        }

        // Cerrar la ventana
        if (Evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            Salir = true;

        // Revisar las teclas que se presionan
        if (Evento.type == ALLEGRO_EVENT_KEY_DOWN)
        {
            if (Evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
            {
                if (EnMenu) Salir = true;
                else EnMenu = true;
                Izquierda = false;
                Derecha = false;
            }

            if (!EnMenu && Evento.keyboard.keycode == ALLEGRO_KEY_LEFT)
                Izquierda = true;

            if (!EnMenu && Evento.keyboard.keycode == ALLEGRO_KEY_RIGHT)
                Derecha = true;

            // Cambiar el color de las proximas balas
            if (!EnMenu && (Evento.keyboard.keycode == ALLEGRO_KEY_1 || Evento.keyboard.keycode == ALLEGRO_KEY_J))
                ColorSeleccionado = 0;
            if (!EnMenu && (Evento.keyboard.keycode == ALLEGRO_KEY_2 || Evento.keyboard.keycode == ALLEGRO_KEY_K))
                ColorSeleccionado = 1;
            if (!EnMenu && (Evento.keyboard.keycode == ALLEGRO_KEY_3 || Evento.keyboard.keycode == ALLEGRO_KEY_L))
                ColorSeleccionado = 2;
        }

        // Cuando se dejan de presionar las flechas
        if (Evento.type == ALLEGRO_EVENT_KEY_UP)
        {
            if (Evento.keyboard.keycode == ALLEGRO_KEY_LEFT)
                Izquierda = false;

            if (Evento.keyboard.keycode == ALLEGRO_KEY_RIGHT)
                Derecha = false;
        }

        // Actualizar lo que se ve en la ventana
        if (Evento.type == ALLEGRO_EVENT_TIMER)
        {
            if (EnMenu) {
                al_clear_to_color(Fondo);
                al_draw_text(Fuente, Crema, 400, 55, ALLEGRO_ALIGN_CENTRE, "FASE ORBITAL");
                al_draw_circle(400, 125, 25, Morado, 2);
                al_draw_filled_circle(400, 125, 10, Coral);
                const char* Botones[] = {"Jugar", "Ranking", "Estadisticas", "Salir"};
                for (int indice = 0; indice < 4; indice++) {
                    const int y = 180 + indice * 60;
                    al_draw_filled_rectangle(280, y, 520, y + 40, Fondo);
                    al_draw_rectangle(280, y, 520, y + 40, Coral, 1);
                    al_draw_text(Fuente, Crema, 400, y + (40 - al_get_font_line_height(Fuente)) / 2,
                                 ALLEGRO_ALIGN_CENTRE, Botones[indice]);
                }
                if (!HistorialCargado) {
                    al_draw_text(Fuente, Coral, 400, 460, ALLEGRO_ALIGN_CENTRE,
                                 "No se pudo cargar el historial");
                }
                if (!CarpetaCapturas.empty() && PasoCaptura == 0) {
                    if (!al_save_bitmap((CarpetaCapturas + "/principal.png").c_str(), al_get_backbuffer(Ventana))) {
                        std::cerr << "No se pudo guardar la captura del menu.\n";
                        Salir = true;
                    }
                    PasoCaptura = 1;
                }
                al_flip_display();
                continue;
            }

            // Mover la nave
            if (Izquierda)
                Angulo -= 0.04f;

            if (Derecha)
                Angulo += 0.04f;

            // Disparar una bala cada 4 vueltas del timer
            TiempoDisparo++;
            if (TiempoDisparo >= 4)
            {
                TiempoDisparo = 0;

                // La bala sale un poco adelante de la nave
                Bala NuevaBala;
                NuevaBala.x = 400 + cos(Angulo) * (Radio + 15);
                NuevaBala.y = 310 + sin(Angulo) * (Radio + 15);

                // Las balas van hacia afuera, donde vienen los enemigos
                NuevaBala.vx = cos(Angulo) * 9;
                NuevaBala.vy = sin(Angulo) * 9;
                NuevaBala.color = ColorSeleccionado;
                NuevaBala.duenio = 1;

                // La lista guarda su propia copia de la bala
                BalaPtr Nueva = &NuevaBala;
                Balas.InsertarInicio(Nueva);
            }

            // Mover los enemigos hacia el nucleo antes de revisar las balas
            NodoEnemigoPtr EnemigoMovimiento = Enemigos.primero();
            while (EnemigoMovimiento != nullptr)
            {
                EnemigoMovimiento->dato->r -= 0.25;
                EnemigoMovimiento = EnemigoMovimiento->siguiente;
            }

            // Mover las balas y revisar sus impactos
            NodoBalaPtr Actual = Balas.primero();
            while (Actual != nullptr)
            {
                BalaPtr BalaActual = Actual->dato;
                BalaActual->x += BalaActual->vx;
                BalaActual->y += BalaActual->vy;
                bool EliminarBala = false;

                // Buscar el primer enemigo que toca la bala
                NodoEnemigoPtr EnemigoColision = Enemigos.primero();
                while (EnemigoColision != nullptr)
                {
                    EnemigoPtr EnemigoActual = EnemigoColision->dato;
                    double xEnemigo = 400 + cos(EnemigoActual->theta) * EnemigoActual->r;
                    double yEnemigo = 310 + sin(EnemigoActual->theta) * EnemigoActual->r;
                    double dx = BalaActual->x - xEnemigo;
                    double dy = BalaActual->y - yEnemigo;
                    double RadioChoque = 5 + EnemigoActual->radio;

                    if (dx * dx + dy * dy <= RadioChoque * RadioChoque)
                    {
                        EliminarBala = true;
                        if (BalaActual->color == EnemigoActual->color)
                        {
                            // El color correcto quita una vida
                            EnemigoActual->vida--;
                            if (EnemigoActual->vida <= 0)
                            {
                                EnemigosEliminados++;
                                Combo++;
                                // Eliminar tambien libera la memoria del enemigo
                                Enemigos.Eliminar(EnemigoColision);
                            }
                        }
                        else
                        {
                            // El color incorrecto empuja al enemigo
                            EnemigoActual->r += 9;
                            Combo = 0;
                        }
                        break;
                    }
                    EnemigoColision = EnemigoColision->siguiente;
                }

                // Las balas se eliminan al alejarse 320 unidades del centro
                double dxCentro = BalaActual->x - 400;
                double dyCentro = BalaActual->y - 310;
                if (dxCentro * dxCentro + dyCentro * dyCentro > 320 * 320)
                    EliminarBala = true;

                if (EliminarBala)
                    Actual = Balas.Eliminar(Actual);
                else
                    Actual = Actual->siguiente;
            }

            // Pintar el fondo
            al_clear_to_color(Fondo);

            // Nombre del juego
            al_draw_text(Fuente, Crema, 400, 35,
                         ALLEGRO_ALIGN_CENTRE, "FASE ORBITAL");

            // Dibujar la orbita
            al_draw_circle(400, 310, Radio, Morado, 3);

            // El nucleo cambia al color que seleccionamos
            al_draw_filled_circle(400, 310, 45, Colores[ColorSeleccionado]);
            al_draw_filled_circle(400, 310, 20, Crema);

            // Dibujar los enemigos de la lista
            NodoEnemigoPtr NodoActual = Enemigos.primero();

            while (NodoActual != nullptr)
            {
                EnemigoPtr EnemigoActual = NodoActual->dato;

                // Calcular la posicion del enemigo
                float x = 400 + cos(EnemigoActual->theta) * EnemigoActual->r;
                float y = 310 + sin(EnemigoActual->theta) * EnemigoActual->r;

                if (EnemigoActual->tipo == TipoEnemigo::Dron)
                {
                    // Dibujar el dron como un circulo
                    al_draw_filled_circle(
                        x, y,
                        EnemigoActual->radio,
                        Colores[EnemigoActual->color]
                    );

                    al_draw_circle(
                        x, y,
                        EnemigoActual->radio,
                        Crema, 2
                    );
                }
                else if (EnemigoActual->tipo == TipoEnemigo::Tanque)
                {
                    // Dibujar el tanque como un cuadrado
                    float RadioTanque = EnemigoActual->radio;

                    al_draw_filled_rectangle(
                        x - RadioTanque,
                        y - RadioTanque,
                        x + RadioTanque,
                        y + RadioTanque,
                        Colores[EnemigoActual->color]
                    );

                    al_draw_rectangle(
                        x - RadioTanque,
                        y - RadioTanque,
                        x + RadioTanque,
                        y + RadioTanque,
                        Crema, 2
                    );

                    // Mostrar la vida del tanque
                    std::string TextoVida = std::to_string(EnemigoActual->vida);

                    al_draw_text(
                        Fuente,
                        Crema,
                        x,
                        y - RadioTanque - 25,
                        ALLEGRO_ALIGN_CENTRE,
                        TextoVida.c_str()
                    );
                }

                NodoActual = NodoActual->siguiente;
            }


            // Posicion de la nave
            float x = 400 + cos(Angulo) * Radio;
            float y = 310 + sin(Angulo) * Radio;

            // Dibujar la nave
            al_draw_filled_circle(x, y, 12, Coral);
            al_draw_circle(x, y, 12, Crema, 2);

            // Dibujar las balas que siguen en pantalla
            NodoBalaPtr BalaDibujo = Balas.primero();
            while (BalaDibujo != nullptr)
            {
                al_draw_filled_circle(
                    BalaDibujo->dato->x,
                    BalaDibujo->dato->y,
                    5,
                    Colores[BalaDibujo->dato->color]
                );
                BalaDibujo = BalaDibujo->siguiente;
            }

            // Mostrar cuantos enemigos se han eliminado
            std::string TextoPuntos = "Enemigos: " + std::to_string(EnemigosEliminados);
            al_draw_text(Fuente, Crema, 780, 95, ALLEGRO_ALIGN_RIGHT, TextoPuntos.c_str());
            std::string TextoCombo = "Combo: " + std::to_string(Combo);
            al_draw_text(Fuente, Crema, 780, 125, ALLEGRO_ALIGN_RIGHT, TextoCombo.c_str());

            // Mostrar el color que esta usando la nave
            al_draw_filled_circle(750, 175, 9, Colores[ColorSeleccionado]);

            // Instrucciones de los controles
            al_draw_text(Fuente, Crema, 400, 520,
                         ALLEGRO_ALIGN_CENTRE, "Flechas: mover nave");

            al_draw_text(Fuente, Morado, 400, 550,
                         ALLEGRO_ALIGN_CENTRE, "ESC: menu");

            al_draw_filled_rectangle(20, 95, 140, 131, Fondo);
            al_draw_rectangle(20, 95, 140, 131, Coral, 1);
            al_draw_text(Fuente, Crema, 80, 95 + (36 - al_get_font_line_height(Fuente)) / 2,
                         ALLEGRO_ALIGN_CENTRE, "Volver");
            if (!HistorialCargado) {
                al_draw_text(Fuente, Coral, 400, 575, ALLEGRO_ALIGN_CENTRE,
                             "No se pudo cargar el historial");
            }
            if (!CarpetaCapturas.empty() && PasoCaptura == 4) {
                if (!al_save_bitmap((CarpetaCapturas + "/jugar.png").c_str(), al_get_backbuffer(Ventana))) {
                    std::cerr << "No se pudo guardar la captura del prototipo.\n";
                }
                Salir = true;
            }

            // Mostrar todo en pantalla
            al_flip_display();
        }
    }

    // Cerrar lo que se utilizo
    al_destroy_font(Fuente);
    al_destroy_timer(Timer);
    al_destroy_event_queue(Cola);
    al_destroy_display(Ventana);

    return 0;
}

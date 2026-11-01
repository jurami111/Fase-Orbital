
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <cmath>
#include <cstdlib>
#include <ctime>

int main()
{
    // Iniciar allegro y lo que vamos a usar
    al_init();
    al_install_keyboard();
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_ttf_addon();

    // Crear la ventana del juego
    ALLEGRO_DISPLAY* Ventana = al_create_display(800, 600);
    ALLEGRO_EVENT_QUEUE* Cola = al_create_event_queue();
    ALLEGRO_TIMER* Timer = al_create_timer(1.0 / 60);

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

    // Colores de la interfaz
    ALLEGRO_COLOR Fondo = al_map_rgb(82, 102, 79);
    ALLEGRO_COLOR Crema = al_map_rgb(242, 229, 201);
    ALLEGRO_COLOR Morado = al_map_rgb(181, 154, 197);
    ALLEGRO_COLOR Coral = al_map_rgb(228, 154, 134);

    // Colores que pueden tener los enemigos
    ALLEGRO_COLOR Colores[3] = {
        al_map_rgb(228, 154, 134),
        al_map_rgb(235, 200, 115),
        al_map_rgb(181, 154, 197)
    };

    // Para saber si se presionan las teclas
    bool Salir = false;
    bool Izquierda = false;
    bool Derecha = false;

    // Posicion de la nave en la orbita
    float Angulo = 0;
    float Radio = 150;

    // Datos de los enemigos
    const int Cantidad = 5;
    float AnguloEnemigo[Cantidad];
    float DistanciaEnemigo[Cantidad];
    int ColorEnemigo[Cantidad];

    srand((unsigned int)time(NULL));

    // Poner a los enemigos en posiciones diferentes
    for (int i = 0; i < Cantidad; i++)
    {
        AnguloEnemigo[i] = (rand() % 628) / 100.0f;
        DistanciaEnemigo[i] = 230 + rand() % 120;
        ColorEnemigo[i] = rand() % 3;
    }

    al_start_timer(Timer);

    // Ciclo principal
    while (!Salir)
    {
        ALLEGRO_EVENT Evento;
        al_wait_for_event(Cola, &Evento);

        // Cerrar la ventana
        if (Evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            Salir = true;

        // Revisar las teclas que se presionan
        if (Evento.type == ALLEGRO_EVENT_KEY_DOWN)
        {
            if (Evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                Salir = true;

            if (Evento.keyboard.keycode == ALLEGRO_KEY_LEFT)
                Izquierda = true;

            if (Evento.keyboard.keycode == ALLEGRO_KEY_RIGHT)
                Derecha = true;
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
            // Mover la nave
            if (Izquierda)
                Angulo -= 0.04f;

            if (Derecha)
                Angulo += 0.04f;

            // Pintar el fondo
            al_clear_to_color(Fondo);

            // Nombre del juego
            al_draw_text(Fuente, Crema, 400, 35,
                         ALLEGRO_ALIGN_CENTRE, "FASE ORBITAL");

            al_draw_text(Fuente, Morado, 400, 75,
                         ALLEGRO_ALIGN_CENTRE, "Version inicial");

            // Dibujar la orbita
            al_draw_circle(400, 310, Radio, Morado, 3);

            // Dibujar el nucleo del centro
            al_draw_filled_circle(400, 310, 45, Morado);
            al_draw_filled_circle(400, 310, 20, Crema);

            // Dibujar y mover los enemigos
            for (int i = 0; i < Cantidad; i++)
            {
                // Cada enemigo se va acercando al centro
                DistanciaEnemigo[i] -= 0.25f;

                // Si llega al centro vuelve a aparecer afuera
                if (DistanciaEnemigo[i] < 50)
                {
                    AnguloEnemigo[i] = (rand() % 628) / 100.0f;
                    DistanciaEnemigo[i] = 250 + rand() % 100;
                    ColorEnemigo[i] = rand() % 3;
                }

                // Calcular donde se dibuja el enemigo
                float x = 400 + cos(AnguloEnemigo[i]) * DistanciaEnemigo[i];
                float y = 310 + sin(AnguloEnemigo[i]) * DistanciaEnemigo[i];

                al_draw_filled_circle(x, y, 11, Colores[ColorEnemigo[i]]);
                al_draw_circle(x, y, 11, Crema, 2);
            }

            // Posicion de la nave
            float x = 400 + cos(Angulo) * Radio;
            float y = 310 + sin(Angulo) * Radio;

            // Dibujar la nave
            al_draw_filled_circle(x, y, 12, Coral);
            al_draw_circle(x, y, 12, Crema, 2);

            // Instrucciones de los controles
            al_draw_text(Fuente, Crema, 400, 520,
                         ALLEGRO_ALIGN_CENTRE, "Flechas: mover nave");

            al_draw_text(Fuente, Morado, 400, 550,
                         ALLEGRO_ALIGN_CENTRE, "ESC: salir");

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

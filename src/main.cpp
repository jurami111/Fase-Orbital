#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>

#include <cmath>       // para calcular las posiciones con seno y coseno
#include <iostream>    // Para mostrar mensajes en la consola
#include <string>      // para trabajar con textos

#include "Estadisticas/Persistencia.h"
#include "Vista/VistaAllegro.h"
#include "Estructuras/ListaBalas.h"
#include "Estructuras/ListaEnemigos.h"
#include "Motor/Oleada.h"
#include "Estructuras/ColaSpawn.h"
#include "Estructuras/RondaJugadores.h"
#include "Estadisticas/BitacoraEventos.h"
#include "Motor/Motor.h"
#include "Vista/Controlador.h"

namespace {
Accion TraducirTecla(int tecla)
{
    switch (tecla)
    {
        case ALLEGRO_KEY_ENTER: return Accion::ConfirmarTurno;
        case ALLEGRO_KEY_P: return Accion::Pausa;
        case ALLEGRO_KEY_LEFT: return Accion::Izquierda;
        case ALLEGRO_KEY_RIGHT: return Accion::Derecha;
        case ALLEGRO_KEY_1:
        case ALLEGRO_KEY_J: return Accion::Cian;
        case ALLEGRO_KEY_2:
        case ALLEGRO_KEY_K: return Accion::Magenta;
        case ALLEGRO_KEY_3:
        case ALLEGRO_KEY_L: return Accion::Ambar;
        case ALLEGRO_KEY_SPACE: return Accion::Pulso;
        default: return Accion::Ninguna;
    }
}
}

int main(int argc, char* argv[])
{
    // Archivo donde se guardan los datos de las partidas
    std::string ArchivoHistorial = "historial.csv";
    std::string CarpetaCapturas;

    // Se revisa si se enviaron opciones al ejecutar el programa
    for (int indice = 1; indice < argc; indice++)
    {
        const std::string argumento = argv[indice];

        if (argumento == "--historial" && indice + 1 < argc)
            ArchivoHistorial = argv[++indice];

        else if (argumento == "--capturas" && indice + 1 < argc)
            CarpetaCapturas = argv[++indice];

        else
        {
            std::cerr << "Uso: faseorbital [--historial archivo.csv] [--capturas carpeta]\n";
            return 1;
        }
    }

    // se cargan los datos de partidas anteriores
    Historial HistorialJugadores;
    Persistencia Archivos;

    const bool HistorialCargado =
        Archivos.cargarHistorial(ArchivoHistorial, &HistorialJugadores);

    Ranking TablaRanking;
    TablaRanking.ConstruirDesdeHistorial(&HistorialJugadores);

    VistaAllegro Reportes; // para mostrar el ranking y las estadisticas
    int PasoCaptura = 0; // controla las capturas de pantalla
    bool CapturaDuplicadoRechazado = false;

    // Se inicia Allegro y las herramientas que necesita el juego
    al_init();
    al_install_keyboard();

    if (!al_install_mouse())
    {
        std::cerr << "No se pudo instalar el mouse.\n";
        return 1;
    }

    al_init_primitives_addon(); // Para dibujar circulos y otras figuras
    al_init_font_addon();       // Para mostrar textos
    al_init_ttf_addon();        // para cargar fuentes de Windows

    if (!CarpetaCapturas.empty() && !al_init_image_addon())
    {
        std::cerr << "No se pudo inicializar el addon de imagen.\n";
        return 1;
    }

    // Se crea la ventana, los eventos y el temporizador
    ALLEGRO_DISPLAY* Ventana = al_create_display(800, 600);
    ALLEGRO_EVENT_QUEUE* Cola = al_create_event_queue();
    ALLEGRO_TIMER* Timer = al_create_timer(1.0 / 30);

    // Se utiliza la fuente Consolas para los textos
    ALLEGRO_FONT* Fuente =
        al_load_ttf_font("C:/Windows/Fonts/consola.ttf", 22, 0);

    if (Fuente == NULL)
        Fuente = al_create_builtin_font(); // Usamos otra si no se encuentra

    // se comprueba que los recursos se hayan creado
    if (Ventana == NULL || Cola == NULL || Timer == NULL || Fuente == NULL)
        return 1;

    al_set_window_title(Ventana, "Fase Orbital");

    // se registran los eventos que vamos a utilizar
    al_register_event_source(Cola, al_get_display_event_source(Ventana));
    al_register_event_source(Cola, al_get_keyboard_event_source());
    al_register_event_source(Cola, al_get_timer_event_source(Timer));
    al_register_event_source(Cola, al_get_mouse_event_source());

    // Colores principales de la pantalla
    ALLEGRO_COLOR Fondo = al_map_rgb(20, 17, 43);
    ALLEGRO_COLOR Crema = al_map_rgb(235, 242, 246);
    ALLEGRO_COLOR Morado = al_map_rgb(58, 52, 90);
    ALLEGRO_COLOR Coral = al_map_rgb(110, 217, 241);

    // Los tres colores que se utiliza para las balas y enemigos
    ALLEGRO_COLOR Colores[3] = {
        al_map_rgb(68, 210, 225),  // cian
        al_map_rgb(238, 91, 176),  // magenta
        al_map_rgb(245, 183, 73)   // Ambar
    };

    // variables para controlar las pantallas y las teclas
    bool Salir = false;
    bool EnMenu = true;
    bool EnNombre = false;
    std::string NombreJugador;
    std::string AvisoRegistro;
    Motor Juego;
    bool GuardadoIntentado = false;

    // empieza cuando ya se anotaron todos
    auto IniciarMultijugador = [&]()
    {
        EnNombre = false;
        EnMenu = false;
        Juego.IniciarMultijugador();
    };

    auto ReiniciarRegistro = [&]()
    {
        Juego.ReiniciarRegistro();
        NombreJugador.clear();
        AvisoRegistro.clear();
        EnNombre = true;
        GuardadoIntentado = false;
    };

    // Se inicia el temporizador para actualizar el juego
    al_start_timer(Timer);

    
    // Ciclo principal del juego
    // Se repite mientras no cerremos la ventana
    while (!Salir)
    {
        ALLEGRO_EVENT Evento;
        al_wait_for_event(Cola, &Evento); // Se espera un evento

        // Esta parte se utiliza para las capturas automaticas
        // la se deja para que sigan funcionando las pruebas
        if (!CarpetaCapturas.empty() && Evento.type == ALLEGRO_EVENT_TIMER &&
            PasoCaptura >= 1 && PasoCaptura <= 16)
        {
            Evento = ALLEGRO_EVENT{};
            if (PasoCaptura <= 3)
            {
                Evento.type = ALLEGRO_EVENT_MOUSE_BUTTON_DOWN;
                Evento.mouse.button = 1;
                Evento.mouse.x = 400;
                Evento.mouse.y = PasoCaptura == 1 ? 250 : (PasoCaptura == 2 ? 310 : 190);
            }
            else if (PasoCaptura <= 6)
            {
                const int letras[] = {'A', 'n', 'a'};
                Evento.type = ALLEGRO_EVENT_KEY_CHAR;
                Evento.keyboard.unichar = letras[PasoCaptura - 4];
                Evento.keyboard.keycode = PasoCaptura == 5 ? ALLEGRO_KEY_N : ALLEGRO_KEY_A;
            }
            else if (PasoCaptura == 7 || PasoCaptura == 11 || PasoCaptura == 15)
            {
                Evento.type = ALLEGRO_EVENT_MOUSE_BUTTON_DOWN;
                Evento.mouse.button = 1;
                Evento.mouse.x = 400;
                Evento.mouse.y = PasoCaptura == 7 || PasoCaptura == 11 ? 420 : 490;
            }
            else if (PasoCaptura <= 10)
            {
                const int letras[] = {'a', 'N', 'A'};
                Evento.type = ALLEGRO_EVENT_KEY_CHAR;
                Evento.keyboard.unichar = letras[PasoCaptura - 8];
                Evento.keyboard.keycode = PasoCaptura == 9 ? ALLEGRO_KEY_N : ALLEGRO_KEY_A;
            }
            else if (PasoCaptura <= 14)
            {
                Evento.type = ALLEGRO_EVENT_KEY_CHAR;
                Evento.keyboard.unichar = '\b';
                Evento.keyboard.keycode = ALLEGRO_KEY_BACKSPACE;
            }
            else
            {
                Evento.type = ALLEGRO_EVENT_KEY_DOWN;
                Evento.keyboard.keycode = ALLEGRO_KEY_ENTER;
            }
            PasoCaptura++;
        }

        // Se revisa si se hizo clic con el mouse
        if (Evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && Evento.mouse.button == 1)
        {
            const float x = Evento.mouse.x; // posicion horizontal
            const float y = Evento.mouse.y; // Posicion vertical

            
            // Registro dinamico: se pueden agregar nombres y comenzar con los registrados.
            if (EnNombre && x >= 280 && x <= 520 && y >= 400 && y <= 445)
            {
                Juego.RegistrarNombre(NombreJugador, AvisoRegistro);
            }
            else if (EnNombre && x >= 280 && x <= 520 && y >= 470 && y <= 515)
            {
                if (!NombreJugador.empty() && !Juego.RegistrarNombre(NombreJugador, AvisoRegistro)) continue;
                if (Juego.Registrados > 0) IniciarMultijugador();
                else AvisoRegistro = "Agregue por lo menos un jugador.";
            }
            else if (EnNombre && x >= 280 && x <= 520 && y >= 530 && y <= 570)
            {
                EnNombre = false;
                EnMenu = true;
            }
            else
            // se revisa si estamos en victoria o derrota (no en el menu)
            if ((Juego.GameOver || Juego.Victoria) && !EnMenu)
            {
                // Boton para volver a jugar
                if (x >= 280 && x <= 520 && y >= 320 && y <= 365)
                {
                    Juego.ReiniciarDesdeVictoria();
                    EnMenu = false;
                    ReiniciarRegistro(); // Se registran los jugadores

                }

                // Boton para regresar al menu principal
                else if (x >= 280 && x <= 520 && y >= 385 && y <= 430)
                {
                    EnMenu = true; // Regresamos al menu
                    Juego.EnPausa = false; // Se quitan la pausa

                    Juego.Izquierda = false;
                    Juego.Derecha = false;
                }
            }


            // Si estamos en pausa, se revisa el boton Seguir
            else if (!EnMenu && Juego.EnPausa && x >= 335 && x <= 465 && y >= 270 && y <= 313)
            {
                Juego.EnPausa = false; // Se quitan la pausa
            }

            // si estamos en el menu, se revisa los botones
            else if (EnMenu && x >= 280 && x <= 520)
            {
                if (y >= 180 && y <= 220)
                {
                    Juego.ReiniciarDesdeMenu();

                    EnMenu = false;
                    ReiniciarRegistro(); // Se registran los jugadores
                }

                // botones de ranking y estadisticas
                else if ((y >= 240 && y <= 280) || (y >= 300 && y <= 340))
                {
                    const bool EsRanking = y <= 280;

                    // se preparan la ruta si se necesita una captura
                    const std::string Captura = CarpetaCapturas.empty() ? "" : CarpetaCapturas + (EsRanking ? "/ranking.png" : "/estadisticas.png");

                    // Abrimos la pantalla que corresponde
                    Salir = !Reportes.MostrarReporte(
                        Ventana,
                        Cola,
                        HistorialJugadores,
                        TablaRanking,
                        EsRanking ? Pantalla::Ranking : Pantalla::Estadisticas,
                        HistorialCargado ? "" : "No se pudo cargar el historial",
                        Captura
                    );
                }

                else if (y >= 360 && y <= 400)
                {
                    Salir = true; // se cierra el juego
                }
            }

            // se evita que la nave siga girando al usar el mouse
            Juego.Izquierda = false;
            Juego.Derecha = false;
            if (!CarpetaCapturas.empty() && PasoCaptura == 12 && EnNombre &&
                Juego.Registrados == 1 && NombreJugador == "aNA") {
                CapturaDuplicadoRechazado = true;
            }
        }

        // se cierra el juego si se presiona la X de la ventana
        if (Evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        {
            Salir = true;
        }

        // se revisa cuando se presiona alguna tecla
        if (Evento.type == ALLEGRO_EVENT_KEY_DOWN)
        {
            // ESC permite regresar al menu o cerrar el programa
            if (Evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
            {
                if (EnNombre)
                {
                    EnNombre = false;
                    EnMenu = true;
                }
                else if (EnMenu)
                {
                    Salir = true; // Salimos si ya estamos en el menu
                }
                else
                {
                    EnMenu = true; // Regresamos al menu
                    Juego.EnPausa = false; // Se quitan la pausa
                }

                Juego.Izquierda = false;
                Juego.Derecha = false;
            }

            // al terminar se puede regresar al menu con ESC
            // El reinicio se hace cuando se vuelve a presionar Jugar
            Controlador::Presionar(TraducirTecla(Evento.keyboard.keycode), Juego, EnMenu, EnNombre);
        }

        // se escribe el alias antes de iniciar la partida
        if (Evento.type == ALLEGRO_EVENT_KEY_CHAR && EnNombre)
        {
            int Tecla = Evento.keyboard.keycode;
            int Letra = Evento.keyboard.unichar;
            if (Tecla == ALLEGRO_KEY_BACKSPACE && !NombreJugador.empty()) NombreJugador.pop_back();
            else if (Tecla == ALLEGRO_KEY_ENTER && !NombreJugador.empty())
            {
                Juego.RegistrarNombre(NombreJugador, AvisoRegistro);
            }
            else if (Letra >= 32 && Letra <= 126 && NombreJugador.size() < sizeof(Juego.DatosPartida.Alias) - 1)
            {
                // los nombres se guardan con caracteres simples para el historial CSV
                if (Letra != ',' && Letra != '"') NombreJugador += (char)Letra;
            }
        }

        if (Evento.type == ALLEGRO_EVENT_KEY_UP)
        {
            Controlador::Soltar(TraducirTecla(Evento.keyboard.keycode), Juego);
        }

        // El temporizador actualiza el juego varias veces por segundo
        if (Evento.type == ALLEGRO_EVENT_TIMER)
        {
            // registro de jugadores
            if (EnNombre)
            {
                al_clear_to_color(Fondo);
                al_draw_text(Fuente, Colores[0], 400, 85, ALLEGRO_ALIGN_CENTRE, "REGISTRO MULTIJUGADOR");
                std::string Titulo = "Agregados: " + std::to_string(Juego.Registrados) + " de 6";
                al_draw_text(Fuente, Crema, 400, 140, ALLEGRO_ALIGN_CENTRE, Titulo.c_str());

                for (int indice = 0; indice < Juego.Registrados; indice++)
                {
                    std::string nombre = std::to_string(indice + 1) + ". " + Juego.DatosJugadores[indice].Alias;
                    al_draw_text(Fuente, Colores[indice % 3], 300, 170 + indice * 20, 0, nombre.c_str());
                }

                // nombre
                al_draw_filled_rectangle(250, 300, 550, 355, Morado);
                al_draw_rectangle(250, 300, 550, 355, Colores[0], 2);
                std::string TextoNombre = NombreJugador.empty() ? "Nombre..." : NombreJugador;
                al_draw_text(Fuente, Crema, 400, 315, ALLEGRO_ALIGN_CENTRE, TextoNombre.c_str());

                al_draw_filled_rectangle(280, 400, 520, 445, Colores[0]);
                al_draw_text(Fuente, Fondo, 400, 410, ALLEGRO_ALIGN_CENTRE, "Agregar");
                al_draw_filled_rectangle(280, 470, 520, 515, Colores[1]);
                al_draw_text(Fuente, Fondo, 400, 480, ALLEGRO_ALIGN_CENTRE, "Jugar");
                al_draw_rectangle(280, 530, 520, 570, Colores[2], 2);
                al_draw_text(Fuente, Crema, 400, 540, ALLEGRO_ALIGN_CENTRE, "Volver");
                if (!AvisoRegistro.empty())
                    al_draw_text(Fuente, Coral, 400, 370, ALLEGRO_ALIGN_CENTRE, AvisoRegistro.c_str());
                al_flip_display();
                continue;
            }

            if (!Juego.DatosPartida.isBot)
            {
                if (Juego.EnTurno)
                {
                al_clear_to_color(Fondo);
                al_draw_text(Fuente, Colores[0], 400, 170, ALLEGRO_ALIGN_CENTRE, "SIGUIENTE TURNO");
                al_draw_text(Fuente, Crema, 400, 245, ALLEGRO_ALIGN_CENTRE, Juego.DatosPartida.Alias);
                std::string Aviso = "Oleada " + std::to_string(Juego.Oleada) + " de " + std::to_string(Juego.TotalOleadas);
                al_draw_text(Fuente, Crema, 400, 295, ALLEGRO_ALIGN_CENTRE, Aviso.c_str());
                al_draw_text(Fuente, Colores[2], 400, 365, ALLEGRO_ALIGN_CENTRE, "Presione ENTER para jugar");
                al_flip_display();
                continue;
            }
            }

            // Primero se revisa si estamos en el menu
            if (EnMenu)
            {
                al_clear_to_color(Fondo); // fondo oscuro y sin rayas

                // Titulo principal
                al_draw_text( Fuente, Colores[0], 400, 55, ALLEGRO_ALIGN_CENTRE, "FASE ORBITAL" );

                // Se dibuja el nucleo que aparece en el menu
                al_draw_filled_circle(400, 125, 28, Morado);
                al_draw_filled_circle(400, 125, 17, Crema);

                // Textos de los cuatro botones
                const char* Botones[] = {
                    "Jugar", "Ranking", "Estadisticas", "Salir"
                };

                // Se dibuja cada boton en una posicion diferente
                for (int indice = 0; indice < 4; indice++)
                {
                    const int y = 180 + indice * 60;

                    // Fondo del boton
                    al_draw_filled_rectangle( 280, y, 520, y + 40, al_map_rgb(36, 31, 66) );

                    // Borde del boton
                    al_draw_rectangle( 280, y, 520, y + 40, indice % 2 == 0 ? Colores[0] : Colores[1], 1 );

                    // Nombre del boton
                    al_draw_text( Fuente, Crema, 400, y + (40 - al_get_font_line_height(Fuente)) / 2, ALLEGRO_ALIGN_CENTRE, Botones[indice] );
                }

                // Avisamos si hubo un problema al cargar los datos
                if (!HistorialCargado)
                {
                    al_draw_text( Fuente, Coral, 400, 460, ALLEGRO_ALIGN_CENTRE, "No se pudo cargar el historial" );
                }

                // Se guardan una captura del menu si se solicito
                if (!CarpetaCapturas.empty() && PasoCaptura == 0)
                {
                    if (!al_save_bitmap( (CarpetaCapturas + "/principal.png").c_str(), al_get_backbuffer(Ventana)))
                    {
                        std::cerr << "No se pudo guardar la captura del menu.\n";
                        Salir = true;
                    }

                    PasoCaptura = 1;
                }

                al_flip_display(); // se muestra el menu
                continue; // No actualizamos la partida mientras estamos aqui
            }
            if (Juego.DatosPartida.isBot){
                AccionesTick acciones = Controlador::AccionesBot(Juego);
                Juego.AvanzarTick(acciones);
            } else{
                Juego.AvanzarTick();
            }
              // los dibujos quedan afuera para mostrar Game Over y Victoria

            // Se guardan la partida una sola vez al terminar
            if ((Juego.GameOver || Juego.Victoria) && !GuardadoIntentado)
            {
                GuardadoIntentado = true;
                Juego.CalcularPosiciones();
                JugadorPtr Jugadores[6];
                for (int i = 0; i < Juego.Registrados; i++) Jugadores[i] = &Juego.DatosJugadores[i];
                const bool guardado = Archivos.guardarPartidaCompleta(
                    ArchivoHistorial, Jugadores, Juego.Registrados, Juego.Semilla, "Turnos", &HistorialJugadores);
                if (guardado)
                {
                    TablaRanking.ConstruirDesdeHistorial(&HistorialJugadores);
                    for (int indice = 0; indice < Juego.Registrados; indice++)
                        Estadisticas().imprimirLineaTiempo(&Juego.Bitacora, &Juego.DatosJugadores[indice]);
                }
                else std::cerr << "No se pudo guardar la partida en el historial.\n";
            }

            // limpiamos la pantalla antes de volver a dibujar todo
            al_clear_to_color(Fondo);

            // Se dibuja solamente la orbita por donde se mueve la nave
            // No agregamos las otras lineas ni circulos decorativos
            al_draw_circle( 400, 310, Juego.Radio, Morado, 2 );

            // se dibuja el nucleo en el centro de la pantalla
            al_draw_filled_circle( 400, 310, 20, Morado );

            // este circulo muestra el color seleccionado
            al_draw_filled_circle( 400, 310, 15, Colores[Juego.ColorSeleccionado] );

            // Parte interna del nucleo
            al_draw_filled_circle( 400, 310, 10, Crema );

            // circulo del pulso mientras se expande
            if (Juego.PulsoActivo) al_draw_circle(400, 310, Juego.RadioPulso, Colores[0], 4);

            // Se recorre los enemigos para dibujarlos
            NodoEnemigoPtr EnemigoDibujo = Juego.Enemigos.primero();

            while (EnemigoDibujo != nullptr)
            {
                // Se toma los datos del enemigo actual
                Enemigo* Actual = EnemigoDibujo->dato;

                // se calcula su posicion alrededor del nucleo
                float x = 400 + cos(Actual->theta) * Actual->r;
                float y = 310 - sin(Actual->theta) * Actual->r;

                // Cada enemigo tiene una forma sencilla
                if (Actual->tipo == TipoEnemigo::Tanque)
                {
                    al_draw_filled_rectangle(x - Actual->radio, y - Actual->radio,
                                             x + Actual->radio, y + Actual->radio, Colores[Actual->color]);
                    al_draw_filled_rectangle(x - Actual->radio / 2, y - Actual->radio / 2,
                                             x + Actual->radio / 2, y + Actual->radio / 2, Fondo);
                }
                else if (Actual->tipo == TipoEnemigo::Girador)
                {
                    al_draw_filled_triangle(x, y - Actual->radio, x - Actual->radio, y + Actual->radio,
                                            x + Actual->radio, y + Actual->radio, Colores[Actual->color]);
                    al_draw_filled_circle(x, y, 2, Fondo);
                }
                else if (Actual->tipo == TipoEnemigo::Divisor)
                {
                    al_draw_filled_rectangle(x - Actual->radio, y - Actual->radio,
                                             x + Actual->radio, y + Actual->radio, Colores[Actual->color]);
                    al_draw_line(x - Actual->radio + 1, y - Actual->radio + 1,
                                 x + Actual->radio - 1, y + Actual->radio - 1, Fondo, 2);
                }
                else if (Actual->tipo == TipoEnemigo::Mini)
                {
                    al_draw_filled_triangle(x, y - Actual->radio, x - Actual->radio, y + Actual->radio,
                                            x + Actual->radio, y + Actual->radio, Colores[Actual->color]);
                }
                else if (Actual->tipo == TipoEnemigo::Camaleon)
                {
                    al_draw_filled_ellipse(x, y, Actual->radio, Actual->radio, Colores[Actual->color]);
                    al_draw_filled_circle(x, y, 2, Fondo);
                }
                else
                {
                    // el dron es un rombo relleno
                    al_draw_filled_triangle(x, y - Actual->radio, x + Actual->radio, y,
                                            x - Actual->radio, y, Colores[Actual->color]);
                    al_draw_filled_triangle(x, y + Actual->radio, x + Actual->radio, y,
                                            x - Actual->radio, y, Colores[Actual->color]);
                }

                // se pasa al siguiente enemigo
                EnemigoDibujo = EnemigoDibujo->siguiente;
            }

            // Se recorre la lista para dibujar las balas
            NodoBalaPtr BalaDibujo = Juego.Balas.primero();

            while (BalaDibujo != nullptr)
            {
                Bala* Actual = BalaDibujo->dato; // Bala que vamos a dibujar

                // cada bala utiliza el color con el que fue disparada
                al_draw_filled_circle( Actual->x, Actual->y, 5, Colores[Actual->color] );

                BalaDibujo = BalaDibujo->siguiente; // Siguiente bala
            }

            // Se calcula donde se encuentra la nave
            float NaveX = 400 + cos(Juego.Angulo) * Juego.Radio;
            float NaveY = 310 - sin(Juego.Angulo) * Juego.Radio;

            // se calcula la punta de la nave
            float PuntaX = NaveX + cos(Juego.Angulo) * 18;
            float PuntaY = NaveY - sin(Juego.Angulo) * 18;

            // se calcula los otros dos puntos del triangulo
            float Lado1X = NaveX + cos(Juego.Angulo + 2.5f) * 13;
            float Lado1Y = NaveY - sin(Juego.Angulo + 2.5f) * 13;

            float Lado2X = NaveX + cos(Juego.Angulo - 2.5f) * 13;
            float Lado2Y = NaveY - sin(Juego.Angulo - 2.5f) * 13;

            // Se dibuja la nave como un triangulo
            al_draw_filled_triangle( PuntaX, PuntaY, Lado1X, Lado1Y, Lado2X, Lado2Y, Colores[Juego.ColorSeleccionado] );

            // se muestra el puntaje en la esquina superior izquierda
            // cada enemigo eliminado vale 100 puntos
            std::string NombreEnTurno = std::string("Turno: ") + Juego.DatosPartida.Alias;
            al_draw_text(Fuente, Colores[0], 400, 20, ALLEGRO_ALIGN_CENTRE, NombreEnTurno.c_str());
            std::string TextoPuntaje = "Puntos: " + std::to_string(Juego.Puntaje);

            al_draw_text( Fuente, Crema, 20, 20, 0, TextoPuntaje.c_str() );

            // Dibuja la barra de vida del nucleo
            float AnchoBarra = 150;
            float AltoBarra = 10;
            float x = 20;
            float y = 80;

            // Se muestra el texto arriba de la barra
            al_draw_text(Fuente, Crema, x, 50, 0, "Vida");

            // se calcula cuanto debe llenarse la barra
            float VidaActual = AnchoBarra * Juego.VidaNucleo / 100.0f;

            // Se dibuja el fondo gris
            al_draw_filled_rectangle( x, y, x + AnchoBarra, y + AltoBarra, al_map_rgb(80, 80, 80) );

            // se dibuja la vida en verde
            al_draw_filled_rectangle( x, y, x + VidaActual, y + AltoBarra, al_map_rgb(60, 180, 80) );

            // Se dibuja un borde sencillo
            al_draw_rectangle( x, y, x + AnchoBarra, y + AltoBarra, Crema, 1 );

            // muestra la oleada actual y el combo
            std::string TextoOleada = "Oleada " + std::to_string(Juego.Oleada) + "  Combo x" + std::to_string(Juego.Combo);

            al_draw_text( Fuente, Crema, 780, 20, ALLEGRO_ALIGN_RIGHT, TextoOleada.c_str() );

            // Textos que muestran los colores disponibles
            al_draw_text( Fuente, Colores[0], 140, 545, ALLEGRO_ALIGN_CENTRE, "1 Cian" );

            al_draw_text( Fuente, Colores[1], 310, 545, ALLEGRO_ALIGN_CENTRE, "2 Magenta" );

            al_draw_text( Fuente, Colores[2], 510, 545, ALLEGRO_ALIGN_CENTRE, "3 Ambar" );

            // barra para ver la carga del pulso
            al_draw_text(Fuente, Crema, 680, 530, ALLEGRO_ALIGN_CENTRE, Juego.CargaPulso == 100 ? "ESPACIO Pulso" : "Pulso");
            al_draw_filled_rectangle(615, 566, 745, 578, Morado);
            al_draw_filled_rectangle(615, 566, 615 + Juego.CargaPulso * 1.3f, 578, Colores[0]);
            al_draw_rectangle(615, 566, 745, 578, Crema, 1);

            // cuando termina una oleada se muestra un aviso
            if (Juego.OleadaTerminada && !Juego.EnPausa)
            {
                // Fondo del aviso para que se pueda leer
                al_draw_filled_rectangle( 210, 220, 590, 315, al_map_rgb(33, 29, 62) );

                // Borde del aviso
                al_draw_rectangle( 210, 220, 590, 315, Colores[0], 2 );

                // texto principal
                al_draw_text( Fuente, Crema, 400, 235, ALLEGRO_ALIGN_CENTRE, "Oleada completada" );

                // Texto que indica que pronto empieza otra oleada
                al_draw_text( Fuente, Colores[0], 400, 275, ALLEGRO_ALIGN_CENTRE, "Preparando la siguiente..." );
            }

            // Si el juego esta en pausa, se muestra una ventana encima
            if (Juego.EnPausa)
            {
                // fondo de la ventana de pausa
                al_draw_filled_rectangle( 280, 200, 520, 340, al_map_rgb(33, 29, 62) );

                // borde de la ventana
                al_draw_rectangle( 280, 200, 520, 340, Colores[1], 2 );

                // Titulo de la pausa
                al_draw_text( Fuente, Crema, 400, 225, ALLEGRO_ALIGN_CENTRE, "Pausa" );

                // boton para seguir jugando
                al_draw_filled_rectangle( 335, 270, 465, 313, Morado );

                al_draw_rectangle( 335, 270, 465, 313, Colores[0], 1 );

                al_draw_text( Fuente, Crema, 400, 278, ALLEGRO_ALIGN_CENTRE, "Seguir" );
            }

            // Se muestra la pantalla de derrota cuando un enemigo llega al nucleo
            if (Juego.GameOver)
            {
                // Se cubre el juego con un fondo oscuro
                al_draw_filled_rectangle( 0, 0, 800, 600, al_map_rgb(20, 17, 43) );

                // Titulo de derrota
                al_draw_text( Fuente, Colores[1], 400, 190, ALLEGRO_ALIGN_CENTRE, "GAME OVER" );

                // se muestra cuantos puntos conseguimos
                std::string PuntosFinales = "Puntos: " + std::to_string(Juego.Puntaje);

                al_draw_text( Fuente, Crema, 400, 260, ALLEGRO_ALIGN_CENTRE, PuntosFinales.c_str() );

                
                // Boton para intentar otra vez
                al_draw_filled_rectangle( 280, 320, 520, 365, Colores[1] );

                // Texto del boton
                al_draw_text( Fuente, Fondo, 400, 330, ALLEGRO_ALIGN_CENTRE, "Reintentar" );

                // Boton para regresar al menu
                al_draw_filled_rectangle( 280, 385, 520, 430, Morado );

                // Se dibuja el borde del boton
                al_draw_rectangle( 280, 385, 520, 430, Colores[1], 2 );

                // Texto del segundo boton
                al_draw_text( Fuente, Crema, 400, 395, ALLEGRO_ALIGN_CENTRE, "Menu principal" );

            }

            // Se muestra la pantalla de victoria al superar todas las oleadas
            if (Juego.Victoria)
            {
                // Fondo para que solo se vea el resultado
                al_draw_filled_rectangle( 0, 0, 800, 600, al_map_rgb(20, 17, 43) );

                // Titulo de victoria
                al_draw_text( Fuente, Colores[0], 400, 190, ALLEGRO_ALIGN_CENTRE, "VICTORIA" );

                al_draw_text( Fuente, Crema, 400, 250, ALLEGRO_ALIGN_CENTRE, "Completaste las 5 oleadas" );

                // Puntaje final
                std::string PuntosFinales = "Puntos: " + std::to_string(Juego.Puntaje);

                al_draw_text( Fuente, Colores[2], 400, 300, ALLEGRO_ALIGN_CENTRE, PuntosFinales.c_str() );

                
                // boton para comenzar otra partida
                al_draw_filled_rectangle( 280, 320, 520, 365, Colores[0] );

                // texto del boton
                al_draw_text( Fuente, Fondo, 400, 330, ALLEGRO_ALIGN_CENTRE, "Reintentar" );

                // Boton para regresar al menu
                al_draw_filled_rectangle( 280, 385, 520, 430, Morado );

                // borde del boton
                al_draw_rectangle( 280, 385, 520, 430, Colores[0], 2 );

                // texto del segundo boton
                al_draw_text( Fuente, Crema, 400, 395, ALLEGRO_ALIGN_CENTRE, "Menu principal" );

            }

            // se muestra en pantalla todo lo que se dibuja
            if (!CarpetaCapturas.empty() && PasoCaptura == 17)
            {
                if (!al_save_bitmap((CarpetaCapturas + "/jugar.png").c_str(), al_get_backbuffer(Ventana)))
                    std::cerr << "No se pudo guardar la captura de la partida.\n";
                if (!CapturaDuplicadoRechazado)
                    std::cerr << "No se valido el rechazo de un alias duplicado.\n";
                Salir = true;
            }
            al_flip_display();

        } // Termina el evento del temporizador

    } // termina el ciclo principal del juego

    // Se liberan los recursos antes de cerrar el programa
    al_destroy_font(Fuente); // Se liberan la fuente
    al_destroy_timer(Timer); // se liberan el temporizador
    al_destroy_event_queue(Cola); // se liberan la cola de eventos
    al_destroy_display(Ventana); // Se cierra la ventana

    return 0; // El programa termino correctamente
}




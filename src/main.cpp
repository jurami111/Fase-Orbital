#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>

#include <cmath>       // para calcular las posiciones con seno y coseno
#include <cstdlib>     // para utilizar numeros aleatorios
#include <cstring>     // Para copiar el alias del jugador
#include <ctime>       // Para cambiar los numeros aleatorios en cada partida
#include <iostream>    // Para mostrar mensajes en la consola
#include <string>      // para trabajar con textos

#include "Estadisticas/Persistencia.h"
#include "Vista/VistaAllegro.h"
#include "Estructuras/ListaBalas.h"
#include "Estructuras/ListaEnemigos.h"
#include "Motor/Oleada.h"
#include "Estructuras/ColaSpawn.h"

// Agrega un enemigo a la lista usando los datos de la cola
void CrearEnemigo(ListaEnemigos& Enemigos, DatosSpawn Datos, int& SiguienteId, int OleadaActual)
{
    const double Pi = 3.14159265;

    Enemigo NuevoEnemigo{};

    NuevoEnemigo.id = SiguienteId;
    SiguienteId++;

    NuevoEnemigo.tipo = Datos.tipo;
    NuevoEnemigo.r = 300;
    NuevoEnemigo.theta = Datos.angulo * Pi / 180.0;
    NuevoEnemigo.color = Datos.color;
    NuevoEnemigo.ticksColor = 0;

    // el girador puede moverse en dos sentidos
    if (Datos.tipo == TipoEnemigo::Girador)
    {
        NuevoEnemigo.omega = (Datos.sentido ? 1 : -1) * (0.9 + 0.05 * OleadaActual) * 3.14159265 / 180.0;
    }
    else
    {
        NuevoEnemigo.omega = 0;
    }

    // los tanques tienen mas resistencia
    if (Datos.tipo == TipoEnemigo::Tanque)
    {
        NuevoEnemigo.vida = 3;
        NuevoEnemigo.radio = 13;
    }
    else if (Datos.tipo == TipoEnemigo::Mini)
    {
        NuevoEnemigo.vida = 1;
        NuevoEnemigo.radio = 7;
    }
    else
    {
        NuevoEnemigo.vida = 1;
        NuevoEnemigo.radio = 11;
        if (Datos.tipo == TipoEnemigo::Divisor) NuevoEnemigo.radio = 10;
        if (Datos.tipo == TipoEnemigo::Dron) NuevoEnemigo.radio = 12;
        if (Datos.tipo == TipoEnemigo::Camaleon) NuevoEnemigo.radio = 11;
    }

    EnemigoPtr Nuevo = &NuevoEnemigo;
    Enemigos.InsertarFinal(Nuevo);
}

// deja listas las estadisticas y la semilla para una nueva partida
void ReiniciarPartida(Jugador& Datos, int& Semilla, bool& Guardada)
{
    Datos = Jugador{};
    std::strcpy(Datos.Alias, "Jugador");
    Semilla = (int)time(NULL);
    srand((unsigned int)Semilla);
    Guardada = false;
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
    bool EnPausa = false;
    bool Izquierda = false;
    bool Derecha = false;

    // Posicion de la nave
    float Angulo = 0; // Indica en que parte de la orbita esta
    float Radio = 150; // Distancia de la nave al nucleo

    // listas que guardan las balas y los enemigos
    ListaBalas Balas;
    ListaEnemigos Enemigos;

    // esto es para los enemigos de cada oleada
    Oleada GeneradorOleadas;
    ColaSpawn EnemigosPendientes;
    int TickOleada = 0;

    int TiempoDisparo = 0; // cuenta el tiempo entre disparos
    int CargaPulso = 0; // carga de 0 a 100
    bool PulsoActivo = false;
    float RadioPulso = 20; // radio de la onda
    int Puntaje = 0; // puntos de la partida
    int ColorSeleccionado = 0; // Se empieza utilizando cian
    int EnemigosEliminados = 0; // Para calcular el puntaje
    int Combo = 0; // aumenta al eliminar enemigos
    Jugador DatosPartida; // estadisticas de la partida actual
    int Semilla = 0; // Semilla de la partida actual
    bool PartidaGuardada = false; // Evita guardar la misma partida dos veces

    // variables nuevas para controlar las oleadas
    int Oleada = 1; // la partida empieza en la oleada uno
    int SiguienteId = 1; // Numero que se asigna al proximo enemigo
    int EsperaOleada = 0; // Tiempo antes de comenzar otra oleada
    bool OleadaTerminada = false; // Indica si ya eliminamos todos
    bool GameOver = false; // indica si perdimos la partida
    bool Victoria = false; // indica si se gana la partida
    int VidaNucleo = 100; // Empieza con 100
    const int TotalOleadas = 5; // Hay que superar cinco oleadas

    // se cambia la semilla para que los colores no sean siempre iguales
    ReiniciarPartida(DatosPartida, Semilla, PartidaGuardada);
    

    // Prepara los enemigos de la primera oleada
    EnemigosPendientes = GeneradorOleadas.Generar(Semilla, Oleada);
    TickOleada = 0;

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
        if (!CarpetaCapturas.empty() && Evento.type == ALLEGRO_EVENT_TIMER && PasoCaptura >= 1 && PasoCaptura <= 3)
        {
            Evento = ALLEGRO_EVENT{};
            Evento.type = ALLEGRO_EVENT_MOUSE_BUTTON_DOWN;
            Evento.mouse.button = 1;
            Evento.mouse.x = 400;

            // se elige la posicion del boton que se va a presionar
            Evento.mouse.y = PasoCaptura == 1 ? 250 :
                             (PasoCaptura == 2 ? 310 : 190);

            PasoCaptura++; // Se pasa a la siguiente captura
        }

        // Se revisa si se hizo clic con el mouse
        if (Evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && Evento.mouse.button == 1)
        {
            const float x = Evento.mouse.x; // posicion horizontal
            const float y = Evento.mouse.y; // Posicion vertical

            
            // se confirma el nombre con el boton de inicio
            if (EnNombre && x >= 280 && x <= 520 && y >= 340 && y <= 385 && !NombreJugador.empty())
            {
                std::strncpy(DatosPartida.Alias, NombreJugador.c_str(), sizeof(DatosPartida.Alias) - 1);
                DatosPartida.Alias[sizeof(DatosPartida.Alias) - 1] = '\0';
                EnNombre = false;
            }
            else if (EnNombre && x >= 280 && x <= 520 && y >= 405 && y <= 445)
            {
                EnNombre = false;
                EnMenu = true;
            }
            else
            // se revisa si estamos en victoria o derrota (no en el menu)
            if ((GameOver || Victoria) && !EnMenu)
            {
                // Boton para volver a jugar
                if (x >= 280 && x <= 520 && y >= 320 && y <= 365)
                {
                    // Se quitan las balas que quedaron de la partida
                    while (Balas.primero() != nullptr)
                    {
                        NodoBalaPtr BalaActual = Balas.primero();
                        Balas.Eliminar(BalaActual); // eliminamos la bala
                    }

                    // Se quitan los enemigos que quedaron de la partida anterior
                    while (Enemigos.primero() != nullptr)
                    {
                        NodoEnemigoPtr EnemigoActual = Enemigos.primero(); // Se toma el primero
                        Enemigos.Eliminar(EnemigoActual); // Lo se quitan de la lista
                    }

                    // se reinicia los datos de la partida
                    Oleada = 1; // Se vuelve a la primera oleada
                    SiguienteId = 1; // Se reinicia los numeros de enemigos
                    EsperaOleada = 0; // Ya no hay tiempo de espera
                    OleadaTerminada = false; // la nueva oleada no ha terminado

                    EnemigosEliminados = 0; // Se vuelve a cero puntos
                    ReiniciarPartida(DatosPartida, Semilla, PartidaGuardada);
                    VidaNucleo = 100; // La nueva partida empieza con vida completa
                    Combo = 0; // se reinicia el combo
                    TiempoDisparo = 0; // se reinicia los disparos
                    CargaPulso = 0; PulsoActivo = false; RadioPulso = 20; Puntaje = 0; // Reinicia el pulso
                    ColorSeleccionado = 0; // Se empieza con cian
                    Angulo = 0; // La nave vuelve a su posicion inicial

                    Izquierda = false; // se deja de mover la nave
                    Derecha = false;
                    EnPausa = false;

                    GameOver = false; // se quitan la derrota
                    Victoria = false; // se quitan la victoria
                    EnMenu = false; EnNombre = true; NombreJugador.clear(); // Se pide el nombre antes de jugar

                    // Se preparan la primera oleada de la nueva partida
                    EnemigosPendientes = GeneradorOleadas.Generar(Semilla, Oleada);
                    TickOleada = 0;
                }

                // Boton para regresar al menu principal
                else if (x >= 280 && x <= 520 && y >= 385 && y <= 430)
                {
                    EnMenu = true; // Regresamos al menu
                    EnPausa = false; // Se quitan la pausa

                    Izquierda = false;
                    Derecha = false;
                }
            }


            // Si estamos en pausa, se revisa el boton Seguir
            else if (!EnMenu && EnPausa && x >= 335 && x <= 465 && y >= 270 && y <= 313)
            {
                EnPausa = false; // Se quitan la pausa
            }

            // si estamos en el menu, se revisa los botones
            else if (EnMenu && x >= 280 && x <= 520)
            {
                if (y >= 180 && y <= 220)
                {
                    // limpiamos las balas de la partida anterior
                    while (Balas.primero() != nullptr)
                    {
                        NodoBalaPtr BalaActual = Balas.primero(); // Se guardan la primera bala
                        Balas.Eliminar(BalaActual); // La se quitan de la lista
                    }// se quitan las balas viejas

                    while (Enemigos.primero() != nullptr)
                        Enemigos.Eliminar(Enemigos.primero()); // Se quitan los enemigos viejos

                    Oleada = 1; // se vuelve a la primera oleada
                    SiguienteId = 1; // se reinicia los numeros de enemigos
                    EsperaOleada = 0; // Ya no estamos esperando otra oleada
                    OleadaTerminada = false;
                    GameOver = false; // Se quitan la pantalla de derrota
                    Victoria = false; // se quitan la pantalla de victoria
                    EnPausa = false;
                    Angulo = 0; // La nave vuelve a su posicion inicial
                    ColorSeleccionado = 0; // Se vuelve al cian
                    TiempoDisparo = 0;
                    CargaPulso = 0; PulsoActivo = false; RadioPulso = 20; Puntaje = 0; // reinicia el pulso
                    EnemigosEliminados = 0; // se reinicia los puntos
                    ReiniciarPartida(DatosPartida, Semilla, PartidaGuardada);
                    VidaNucleo = 100; // La nueva partida empieza con vida completa
                    Combo = 0; // Se reinicia el combo
                    Izquierda = false;
                    Derecha = false;

                    // Se preparan la primera oleada de la nueva partida
                    EnemigosPendientes = GeneradorOleadas.Generar(Semilla, Oleada);
                    TickOleada = 0;

                    EnMenu = false; EnNombre = true; NombreJugador.clear(); // Se pide el nombre antes de jugar
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
            Izquierda = false;
            Derecha = false;
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
                    EnPausa = false; // Se quitan la pausa
                }

                Izquierda = false;
                Derecha = false;
            }

            // al terminar se puede regresar al menu con ESC
            // El reinicio se hace cuando se vuelve a presionar Jugar

            // La tecla P pone y quita la pausa
            if (!EnMenu && !EnNombre && !GameOver && !Victoria && Evento.keyboard.keycode == ALLEGRO_KEY_P)
            {
                EnPausa = !EnPausa;
            }

            // la flecha izquierda mueve la nave en una direccion
            if (!EnMenu && !EnNombre && !EnPausa && Evento.keyboard.keycode == ALLEGRO_KEY_LEFT)
            {
                Izquierda = true;
            }

            // La flecha derecha la mueve en la otra direccion
            if (!EnMenu && !EnNombre && !EnPausa && Evento.keyboard.keycode == ALLEGRO_KEY_RIGHT)
            {
                Derecha = true;
            }

            // se selecciona el color cian con 1 o J
            if (!EnMenu && !EnNombre && !EnPausa && (Evento.keyboard.keycode == ALLEGRO_KEY_1 || Evento.keyboard.keycode == ALLEGRO_KEY_J))
            {
                ColorSeleccionado = 0;
            }

            // Se selecciona magenta con 2 o K
            if (!EnMenu && !EnNombre && !EnPausa && (Evento.keyboard.keycode == ALLEGRO_KEY_2 || Evento.keyboard.keycode == ALLEGRO_KEY_K))
            {
                ColorSeleccionado = 1;
            }

            // se selecciona ambar con 3 o L
            if (!EnMenu && !EnNombre && !EnPausa && (Evento.keyboard.keycode == ALLEGRO_KEY_3 || Evento.keyboard.keycode == ALLEGRO_KEY_L))
            {
                ColorSeleccionado = 2;
            }
        }

        // se escribe el alias antes de iniciar la partida
        if (Evento.type == ALLEGRO_EVENT_KEY_CHAR && EnNombre)
        {
            int Tecla = Evento.keyboard.keycode;
            int Letra = Evento.keyboard.unichar;
            if (Tecla == ALLEGRO_KEY_BACKSPACE && !NombreJugador.empty()) NombreJugador.pop_back();
            else if (Tecla == ALLEGRO_KEY_ENTER && !NombreJugador.empty())
            {
                std::strncpy(DatosPartida.Alias, NombreJugador.c_str(), sizeof(DatosPartida.Alias) - 1);
                DatosPartida.Alias[sizeof(DatosPartida.Alias) - 1] = '\0';
                EnNombre = false;
            }
            else if (Letra >= 32 && Letra <= 126 && NombreJugador.size() < sizeof(DatosPartida.Alias) - 1)
            {
                // los nombres se guardan con caracteres simples para el historial CSV
                if (Letra != ',' && Letra != '"') NombreJugador += (char)Letra;
            }
        }

        // el pulso se activa solo cuando la carga esta completa
        if (Evento.type == ALLEGRO_EVENT_KEY_DOWN && !EnMenu && !EnNombre && !EnPausa && !GameOver && !Victoria && Evento.keyboard.keycode == ALLEGRO_KEY_SPACE && CargaPulso == 100 && !PulsoActivo)
        {
            PulsoActivo = true;
            RadioPulso = 20;
            CargaPulso = 0;
            DatosPartida.pulsos++;
        }

        // se revisa cuando se sueltan las flechas
        if (Evento.type == ALLEGRO_EVENT_KEY_UP)
        {
            if (Evento.keyboard.keycode == ALLEGRO_KEY_LEFT)
            {
                Izquierda = false; // se deja de girar a la izquierda
            }

            if (Evento.keyboard.keycode == ALLEGRO_KEY_RIGHT)
            {
                Derecha = false; // se deja de girar a la derecha
            }
        }

        // El temporizador actualiza el juego varias veces por segundo
        if (Evento.type == ALLEGRO_EVENT_TIMER)
        {
            // pantalla para ingresar el nombre del jugador
            if (EnNombre)
            {
                al_clear_to_color(Fondo);
                al_draw_text(Fuente, Colores[0], 400, 115, ALLEGRO_ALIGN_CENTRE, "NOMBRE DEL JUGADOR");
                al_draw_text(Fuente, Crema, 400, 190, ALLEGRO_ALIGN_CENTRE, "Escriba su nombre para comenzar");
                al_draw_filled_rectangle(220, 250, 580, 305, Morado);
                al_draw_rectangle(220, 250, 580, 305, Colores[0], 2);
                std::string TextoNombre = NombreJugador.empty() ? "Nombre..." : NombreJugador;
                al_draw_text(Fuente, Crema, 400, 265, ALLEGRO_ALIGN_CENTRE, TextoNombre.c_str());
                al_draw_filled_rectangle(280, 340, 520, 385, Colores[0]);
                al_draw_text(Fuente, Fondo, 400, 350, ALLEGRO_ALIGN_CENTRE, "Comenzar");
                al_draw_rectangle(280, 405, 520, 445, Colores[1], 2);
                al_draw_text(Fuente, Crema, 400, 412, ALLEGRO_ALIGN_CENTRE, "Volver");
                al_draw_text(Fuente, Crema, 400, 480, ALLEGRO_ALIGN_CENTRE, "Enter para iniciar, ESC para volver");
                al_flip_display();
                continue;
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

            // solo actualizamos la partida cuando no esta en pausa
            if (!EnPausa && !GameOver && !Victoria)
            {
                DatosPartida.ticks++;

                // se mueve la nave por su orbita
                if (Izquierda)
                {
                    Angulo -= 0.04f; // Giramos hacia un lado
                }

                if (Derecha)
                {
                    Angulo += 0.04f; // Giramos hacia el otro lado
                }

                // La oleada se revisa despues de las colisiones
                // Asi tambien se puede comprobar si ya se gana

                // si terminamos la oleada, contamos el tiempo de espera
                if (OleadaTerminada)
                {
                    EsperaOleada--; // Vamos descontando el tiempo

                    if (EsperaOleada <= 0)
                    {
                        Oleada++; // Se pasa a la siguiente oleada

                        // Se preparan los enemigos que apareceran en esta oleada
                        EnemigosPendientes = GeneradorOleadas.Generar(Semilla, Oleada);
                        TickOleada = 0;

                        OleadaTerminada = false; // Ya puede continuar
                        Combo = 0; // Se reinicia el combo
                    }
                }

                // Se revisa si ya debe aparecer otro enemigo
                if (!OleadaTerminada)
                {
                    TickOleada++;

                    const DatosSpawn* Siguiente = EnemigosPendientes.VerFrente();

                    while (Siguiente != nullptr && Siguiente->tick <= TickOleada)
                    {
                        // Se guardan los datos antes de quitar el elemento de la cola
                        DatosSpawn Datos = *Siguiente;

                        CrearEnemigo(Enemigos, Datos, SiguienteId, Oleada);
                        EnemigosPendientes.Desencolar();

                        Siguiente = EnemigosPendientes.VerFrente();
                    }
                }

                // La nave dispara solamente cuando hay enemigos en pantalla
                if (!OleadaTerminada)
                {
                    TiempoDisparo++; // cuenta los ticks entre disparos

                    if (TiempoDisparo >= 4)
                    {
                        TiempoDisparo = 0; // Reinicia el contador
                        Bala NuevaBala{}; // Crea una bala nueva

                        // La bala aparece un poco adelante de la nave
                        NuevaBala.x =
                            400 + cos(Angulo) * (Radio + 15);

                        NuevaBala.y =
                            310 + sin(Angulo) * (Radio + 15);

                        // se indica hacia donde se va a mover
                        NuevaBala.vx = cos(Angulo) * 9;
                        NuevaBala.vy = sin(Angulo) * 9;

                        NuevaBala.color = ColorSeleccionado; // color actual
                        NuevaBala.duenio = 1; // pertenece al jugador
                        DatosPartida.disparos++;

                        // Se guardan la bala en la lista
                        BalaPtr Nueva = &NuevaBala;
                        Balas.InsertarInicio(Nueva);
                    }
                }

                
                // se recorre la lista para mover todos los enemigos
                NodoEnemigoPtr EnemigoActual = Enemigos.primero();

                while (EnemigoActual != nullptr)
                {
                    // se toma los datos del enemigo actual
                    Enemigo* Actual = EnemigoActual->dato;

                    // velocidad de cada tipo de enemigo
                    float Velocidad = 0.5f + 0.05f * Oleada;   // esto luego cambiar a 8
                    if (Actual->tipo == TipoEnemigo::Girador || Actual->tipo == TipoEnemigo::Camaleon) Velocidad *= 0.85f;
                    if (Actual->tipo == TipoEnemigo::Divisor) Velocidad *= 0.80f;
                    if (Actual->tipo == TipoEnemigo::Mini) Velocidad *= 1.25f;
                    if (Actual->tipo == TipoEnemigo::Tanque) Velocidad *= 0.50f;
                    Actual->r -= Velocidad;

                    // Los minis y giradores avanzan en espiral
                    if (Actual->tipo == TipoEnemigo::Mini || Actual->tipo == TipoEnemigo::Girador) Actual->theta += Actual->omega;

                    // el camaleon cambia de color cada cierto tiempo
                    if (Actual->tipo == TipoEnemigo::Camaleon)
                    {
                        Actual->ticksColor++;

                        // cambia de color cada 42 ticks
                        if (Actual->ticksColor >= 42)
                        {
                            Actual->color = (Actual->color + 1) % 3;
                            Actual->ticksColor = 0;
                        }
                    }

                    // Se calcula la posicion del enemigo
                    float EnemigoX = 400 + cos(Actual->theta) * Actual->r;
                    float EnemigoY = 310 + sin(Actual->theta) * Actual->r;

                    // Se calcula la posicion de la nave
                    float NaveX = 400 + cos(Angulo) * Radio;
                    float NaveY = 310 + sin(Angulo) * Radio;

                    // se calcula la distancia entre el enemigo y la nave
                    float DiferenciaX = EnemigoX - NaveX;
                    float DiferenciaY = EnemigoY - NaveY;

                    float DistanciaCuadrada = DiferenciaX * DiferenciaX + DiferenciaY * DiferenciaY;

                    float RadioChoque = Actual->radio + 5;

                    // se revisa si el enemigo choco con la nave
                    if (DistanciaCuadrada < RadioChoque * RadioChoque)
                    {
                        VidaNucleo -= 12; // El choque quita 12 puntos
                        Combo = 0;

                        // Se quitan al enemigo y continuamos con el siguiente
                        EnemigoActual = Enemigos.Eliminar(EnemigoActual);

                        if (VidaNucleo <= 0)
                        {
                            VidaNucleo = 0;
                            GameOver = true;
                            break;
                        }

                        continue;
                    }

                    // se revisa si el enemigo llego al nucleo
                    if (Actual->r <= 20 + 0.4f * Actual->radio)
                    {
                        // Los tanques hacen mas daño
                        if (Actual->tipo == TipoEnemigo::Tanque)
                        {
                            VidaNucleo -= 22;
                        }
                        else
                        {
                            VidaNucleo -= 10;
                        }

                        Combo = 0;

                        // eliminamos al enemigo que llego al nucleo
                        EnemigoActual = Enemigos.Eliminar(EnemigoActual);

                        if (VidaNucleo <= 0)
                        {
                            VidaNucleo = 0;
                            GameOver = true;
                            break;
                        }

                        continue;
                    }

                    // Se pasa al siguiente enemigo una sola vez
                    EnemigoActual = EnemigoActual->siguiente;
                }

                // Ahora se recorre las balas para moverlas
                NodoBalaPtr BalaActual = Balas.primero();

                while (BalaActual != nullptr)
                {
                    Bala* Actual = BalaActual->dato; // Bala que estamos revisando

                    // Se suma la velocidad a su posicion
                    Actual->x += Actual->vx;
                    Actual->y += Actual->vy;

                    // Se calcula que tan lejos esta del centro
                    float DistanciaX = Actual->x - 400;
                    float DistanciaY = Actual->y - 310;

                    float DistanciaCuadrada = DistanciaX * DistanciaX + DistanciaY * DistanciaY;

                    // eliminamos las balas que ya salieron de la zona de juego
                    if (DistanciaCuadrada > 320 * 320)
                    {
                        BalaActual = Balas.Eliminar(BalaActual);
                        continue; // se sigue con la siguiente bala
                    }

                    // Si la bala sigue dentro, se revisa si golpea un enemigo
                    bool BalaImpacto = false;

                    NodoEnemigoPtr EnemigoRevisado = Enemigos.primero();

                    while (EnemigoRevisado != nullptr)
                    {
                        Enemigo* Objetivo = EnemigoRevisado->dato;

                        // Se convierte la posicion circular del enemigo
                        // A una posicion X y Y dentro de la pantalla
                        float EnemigoX = 400 + cos(Objetivo->theta) * Objetivo->r;

                        float EnemigoY = 310 + sin(Objetivo->theta) * Objetivo->r;

                        // se calcula la diferencia entre la bala y el enemigo
                        float DiferenciaX = Actual->x - EnemigoX;
                        float DiferenciaY = Actual->y - EnemigoY;

                        // Se suma los radios para saber cuando se tocan
                        float RadioColision = Objetivo->radio + 5;

                        // Se comprueba si la bala esta dentro del area del enemigo
                        if (DiferenciaX * DiferenciaX + DiferenciaY * DiferenciaY <= RadioColision * RadioColision)
                        {
                            BalaImpacto = true; // La bala golpeo al enemigo

                            // Se revisa si ambos tienen el mismo color
                            if (Actual->color == Objetivo->color)
                            {
                                Objetivo->vida--; // Le se quitan una vida
                                DatosPartida.correctos++;

                                // si se quedo sin vida, eliminamos al enemigo
                                if (Objetivo->vida <= 0)
                                {
                                    // Si eliminamos un divisor, aparecen dos minis
                                    if (Objetivo->tipo == TipoEnemigo::Divisor)
                                    {
                                        NodoEnemigoPtr Posicion = EnemigoRevisado;
                                        for (int i = 0; i < 2; i++)
                                        {
                                            Enemigo Mini{};

                                            Mini.id = SiguienteId;
                                            SiguienteId++;

                                            Mini.tipo = TipoEnemigo::Mini;
                                            Mini.r = Objetivo->r;
                                            Mini.theta = Objetivo->theta + (i == 0 ? -0.0698132 : 0.0698132);
                                            Mini.omega = i == 0 ? -0.0296706 : 0.0296706;
                                            Mini.color = (Objetivo->color + i + 1) % 3;
                                            Mini.vida = 1;
                                            Mini.radio = 7;
                                            Mini.ticksColor = 0;

                                            EnemigoPtr Nuevo = &Mini;
                                            Posicion = Enemigos.InsertarDespues(Posicion, Nuevo);
                                        }
                                    }

                                    // puntos y carga segun el enemigo eliminado
                                    int PuntosBaja = 100;
                                    if (Objetivo->tipo == TipoEnemigo::Girador) PuntosBaja = 150;
                                    if (Objetivo->tipo == TipoEnemigo::Divisor) PuntosBaja = 200;
                                    if (Objetivo->tipo == TipoEnemigo::Mini) PuntosBaja = 75;
                                    if (Objetivo->tipo == TipoEnemigo::Tanque) PuntosBaja = 400;
                                    if (Objetivo->tipo == TipoEnemigo::Camaleon) PuntosBaja = 300;
                                    int Multiplicador = 1 + Combo / 6;
                                    if (Multiplicador > 8) Multiplicador = 8;
                                    Puntaje += PuntosBaja * Multiplicador;
                                    CargaPulso += Objetivo->tipo == TipoEnemigo::Tanque ? 14 : 5;
                                    if (CargaPulso > 100) CargaPulso = 100;
                                    EnemigosEliminados++; // aumenta las bajas
                                    Combo++; // Aumenta el combo
                                    if (Combo > DatosPartida.combo_max) DatosPartida.combo_max = Combo;

                                    if (Objetivo->tipo == TipoEnemigo::Tanque) DatosPartida.bajas_tanque++;
                                    else if (Objetivo->tipo == TipoEnemigo::Girador) DatosPartida.bajas_girador++;
                                    else if (Objetivo->tipo == TipoEnemigo::Divisor) DatosPartida.bajas_divisor++;
                                    else if (Objetivo->tipo == TipoEnemigo::Mini) DatosPartida.bajas_mini++;
                                    else if (Objetivo->tipo == TipoEnemigo::Camaleon) DatosPartida.bajas_camaleon++;
                                    else DatosPartida.bajas_dron++;

                                    // Lo se quitan de la lista
                                    EnemigoRevisado =
                                        Enemigos.Eliminar(EnemigoRevisado);
                                }
                            }
                            else
                            {
                                // si el color es incorrecto, lo alejamos un poco
                                Objetivo->r += 9;

                                Combo = 0; // Se pierde el combo
                                DatosPartida.erroneos++;
                            }

                            break; // Una bala solo puede golpear a un enemigo
                        }

                        // si no hubo colision, se revisa el siguiente enemigo
                        EnemigoRevisado = EnemigoRevisado->siguiente;
                    }

                    // se quitan la bala si ya golpeo a un enemigo
                    if (BalaImpacto)
                    {
                        BalaActual = Balas.Eliminar(BalaActual);
                    }
                    else
                    {
                        // Si no golpeo nada, la se deja seguir avanzando
                        BalaActual = BalaActual->siguiente;
                    }
                }

                // La onda avanza y elimina los enemigos que toca
                if (PulsoActivo)
                {
                    RadioPulso += 12;
                    NodoEnemigoPtr EnemigoOnda = Enemigos.primero();
                    while (EnemigoOnda != nullptr)
                    {
                        Enemigo* Actual = EnemigoOnda->dato;
                        if (fabs(Actual->r - RadioPulso) < Actual->radio + 3)
                        {
                            int PuntosBaja = 100;
                            if (Actual->tipo == TipoEnemigo::Girador) PuntosBaja = 150;
                            if (Actual->tipo == TipoEnemigo::Divisor) PuntosBaja = 200;
                            if (Actual->tipo == TipoEnemigo::Mini) PuntosBaja = 75;
                            if (Actual->tipo == TipoEnemigo::Tanque) PuntosBaja = 400;
                            if (Actual->tipo == TipoEnemigo::Camaleon) PuntosBaja = 300;
                            int Multiplicador = 1 + Combo / 6;
                            if (Multiplicador > 8) Multiplicador = 8;
                            Puntaje += PuntosBaja * Multiplicador;
                            EnemigosEliminados++;
                            if (Actual->tipo == TipoEnemigo::Tanque) DatosPartida.bajas_tanque++;
                            else if (Actual->tipo == TipoEnemigo::Girador) DatosPartida.bajas_girador++;
                            else if (Actual->tipo == TipoEnemigo::Divisor) DatosPartida.bajas_divisor++;
                            else if (Actual->tipo == TipoEnemigo::Mini) DatosPartida.bajas_mini++;
                            else if (Actual->tipo == TipoEnemigo::Camaleon) DatosPartida.bajas_camaleon++;
                            else DatosPartida.bajas_dron++;
                            EnemigoOnda = Enemigos.Eliminar(EnemigoOnda);
                        }
                        else EnemigoOnda = EnemigoOnda->siguiente;
                    }
                    if (RadioPulso > 320) PulsoActivo = false;
                }

                // Se revisa si eliminamos todos los enemigos
                if (!OleadaTerminada && Enemigos.vacia() && EnemigosPendientes.isVacia() &&!GameOver)
                {
                    if (Oleada >= TotalOleadas)
                    {
                        Victoria = true; // Se gana al completar la quinta oleada
                    }
                    else
                    {
                        OleadaTerminada = true; // terminamos la oleada actual
                        EsperaOleada = 60; // Se espera antes de empezar otra

                        // Se recuperan 8 puntos de vida al completar la oleada
                        VidaNucleo += 8;

                        // La vida nunca puede superar los 100 puntos
                        if (VidaNucleo > 100)
                        {
                            VidaNucleo = 100;
                        }
                    }
                }
            } // Se cierra la actualizacion de la partida
              // los dibujos quedan afuera para mostrar Game Over y Victoria

            // Se guardan la partida una sola vez al terminar
            if ((GameOver || Victoria) && !PartidaGuardada)
            {
                PartidaGuardada = true;
                DatosPartida.puntaje = Puntaje;
                DatosPartida.numero_oleada = Oleada;
                DatosPartida.posicion = 1;

                JugadorPtr Jugadores[1] = {&DatosPartida};
                Archivos.guardarPartidaCompleta(ArchivoHistorial, Jugadores, 1, Semilla, "Clasico", &HistorialJugadores);
                TablaRanking.ConstruirDesdeHistorial(&HistorialJugadores);
            }

            // limpiamos la pantalla antes de volver a dibujar todo
            al_clear_to_color(Fondo);

            // Se dibuja solamente la orbita por donde se mueve la nave
            // No agregamos las otras lineas ni circulos decorativos
            al_draw_circle( 400, 310, Radio, Morado, 2 );

            // se dibuja el nucleo en el centro de la pantalla
            al_draw_filled_circle( 400, 310, 40, Morado );

            // este circulo muestra el color seleccionado
            al_draw_filled_circle( 400, 310, 29, Colores[ColorSeleccionado] );

            // Parte interna del nucleo
            al_draw_filled_circle( 400, 310, 24, Crema );

            // circulo del pulso mientras se expande
            if (PulsoActivo) al_draw_circle(400, 310, RadioPulso, Colores[0], 4);

            // Se recorre los enemigos para dibujarlos
            NodoEnemigoPtr EnemigoDibujo = Enemigos.primero();

            while (EnemigoDibujo != nullptr)
            {
                // Se toma los datos del enemigo actual
                Enemigo* Actual = EnemigoDibujo->dato;

                // se calcula su posicion alrededor del nucleo
                float x = 400 + cos(Actual->theta) * Actual->r;
                float y = 310 + sin(Actual->theta) * Actual->r;

                // Cada enemigo tiene una forma sencilla
                if (Actual->tipo == TipoEnemigo::Tanque)
                {
                    al_draw_filled_rectangle(x - 13, y - 13, x + 13, y + 13, Colores[Actual->color]);
                    al_draw_filled_rectangle(x - 5, y - 5, x + 5, y + 5, Fondo);
                }
                else if (Actual->tipo == TipoEnemigo::Girador)
                {
                    al_draw_filled_triangle(x, y - 11, x - 11, y + 9, x + 11, y + 9, Colores[Actual->color]);
                    al_draw_filled_circle(x, y, 2, Fondo);
                }
                else if (Actual->tipo == TipoEnemigo::Divisor)
                {
                    al_draw_filled_rectangle(x - 10, y - 10, x + 10, y + 10, Colores[Actual->color]);
                    al_draw_line(x - 7, y - 7, x + 7, y + 7, Fondo, 2);
                }
                else if (Actual->tipo == TipoEnemigo::Mini)
                {
                    al_draw_filled_triangle(x, y - 7, x - 7, y + 6, x + 7, y + 6, Colores[Actual->color]);
                }
                else if (Actual->tipo == TipoEnemigo::Camaleon)
                {
                    al_draw_filled_ellipse(x, y, 11, 9, Colores[Actual->color]);
                    al_draw_filled_circle(x, y, 2, Fondo);
                }
                else
                {
                    // el dron es un rombo relleno
                    al_draw_filled_triangle(x, y - 12, x + 12, y, x - 12, y, Colores[Actual->color]);
                    al_draw_filled_triangle(x, y + 12, x + 12, y, x - 12, y, Colores[Actual->color]);
                }

                // se pasa al siguiente enemigo
                EnemigoDibujo = EnemigoDibujo->siguiente;
            }

            // Se recorre la lista para dibujar las balas
            NodoBalaPtr BalaDibujo = Balas.primero();

            while (BalaDibujo != nullptr)
            {
                Bala* Actual = BalaDibujo->dato; // Bala que vamos a dibujar

                // cada bala utiliza el color con el que fue disparada
                al_draw_filled_circle( Actual->x, Actual->y, 5, Colores[Actual->color] );

                BalaDibujo = BalaDibujo->siguiente; // Siguiente bala
            }

            // Se calcula donde se encuentra la nave
            float NaveX = 400 + cos(Angulo) * Radio;
            float NaveY = 310 + sin(Angulo) * Radio;

            // se calcula la punta de la nave
            float PuntaX = NaveX + cos(Angulo) * 18;
            float PuntaY = NaveY + sin(Angulo) * 18;

            // se calcula los otros dos puntos del triangulo
            float Lado1X = NaveX + cos(Angulo + 2.5f) * 13;
            float Lado1Y = NaveY + sin(Angulo + 2.5f) * 13;

            float Lado2X = NaveX + cos(Angulo - 2.5f) * 13;
            float Lado2Y = NaveY + sin(Angulo - 2.5f) * 13;

            // Se dibuja la nave como un triangulo
            al_draw_filled_triangle( PuntaX, PuntaY, Lado1X, Lado1Y, Lado2X, Lado2Y, Colores[ColorSeleccionado] );

            // se muestra el puntaje en la esquina superior izquierda
            // cada enemigo eliminado vale 100 puntos
            std::string TextoPuntaje = "Puntos: " + std::to_string(Puntaje);

            al_draw_text( Fuente, Crema, 20, 20, 0, TextoPuntaje.c_str() );

            // Dibuja la barra de vida del nucleo
            float AnchoBarra = 150;
            float AltoBarra = 10;
            float x = 20;
            float y = 80;

            // Se muestra el texto arriba de la barra
            al_draw_text(Fuente, Crema, x, 50, 0, "Vida");

            // se calcula cuanto debe llenarse la barra
            float VidaActual = AnchoBarra * VidaNucleo / 100.0f;

            // Se dibuja el fondo gris
            al_draw_filled_rectangle( x, y, x + AnchoBarra, y + AltoBarra, al_map_rgb(80, 80, 80) );

            // se dibuja la vida en verde
            al_draw_filled_rectangle( x, y, x + VidaActual, y + AltoBarra, al_map_rgb(60, 180, 80) );

            // Se dibuja un borde sencillo
            al_draw_rectangle( x, y, x + AnchoBarra, y + AltoBarra, Crema, 1 );

            // muestra la oleada actual y el combo
            std::string TextoOleada = "Oleada " + std::to_string(Oleada) + "  Combo x" + std::to_string(Combo);

            al_draw_text( Fuente, Crema, 780, 20, ALLEGRO_ALIGN_RIGHT, TextoOleada.c_str() );

            // Textos que muestran los colores disponibles
            al_draw_text( Fuente, Colores[0], 140, 545, ALLEGRO_ALIGN_CENTRE, "1 Cian" );

            al_draw_text( Fuente, Colores[1], 310, 545, ALLEGRO_ALIGN_CENTRE, "2 Magenta" );

            al_draw_text( Fuente, Colores[2], 510, 545, ALLEGRO_ALIGN_CENTRE, "3 Ambar" );

            // barra para ver la carga del pulso
            al_draw_text(Fuente, Crema, 680, 530, ALLEGRO_ALIGN_CENTRE, CargaPulso == 100 ? "ESPACIO Pulso" : "Pulso");
            al_draw_filled_rectangle(615, 566, 745, 578, Morado);
            al_draw_filled_rectangle(615, 566, 615 + CargaPulso * 1.3f, 578, Colores[0]);
            al_draw_rectangle(615, 566, 745, 578, Crema, 1);

            // cuando termina una oleada se muestra un aviso
            if (OleadaTerminada && !EnPausa)
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
            if (EnPausa)
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
            if (GameOver)
            {
                // Se cubre el juego con un fondo oscuro
                al_draw_filled_rectangle( 0, 0, 800, 600, al_map_rgb(20, 17, 43) );

                // Titulo de derrota
                al_draw_text( Fuente, Colores[1], 400, 190, ALLEGRO_ALIGN_CENTRE, "GAME OVER" );

                // se muestra cuantos puntos conseguimos
                std::string PuntosFinales = "Puntos: " + std::to_string(Puntaje);

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
            if (Victoria)
            {
                // Fondo para que solo se vea el resultado
                al_draw_filled_rectangle( 0, 0, 800, 600, al_map_rgb(20, 17, 43) );

                // Titulo de victoria
                al_draw_text( Fuente, Colores[0], 400, 190, ALLEGRO_ALIGN_CENTRE, "VICTORIA" );

                al_draw_text( Fuente, Crema, 400, 250, ALLEGRO_ALIGN_CENTRE, "Completaste las 5 oleadas" );

                // Puntaje final
                std::string PuntosFinales = "Puntos: " + std::to_string(Puntaje);

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




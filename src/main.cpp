#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>

#include <cmath>       // Para calcular las posiciones con seno y coseno
#include <cstdlib>     // Para utilizar numeros aleatorios
#include <cstring>     // Para copiar el alias del jugador
#include <ctime>       // Para cambiar los numeros aleatorios en cada partida
#include <iostream>    // Para mostrar mensajes en la consola
#include <string>      // Para trabajar con textos

#include "Estadisticas/Persistencia.h"
#include "Vista/VistaAllegro.h"
#include "Estructuras/ListaBalas.h"
#include "Estructuras/ListaEnemigos.h"

// Esta funcion agrega los enemigos de una nueva oleada
// Recibe la lista donde se guardan y el numero de oleada
void CrearOleada(ListaEnemigos& Enemigos, int Oleada, int& SiguienteId)
{
    const float Pi = 3.14159265f;

    // En la primera oleada aparecen dos enemigos
    // En las siguientes se agrega uno mas
    int Cantidad = Oleada + 1;

    if (Cantidad > 10)
        Cantidad = 10; // Dejamos un maximo para no llenar la pantalla

    // Creamos los enemigos uno por uno
    for (int i = 0; i < Cantidad; i++)
    {
        Enemigo NuevoEnemigo{}; // Guardamos los datos del enemigo nuevo

        NuevoEnemigo.id = SiguienteId; // Cada enemigo tiene un numero diferente
        SiguienteId++; // Preparamos el numero del siguiente enemigo

        // Alternamos entre drones y tanques
        if (i % 3 == 2)
            NuevoEnemigo.tipo = TipoEnemigo::Tanque;
        else
            NuevoEnemigo.tipo = TipoEnemigo::Dron;

        // Los enemigos aparecen alrededor del nucleo
        // Dividimos el circulo para que no aparezcan todos juntos
        NuevoEnemigo.theta = (2.0f * Pi * i) / Cantidad;

        // Todos empiezan lejos del centro
        NuevoEnemigo.r = 300;

        // Por ahora no giran alrededor del nucleo
        NuevoEnemigo.omega = 0;

        // Asignamos uno de los tres colores disponibles
        NuevoEnemigo.color = rand() % 3;

        // El tanque tiene mas vida que el dron
        if (NuevoEnemigo.tipo == TipoEnemigo::Tanque)
        {
            NuevoEnemigo.vida = 3;
            NuevoEnemigo.radio = 16; // El tanque se dibuja mas grande
        }
        else
        {
            NuevoEnemigo.vida = 1;
            NuevoEnemigo.radio = 11; // El dron es mas pequeno
        }

        NuevoEnemigo.ticksColor = 0; // Todavia no cambia de color

        // La lista guarda una copia del enemigo
        EnemigoPtr Nuevo = &NuevoEnemigo;
        Enemigos.InsertarFinal(Nuevo);
    }
}

// Deja listas las estadisticas y la semilla para una nueva partida
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

    // Revisamos si se enviaron opciones al ejecutar el programa
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

    // Cargamos los datos de partidas anteriores
    Historial HistorialJugadores;
    Persistencia Archivos;

    const bool HistorialCargado =
        Archivos.cargarHistorial(ArchivoHistorial, &HistorialJugadores);

    Ranking TablaRanking;
    TablaRanking.ConstruirDesdeHistorial(&HistorialJugadores);

    VistaAllegro Reportes; // Para mostrar el ranking y las estadisticas
    int PasoCaptura = 0; // Controla las capturas de pantalla

    // Iniciamos Allegro y las herramientas que necesita el juego
    al_init();
    al_install_keyboard();

    if (!al_install_mouse())
    {
        std::cerr << "No se pudo instalar el mouse.\n";
        return 1;
    }

    al_init_primitives_addon(); // Para dibujar circulos y otras figuras
    al_init_font_addon();       // Para mostrar textos
    al_init_ttf_addon();        // Para cargar fuentes de Windows

    if (!CarpetaCapturas.empty() && !al_init_image_addon())
    {
        std::cerr << "No se pudo inicializar el addon de imagen.\n";
        return 1;
    }

    // Creamos la ventana, los eventos y el temporizador
    ALLEGRO_DISPLAY* Ventana = al_create_display(800, 600);
    ALLEGRO_EVENT_QUEUE* Cola = al_create_event_queue();
    ALLEGRO_TIMER* Timer = al_create_timer(1.0 / 30);

    // Utilizamos la fuente Consolas para los textos
    ALLEGRO_FONT* Fuente =
        al_load_ttf_font("C:/Windows/Fonts/consola.ttf", 22, 0);

    if (Fuente == NULL)
        Fuente = al_create_builtin_font(); // Usamos otra si no se encuentra

    // Comprobamos que los recursos se hayan creado
    if (Ventana == NULL || Cola == NULL || Timer == NULL || Fuente == NULL)
        return 1;

    al_set_window_title(Ventana, "Fase Orbital");

    // Registramos los eventos que vamos a utilizar
    al_register_event_source(Cola, al_get_display_event_source(Ventana));
    al_register_event_source(Cola, al_get_keyboard_event_source());
    al_register_event_source(Cola, al_get_timer_event_source(Timer));
    al_register_event_source(Cola, al_get_mouse_event_source());

    // Colores principales de la pantalla
    ALLEGRO_COLOR Fondo = al_map_rgb(20, 17, 43);
    ALLEGRO_COLOR Crema = al_map_rgb(235, 242, 246);
    ALLEGRO_COLOR Morado = al_map_rgb(58, 52, 90);
    ALLEGRO_COLOR Coral = al_map_rgb(110, 217, 241);

    // Los tres colores que utilizamos para las balas y enemigos
    ALLEGRO_COLOR Colores[3] = {
        al_map_rgb(68, 210, 225),  // Cian
        al_map_rgb(238, 91, 176),  // Magenta
        al_map_rgb(245, 183, 73)   // Ambar
    };

    // Variables para controlar las pantallas y las teclas
    bool Salir = false;
    bool EnMenu = true;
    bool EnPausa = false;
    bool Izquierda = false;
    bool Derecha = false;

    // Posicion de la nave
    float Angulo = 0; // Indica en que parte de la orbita esta
    float Radio = 150; // Distancia de la nave al nucleo

    // Listas que guardan las balas y los enemigos
    ListaBalas Balas;
    ListaEnemigos Enemigos;

    int TiempoDisparo = 0; // Cuenta el tiempo entre disparos
    int ColorSeleccionado = 0; // Empezamos utilizando cian
    int EnemigosEliminados = 0; // Para calcular el puntaje
    int Combo = 0; // Aumenta al eliminar enemigos
    Jugador DatosPartida; // Estadisticas de la partida actual
    int Semilla = 0; // Semilla de la partida actual
    bool PartidaGuardada = false; // Evita guardar la misma partida dos veces

    // Variables nuevas para controlar las oleadas
    int Oleada = 1; // La partida empieza en la oleada uno
    int SiguienteId = 1; // Numero que se asigna al proximo enemigo
    int EsperaOleada = 0; // Tiempo antes de comenzar otra oleada
    bool OleadaTerminada = false; // Indica si ya eliminamos todos
    bool GameOver = false; // Indica si perdimos la partida
    bool Victoria = false; // Indica si ganamos la partida
    const int TotalOleadas = 5; // Hay que superar cinco oleadas

    // Cambiamos la semilla para que los colores no sean siempre iguales
    ReiniciarPartida(DatosPartida, Semilla, PartidaGuardada);

    // Creamos los enemigos de la primera oleada
    CrearOleada(Enemigos, Oleada, SiguienteId);

    // Iniciamos el temporizador para actualizar el juego
    al_start_timer(Timer);

    
    // Ciclo principal del juego
    // Se repite mientras no cerremos la ventana
    while (!Salir)
    {
        ALLEGRO_EVENT Evento;
        al_wait_for_event(Cola, &Evento); // Esperamos un evento

        // Esta parte se utiliza para las capturas automaticas
        // La dejamos para que sigan funcionando las pruebas
        if (!CarpetaCapturas.empty() &&
            Evento.type == ALLEGRO_EVENT_TIMER &&
            PasoCaptura >= 1 && PasoCaptura <= 3)
        {
            Evento = ALLEGRO_EVENT{};
            Evento.type = ALLEGRO_EVENT_MOUSE_BUTTON_DOWN;
            Evento.mouse.button = 1;
            Evento.mouse.x = 400;

            // Elegimos la posicion del boton que se va a presionar
            Evento.mouse.y = PasoCaptura == 1 ? 250 :
                             (PasoCaptura == 2 ? 310 : 190);

            PasoCaptura++; // Pasamos a la siguiente captura
        }

        // Revisamos si se hizo clic con el mouse
        if (Evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN &&
            Evento.mouse.button == 1)
        {
            const float x = Evento.mouse.x; // Posicion horizontal
            const float y = Evento.mouse.y; // Posicion vertical

            
            // Revisamos si estamos en victoria o derrota (no en el menu)
            if ((GameOver || Victoria) && !EnMenu)
            {
                // Boton para volver a jugar
                if (x >= 280 && x <= 520 &&
                    y >= 320 && y <= 365)
                {
                    // Quitamos las balas que quedaron de la partida
                    while (Balas.primero() != nullptr)
                    {
                        NodoBalaPtr BalaActual = Balas.primero();
                        Balas.Eliminar(BalaActual); // Eliminamos la bala
                    }

                    // Quitamos los enemigos que quedaron de la partida anterior
                    while (Enemigos.primero() != nullptr)
                    {
                        NodoEnemigoPtr EnemigoActual = Enemigos.primero(); // Tomamos el primero
                        Enemigos.Eliminar(EnemigoActual); // Lo quitamos de la lista
                    }

                    // Reiniciamos los datos de la partida
                    Oleada = 1; // Volvemos a la primera oleada
                    SiguienteId = 1; // Reiniciamos los numeros de enemigos
                    EsperaOleada = 0; // Ya no hay tiempo de espera
                    OleadaTerminada = false; // La nueva oleada no ha terminado

                    EnemigosEliminados = 0; // Volvemos a cero puntos
                    ReiniciarPartida(DatosPartida, Semilla, PartidaGuardada);
                    Combo = 0; // Reiniciamos el combo
                    TiempoDisparo = 0; // Reiniciamos los disparos
                    ColorSeleccionado = 0; // Empezamos con cian
                    Angulo = 0; // La nave vuelve a su posicion inicial

                    Izquierda = false; // Dejamos de mover la nave
                    Derecha = false;
                    EnPausa = false;

                    GameOver = false; // Quitamos la derrota
                    Victoria = false; // Quitamos la victoria
                    EnMenu = false; // Entramos directamente al juego

                    // Creamos los enemigos de la primera oleada
                    CrearOleada(Enemigos, Oleada, SiguienteId);
                }

                // Boton para regresar al menu principal
                else if (x >= 280 && x <= 520 &&
                         y >= 385 && y <= 430)
                {
                    EnMenu = true; // Regresamos al menu
                    EnPausa = false; // Quitamos la pausa

                    Izquierda = false;
                    Derecha = false;
                }
            }


            // Si estamos en pausa, revisamos el boton Seguir
            else if (!EnMenu && EnPausa &&
                x >= 335 && x <= 465 &&
                y >= 270 && y <= 313)
            {
                EnPausa = false; // Quitamos la pausa
            }

            // Si estamos en el menu, revisamos los botones
            else if (EnMenu && x >= 280 && x <= 520)
            {
                if (y >= 180 && y <= 220)
                {
                    // Limpiamos las balas de la partida anterior
                    while (Balas.primero() != nullptr)
                    {
                        NodoBalaPtr BalaActual = Balas.primero(); // Guardamos la primera bala
                        Balas.Eliminar(BalaActual); // La quitamos de la lista
                    }// Quitamos las balas viejas

                    while (Enemigos.primero() != nullptr)
                        Enemigos.Eliminar(Enemigos.primero()); // Quitamos los enemigos viejos

                    Oleada = 1; // Volvemos a la primera oleada
                    SiguienteId = 1; // Reiniciamos los numeros de enemigos
                    EsperaOleada = 0; // Ya no estamos esperando otra oleada
                    OleadaTerminada = false;
                    GameOver = false; // Quitamos la pantalla de derrota
                    Victoria = false; // Quitamos la pantalla de victoria
                    EnPausa = false;
                    Angulo = 0; // La nave vuelve a su posicion inicial
                    ColorSeleccionado = 0; // Volvemos al cian
                    TiempoDisparo = 0;
                    EnemigosEliminados = 0; // Reiniciamos los puntos
                    ReiniciarPartida(DatosPartida, Semilla, PartidaGuardada);
                    Combo = 0; // Reiniciamos el combo
                    Izquierda = false;
                    Derecha = false;

                    CrearOleada(Enemigos, Oleada, SiguienteId); // Primera oleada nueva
                    EnMenu = false; // Comenzamos a jugar
                }

                // Botones de ranking y estadisticas
                else if ((y >= 240 && y <= 280) ||
                         (y >= 300 && y <= 340))
                {
                    const bool EsRanking = y <= 280;

                    // Preparamos la ruta si se necesita una captura
                    const std::string Captura =
                        CarpetaCapturas.empty() ? "" :
                        CarpetaCapturas +
                        (EsRanking ? "/ranking.png" : "/estadisticas.png");

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
                    Salir = true; // Cerramos el juego
                }
            }

            // Evitamos que la nave siga girando al usar el mouse
            Izquierda = false;
            Derecha = false;
        }

        // Cerramos el juego si se presiona la X de la ventana
        if (Evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        {
            Salir = true;
        }

        // Revisamos cuando se presiona alguna tecla
        if (Evento.type == ALLEGRO_EVENT_KEY_DOWN)
        {
            // ESC permite regresar al menu o cerrar el programa
            if (Evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
            {
                if (EnMenu)
                {
                    Salir = true; // Salimos si ya estamos en el menu
                }
                else
                {
                    EnMenu = true; // Regresamos al menu
                    EnPausa = false; // Quitamos la pausa
                }

                Izquierda = false;
                Derecha = false;
            }

            // Al terminar podemos regresar al menu con ESC
            // El reinicio se hace cuando volvemos a presionar Jugar

            // La tecla P pone y quita la pausa
            if (!EnMenu && !GameOver && !Victoria &&
                Evento.keyboard.keycode == ALLEGRO_KEY_P)
            {
                EnPausa = !EnPausa;
            }

            // La flecha izquierda mueve la nave en una direccion
            if (!EnMenu && !EnPausa &&
                Evento.keyboard.keycode == ALLEGRO_KEY_LEFT)
            {
                Izquierda = true;
            }

            // La flecha derecha la mueve en la otra direccion
            if (!EnMenu && !EnPausa &&
                Evento.keyboard.keycode == ALLEGRO_KEY_RIGHT)
            {
                Derecha = true;
            }

            // Seleccionamos el color cian con 1 o J
            if (!EnMenu && !EnPausa &&
                (Evento.keyboard.keycode == ALLEGRO_KEY_1 ||
                 Evento.keyboard.keycode == ALLEGRO_KEY_J))
            {
                ColorSeleccionado = 0;
            }

            // Seleccionamos magenta con 2 o K
            if (!EnMenu && !EnPausa &&
                (Evento.keyboard.keycode == ALLEGRO_KEY_2 ||
                 Evento.keyboard.keycode == ALLEGRO_KEY_K))
            {
                ColorSeleccionado = 1;
            }

            // Seleccionamos ambar con 3 o L
            if (!EnMenu && !EnPausa &&
                (Evento.keyboard.keycode == ALLEGRO_KEY_3 ||
                 Evento.keyboard.keycode == ALLEGRO_KEY_L))
            {
                ColorSeleccionado = 2;
            }
        }

        // Revisamos cuando se sueltan las flechas
        if (Evento.type == ALLEGRO_EVENT_KEY_UP)
        {
            if (Evento.keyboard.keycode == ALLEGRO_KEY_LEFT)
            {
                Izquierda = false; // Dejamos de girar a la izquierda
            }

            if (Evento.keyboard.keycode == ALLEGRO_KEY_RIGHT)
            {
                Derecha = false; // Dejamos de girar a la derecha
            }
        }

        // El temporizador actualiza el juego varias veces por segundo
        if (Evento.type == ALLEGRO_EVENT_TIMER)
        {
            // Primero revisamos si estamos en el menu
            if (EnMenu)
            {
                al_clear_to_color(Fondo); // Fondo oscuro y sin rayas

                // Titulo principal
                al_draw_text(
                    Fuente, Colores[0], 400, 55,
                    ALLEGRO_ALIGN_CENTRE, "FASE ORBITAL"
                );

                // Dibujamos el nucleo que aparece en el menu
                al_draw_filled_circle(400, 125, 28, Morado);
                al_draw_filled_circle(400, 125, 17, Crema);

                // Textos de los cuatro botones
                const char* Botones[] = {
                    "Jugar", "Ranking", "Estadisticas", "Salir"
                };

                // Dibujamos cada boton en una posicion diferente
                for (int indice = 0; indice < 4; indice++)
                {
                    const int y = 180 + indice * 60;

                    // Fondo del boton
                    al_draw_filled_rectangle(
                        280, y, 520, y + 40,
                        al_map_rgb(36, 31, 66)
                    );

                    // Borde del boton
                    al_draw_rectangle(
                        280, y, 520, y + 40,
                        indice % 2 == 0 ? Colores[0] : Colores[1], 1
                    );

                    // Nombre del boton
                    al_draw_text(
                        Fuente, Crema, 400,
                        y + (40 - al_get_font_line_height(Fuente)) / 2,
                        ALLEGRO_ALIGN_CENTRE, Botones[indice]
                    );
                }

                // Avisamos si hubo un problema al cargar los datos
                if (!HistorialCargado)
                {
                    al_draw_text(
                        Fuente, Coral, 400, 460,
                        ALLEGRO_ALIGN_CENTRE,
                        "No se pudo cargar el historial"
                    );
                }

                // Guardamos una captura del menu si se solicito
                if (!CarpetaCapturas.empty() && PasoCaptura == 0)
                {
                    if (!al_save_bitmap(
                        (CarpetaCapturas + "/principal.png").c_str(),
                        al_get_backbuffer(Ventana)))
                    {
                        std::cerr << "No se pudo guardar la captura del menu.\n";
                        Salir = true;
                    }

                    PasoCaptura = 1;
                }

                al_flip_display(); // Mostramos el menu
                continue; // No actualizamos la partida mientras estamos aqui
            }

            // Solo actualizamos la partida cuando no esta en pausa
            if (!EnPausa && !GameOver && !Victoria)
            {
                DatosPartida.ticks++;

                // Movemos la nave por su orbita
                if (Izquierda)
                {
                    Angulo -= 0.04f; // Giramos hacia un lado
                }

                if (Derecha)
                {
                    Angulo += 0.04f; // Giramos hacia el otro lado
                }

                // La oleada se revisa despues de las colisiones
                // Asi tambien podemos comprobar si ya ganamos

                // Si terminamos la oleada, contamos el tiempo de espera
                if (OleadaTerminada)
                {
                    EsperaOleada--; // Vamos descontando el tiempo

                    if (EsperaOleada <= 0)
                    {
                        Oleada++; // Pasamos a la siguiente oleada

                        // Creamos los enemigos de la nueva oleada
                        CrearOleada(Enemigos, Oleada, SiguienteId);

                        OleadaTerminada = false; // Ya puede continuar
                        Combo = 0; // Reiniciamos el combo
                    }
                }

                // Disparamos solamente cuando hay enemigos en pantalla
                if (!OleadaTerminada)
                {
                    TiempoDisparo++; // Contamos los ticks entre disparos

                    if (TiempoDisparo >= 4)
                    {
                        TiempoDisparo = 0; // Reiniciamos el contador

                        Bala NuevaBala{}; // Creamos una bala nueva

                        // La bala aparece un poco adelante de la nave
                        NuevaBala.x =
                            400 + cos(Angulo) * (Radio + 15);

                        NuevaBala.y =
                            310 + sin(Angulo) * (Radio + 15);

                        // Indicamos hacia donde se va a mover
                        NuevaBala.vx = cos(Angulo) * 9;
                        NuevaBala.vy = sin(Angulo) * 9;

                        NuevaBala.color = ColorSeleccionado; // Color actual
                        NuevaBala.duenio = 1; // Pertenece al jugador
                        DatosPartida.disparos++;

                        // Guardamos la bala en la lista
                        BalaPtr Nueva = &NuevaBala;
                        Balas.InsertarInicio(Nueva);
                    }
                }

                
                // Recorremos la lista para mover todos los enemigos
                NodoEnemigoPtr EnemigoActual = Enemigos.primero();

                while (EnemigoActual != nullptr)
                {
                    // Guardamos el enemigo que estamos revisando
                    Enemigo* Actual = EnemigoActual->dato;

                    // Cada oleada aumenta un poquito la velocidad
                    float Velocidad = 0.25f + (Oleada - 1) * 0.06f;

                    Actual->r -= Velocidad; // El enemigo se acerca al nucleo

                    // Revisamos si el enemigo ya llego al nucleo
                    if (Actual->r <= 40 + Actual->radio)
                    {
                        GameOver = true; // Perdemos porque el enemigo alcanzo el centro
                        break; // Ya no necesitamos revisar mas enemigos
}

                    // Pasamos al siguiente enemigo de la lista
                    EnemigoActual = EnemigoActual->siguiente;
                }

                // Ahora recorremos las balas para moverlas
                NodoBalaPtr BalaActual = Balas.primero();

                while (BalaActual != nullptr)
                {
                    Bala* Actual = BalaActual->dato; // Bala que estamos revisando

                    // Sumamos la velocidad a su posicion
                    Actual->x += Actual->vx;
                    Actual->y += Actual->vy;

                    // Calculamos que tan lejos esta del centro
                    float DistanciaX = Actual->x - 400;
                    float DistanciaY = Actual->y - 310;

                    float DistanciaCuadrada =
                        DistanciaX * DistanciaX +
                        DistanciaY * DistanciaY;

                    // Eliminamos las balas que ya salieron de la zona de juego
                    if (DistanciaCuadrada > 320 * 320)
                    {
                        BalaActual = Balas.Eliminar(BalaActual);
                        continue; // Seguimos con la siguiente bala
                    }

                    // Si la bala sigue dentro, revisamos si golpea un enemigo
                    bool BalaImpacto = false;

                    NodoEnemigoPtr EnemigoRevisado = Enemigos.primero();

                    while (EnemigoRevisado != nullptr)
                    {
                        Enemigo* Objetivo = EnemigoRevisado->dato;

                        // Convertimos la posicion circular del enemigo
                        // a una posicion X y Y dentro de la pantalla
                        float EnemigoX =
                            400 + cos(Objetivo->theta) * Objetivo->r;

                        float EnemigoY =
                            310 + sin(Objetivo->theta) * Objetivo->r;

                        // Calculamos la diferencia entre la bala y el enemigo
                        float DiferenciaX = Actual->x - EnemigoX;
                        float DiferenciaY = Actual->y - EnemigoY;

                        // Sumamos los radios para saber cuando se tocan
                        float RadioColision = Objetivo->radio + 5;

                        // Comprobamos si la bala esta dentro del area del enemigo
                        if (DiferenciaX * DiferenciaX +
                            DiferenciaY * DiferenciaY <=
                            RadioColision * RadioColision)
                        {
                            BalaImpacto = true; // La bala golpeo al enemigo

                            // Revisamos si ambos tienen el mismo color
                            if (Actual->color == Objetivo->color)
                            {
                                Objetivo->vida--; // Le quitamos una vida
                                DatosPartida.correctos++;

                                // Si se quedo sin vida, eliminamos al enemigo
                                if (Objetivo->vida <= 0)
                                {
                                    EnemigosEliminados++; // Aumenta el puntaje
                                    Combo++; // Aumenta el combo
                                    if (Combo > DatosPartida.combo_max) DatosPartida.combo_max = Combo;

                                    if (Objetivo->tipo == TipoEnemigo::Tanque) DatosPartida.bajas_tanque++;
                                    else DatosPartida.bajas_dron++;

                                    // Lo quitamos de la lista
                                    EnemigoRevisado =
                                        Enemigos.Eliminar(EnemigoRevisado);
                                }
                            }
                            else
                            {
                                // Si el color es incorrecto, lo alejamos un poco
                                Objetivo->r += 9;

                                Combo = 0; // Se pierde el combo
                                DatosPartida.erroneos++;
                            }

                            break; // Una bala solo puede golpear a un enemigo
                        }

                        // Si no hubo colision, revisamos el siguiente enemigo
                        EnemigoRevisado = EnemigoRevisado->siguiente;
                    }

                    // Quitamos la bala si ya golpeo a un enemigo
                    if (BalaImpacto)
                    {
                        BalaActual = Balas.Eliminar(BalaActual);
                    }
                    else
                    {
                        // Si no golpeo nada, la dejamos seguir avanzando
                        BalaActual = BalaActual->siguiente;
                    }
                }

                // Revisamos si eliminamos todos los enemigos
                if (!OleadaTerminada && Enemigos.vacia() && !GameOver)
                {
                    if (Oleada >= TotalOleadas)
                    {
                        Victoria = true; // Ganamos al completar la quinta oleada
                    }
                    else
                    {
                        OleadaTerminada = true; // Terminamos la oleada actual
                        EsperaOleada = 60; // Esperamos antes de empezar otra
                    }
                }
            } // Cerramos la actualizacion de la partida
              // Los dibujos quedan afuera para mostrar Game Over y Victoria

            // Guardamos la partida una sola vez al terminar
            if ((GameOver || Victoria) && !PartidaGuardada)
            {
                PartidaGuardada = true;
                DatosPartida.puntaje = EnemigosEliminados * 100;
                DatosPartida.numero_oleada = Oleada;
                DatosPartida.posicion = 1;

                JugadorPtr Jugadores[1] = {&DatosPartida};
                Archivos.guardarPartidaCompleta(ArchivoHistorial, Jugadores, 1, Semilla, "Clasico", &HistorialJugadores);
                TablaRanking.ConstruirDesdeHistorial(&HistorialJugadores);
            }

            // Limpiamos la pantalla antes de volver a dibujar todo
            al_clear_to_color(Fondo);

            // Dibujamos solamente la orbita por donde se mueve la nave
            // No agregamos las otras lineas ni circulos decorativos
            al_draw_circle(
                400, 310, Radio,
                Morado, 2
            );

            // Dibujamos el nucleo en el centro de la pantalla
            al_draw_filled_circle(
                400, 310, 40,
                Morado
            );

            // Este circulo muestra el color seleccionado
            al_draw_filled_circle(
                400, 310, 29,
                Colores[ColorSeleccionado]
            );

            // Parte interna del nucleo
            al_draw_filled_circle(
                400, 310, 24,
                Crema
            );

            // Recorremos los enemigos para dibujarlos
            NodoEnemigoPtr EnemigoDibujo = Enemigos.primero();

            while (EnemigoDibujo != nullptr)
            {
                // Tomamos los datos del enemigo actual
                Enemigo* Actual = EnemigoDibujo->dato;

                // Calculamos su posicion alrededor del nucleo
                float x = 400 + cos(Actual->theta) * Actual->r;
                float y = 310 + sin(Actual->theta) * Actual->r;

                // Revisamos si el enemigo es un tanque
                if (Actual->tipo == TipoEnemigo::Tanque)
                {
                    // Los tanques se representan con cuadrados
                    al_draw_filled_rectangle(
                        x - Actual->radio,
                        y - Actual->radio,
                        x + Actual->radio,
                        y + Actual->radio,
                        Colores[Actual->color]
                    );

                    // Dibujamos un cuadrado pequeno en el centro
                    al_draw_filled_rectangle(
                        x - 5, y - 5,
                        x + 5, y + 5,
                        Fondo
                    );
                }
                else
                {
                    // Los drones se representan con circulos
                    al_draw_filled_circle(
                        x, y,
                        Actual->radio,
                        Colores[Actual->color]
                    );

                    // Agregamos un circulo pequeno en el centro
                    al_draw_filled_circle(
                        x, y, 4,
                        Fondo
                    );
                }

                // Pasamos al siguiente enemigo
                EnemigoDibujo = EnemigoDibujo->siguiente;
            }

            // Recorremos la lista para dibujar las balas
            NodoBalaPtr BalaDibujo = Balas.primero();

            while (BalaDibujo != nullptr)
            {
                Bala* Actual = BalaDibujo->dato; // Bala que vamos a dibujar

                // Cada bala utiliza el color con el que fue disparada
                al_draw_filled_circle(
                    Actual->x,
                    Actual->y,
                    5,
                    Colores[Actual->color]
                );

                BalaDibujo = BalaDibujo->siguiente; // Siguiente bala
            }

            // Calculamos donde se encuentra la nave
            float NaveX = 400 + cos(Angulo) * Radio;
            float NaveY = 310 + sin(Angulo) * Radio;

            // Calculamos la punta de la nave
            float PuntaX = NaveX + cos(Angulo) * 18;
            float PuntaY = NaveY + sin(Angulo) * 18;

            // Calculamos los otros dos puntos del triangulo
            float Lado1X = NaveX + cos(Angulo + 2.5f) * 13;
            float Lado1Y = NaveY + sin(Angulo + 2.5f) * 13;

            float Lado2X = NaveX + cos(Angulo - 2.5f) * 13;
            float Lado2Y = NaveY + sin(Angulo - 2.5f) * 13;

            // Dibujamos la nave como un triangulo
            al_draw_filled_triangle(
                PuntaX, PuntaY,
                Lado1X, Lado1Y,
                Lado2X, Lado2Y,
                Colores[ColorSeleccionado]
            );

            // Mostramos el puntaje en la esquina superior izquierda
            // Cada enemigo eliminado vale 100 puntos
            std::string TextoPuntaje =
                "Puntos: " + std::to_string(EnemigosEliminados * 100);

            al_draw_text(
                Fuente, Crema,
                20, 20,
                0,
                TextoPuntaje.c_str()
            );

            // Mostramos la oleada actual y el combo
            std::string TextoOleada =
                "Oleada " + std::to_string(Oleada) +
                "  Combo x" + std::to_string(Combo);

            al_draw_text(
                Fuente, Crema,
                780, 20,
                ALLEGRO_ALIGN_RIGHT,
                TextoOleada.c_str()
            );

            // Textos que muestran los colores disponibles
            al_draw_text(
                Fuente, Colores[0],
                140, 545,
                ALLEGRO_ALIGN_CENTRE,
                "1 Cian"
            );

            al_draw_text(
                Fuente, Colores[1],
                310, 545,
                ALLEGRO_ALIGN_CENTRE,
                "2 Magenta"
            );

            al_draw_text(
                Fuente, Colores[2],
                510, 545,
                ALLEGRO_ALIGN_CENTRE,
                "3 Ambar"
            );

            // Mostramos el texto del pulso
            // Por ahora es solamente parte de la interfaz
            al_draw_text(
                Fuente, Crema,
                680, 545,
                ALLEGRO_ALIGN_CENTRE,
                "Pulso"
            );

            // Cuando termina una oleada mostramos un aviso
            if (OleadaTerminada && !EnPausa)
            {
                // Fondo del aviso para que se pueda leer
                al_draw_filled_rectangle(
                    210, 220, 590, 315,
                    al_map_rgb(33, 29, 62)
                );

                // Borde del aviso
                al_draw_rectangle(
                    210, 220, 590, 315,
                    Colores[0], 2
                );

                // Texto principal
                al_draw_text(
                    Fuente, Crema,
                    400, 235,
                    ALLEGRO_ALIGN_CENTRE,
                    "Oleada completada"
                );

                // Texto que indica que pronto empieza otra oleada
                al_draw_text(
                    Fuente, Colores[0],
                    400, 275,
                    ALLEGRO_ALIGN_CENTRE,
                    "Preparando la siguiente..."
                );
            }

            // Si el juego esta en pausa, mostramos una ventana encima
            if (EnPausa)
            {
                // Fondo de la ventana de pausa
                al_draw_filled_rectangle(
                    280, 200, 520, 340,
                    al_map_rgb(33, 29, 62)
                );

                // Borde de la ventana
                al_draw_rectangle(
                    280, 200, 520, 340,
                    Colores[1], 2
                );

                // Titulo de la pausa
                al_draw_text(
                    Fuente, Crema,
                    400, 225,
                    ALLEGRO_ALIGN_CENTRE,
                    "Pausa"
                );

                // Boton para seguir jugando
                al_draw_filled_rectangle(
                    335, 270, 465, 313,
                    Morado
                );

                al_draw_rectangle(
                    335, 270, 465, 313,
                    Colores[0], 1
                );

                al_draw_text(
                    Fuente, Crema,
                    400, 278,
                    ALLEGRO_ALIGN_CENTRE,
                    "Seguir"
                );
            }

            // Mostramos la pantalla de derrota cuando un enemigo llega al nucleo
            if (GameOver)
            {
                // Tapamos el juego con un fondo oscuro
                al_draw_filled_rectangle(
                    0, 0, 800, 600,
                    al_map_rgb(20, 17, 43)
                );

                // Titulo de derrota
                al_draw_text(
                    Fuente, Colores[1],
                    400, 190,
                    ALLEGRO_ALIGN_CENTRE,
                    "GAME OVER"
                );

                // Mostramos cuantos puntos conseguimos
                std::string PuntosFinales =
                    "Puntos: " + std::to_string(EnemigosEliminados * 100);

                al_draw_text(
                    Fuente, Crema,
                    400, 260,
                    ALLEGRO_ALIGN_CENTRE,
                    PuntosFinales.c_str()
                );

                
                // Boton para intentar otra vez
                al_draw_filled_rectangle(
                    280, 320, 520, 365,
                    Colores[1]
                );

                // Texto del boton
                al_draw_text(
                    Fuente, Fondo,
                    400, 330,
                    ALLEGRO_ALIGN_CENTRE,
                    "Reintentar"
                );

                // Boton para regresar al menu
                al_draw_filled_rectangle(
                    280, 385, 520, 430,
                    Morado
                );

                // Dibujamos el borde del boton
                al_draw_rectangle(
                    280, 385, 520, 430,
                    Colores[1], 2
                );

                // Texto del segundo boton
                al_draw_text(
                    Fuente, Crema,
                    400, 395,
                    ALLEGRO_ALIGN_CENTRE,
                    "Menu principal"
                );

            }

            // Mostramos la pantalla de victoria al superar todas las oleadas
            if (Victoria)
            {
                // Fondo para que solo se vea el resultado
                al_draw_filled_rectangle(
                    0, 0, 800, 600,
                    al_map_rgb(20, 17, 43)
                );

                // Titulo de victoria
                al_draw_text(
                    Fuente, Colores[0],
                    400, 190,
                    ALLEGRO_ALIGN_CENTRE,
                    "VICTORIA"
                );

                al_draw_text(
                    Fuente, Crema,
                    400, 250,
                    ALLEGRO_ALIGN_CENTRE,
                    "Completaste las 5 oleadas"
                );

                // Puntaje final
                std::string PuntosFinales =
                    "Puntos: " + std::to_string(EnemigosEliminados * 100);

                al_draw_text(
                    Fuente, Colores[2],
                    400, 300,
                    ALLEGRO_ALIGN_CENTRE,
                    PuntosFinales.c_str()
                );

                
                // Boton para comenzar otra partida
                al_draw_filled_rectangle(
                    280, 320, 520, 365,
                    Colores[0]
                );

                // Texto del boton
                al_draw_text(
                    Fuente, Fondo,
                    400, 330,
                    ALLEGRO_ALIGN_CENTRE,
                    "Reintentar"
                );

                // Boton para regresar al menu
                al_draw_filled_rectangle(
                    280, 385, 520, 430,
                    Morado
                );

                // Borde del boton
                al_draw_rectangle(
                    280, 385, 520, 430,
                    Colores[0], 2
                );

                // Texto del segundo boton
                al_draw_text(
                    Fuente, Crema,
                    400, 395,
                    ALLEGRO_ALIGN_CENTRE,
                    "Menu principal"
                );

            }

            // Mostramos en pantalla todo lo que dibujamos
            al_flip_display();

        } // Termina el evento del temporizador

    } // Termina el ciclo principal del juego

    // Liberamos los recursos antes de cerrar el programa
    al_destroy_font(Fuente); // Liberamos la fuente
    al_destroy_timer(Timer); // Liberamos el temporizador
    al_destroy_event_queue(Cola); // Liberamos la cola de eventos
    al_destroy_display(Ventana); // Cerramos la ventana

    return 0; // El programa termino correctamente
}




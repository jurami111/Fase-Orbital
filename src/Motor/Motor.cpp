#include "Motor/Motor.h"
#include <algorithm>   // Para std::max y std::min
#include <cctype>
#include <cstring>     // Para copiar el alias del jugador
#include "Controlador.h"
#include <cmath>       // para calcular las posiciones con seno y coseno
#include <ctime>       // Para cambiar los numeros aleatorios en cada partida

namespace {
bool AliasIguales(const char* alias, const std::string& nombre) {
    if (alias == nullptr || nombre.size() != std::strlen(alias)) return false;
    for (std::size_t indice = 0; indice < nombre.size(); indice++) {
        if (std::tolower(static_cast<unsigned char>(alias[indice])) !=
            std::tolower(static_cast<unsigned char>(nombre[indice]))) return false;
    }
    return true;
}

int IndiceTipo(TipoEnemigo tipo) {
    return static_cast<int>(tipo);
}

int RadioTipo(TipoEnemigo tipo) {
    switch (tipo) {
        case TipoEnemigo::Dron: return 6;
        case TipoEnemigo::Girador: return 6;
        case TipoEnemigo::Divisor: return 8;
        case TipoEnemigo::Mini: return 4;
        case TipoEnemigo::Tanque: return 10;
        case TipoEnemigo::Camaleon: return 7;
    }
    return 6;
}

int VidaTipo(TipoEnemigo tipo) {
    return tipo == TipoEnemigo::Tanque ? 3 : 1;
}

double FactorVelocidad(TipoEnemigo tipo) {
    switch (tipo) {
        case TipoEnemigo::Dron: return 1.0;
        case TipoEnemigo::Girador: return 0.85;
        case TipoEnemigo::Divisor: return 0.80;
        case TipoEnemigo::Mini: return 1.25;
        case TipoEnemigo::Tanque: return 0.50;
        case TipoEnemigo::Camaleon: return 0.85;
    }
    return 1.0;
}

int PuntosTipo(TipoEnemigo tipo) {
    switch (tipo) {
        case TipoEnemigo::Dron: return 100;
        case TipoEnemigo::Girador: return 150;
        case TipoEnemigo::Divisor: return 200;
        case TipoEnemigo::Mini: return 75;
        case TipoEnemigo::Tanque: return 400;
        case TipoEnemigo::Camaleon: return 300;
    }
    return 0;
}

// Agrega un enemigo a la lista usando los datos de la cola
void CrearEnemigo(ListaEnemigos& Enemigos, DatosSpawn Datos, int& SiguienteId, int OleadaActual)
{
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
        // El LCG marca true cuando sentido es 0 (horario); theta positiva es antihoraria.
        NuevoEnemigo.omega = (Datos.sentido ? -1.0 : 1.0) *
                             (0.9 + 0.05 * OleadaActual) * Pi / 180.0;
    }
    else
    {
        NuevoEnemigo.omega = 0;
    }

    NuevoEnemigo.vida = VidaTipo(Datos.tipo);
    NuevoEnemigo.radio = RadioTipo(Datos.tipo);

    EnemigoPtr Nuevo = &NuevoEnemigo;
    Enemigos.InsertarFinal(Nuevo);
}

void CrearMinisDivisor(ListaEnemigos& enemigos, NodoEnemigoPtr divisor, int& siguienteId)
{
    if (divisor == nullptr || divisor->dato == nullptr) return;
    const Enemigo padre = *divisor->dato;
    NodoEnemigoPtr posicion = divisor;
    for (int indice = 0; indice < 2; indice++)
    {
        Enemigo mini{};
        mini.id = siguienteId++;
        mini.tipo = TipoEnemigo::Mini;
        mini.r = padre.r;
        mini.theta = padre.theta + (indice == 0 ? -4.0 : 4.0) * Pi / 180.0;
        mini.omega = (indice == 0 ? -1.7 : 1.7) * Pi / 180.0;
        mini.color = (padre.color + indice + 1) % 3;
        mini.vida = VidaTipo(mini.tipo);
        mini.radio = RadioTipo(mini.tipo);
        mini.ticksColor = 0;
        EnemigoPtr puntero = &mini;
        posicion = enemigos.InsertarDespues(posicion, puntero);
    }
}

}

// deja listas las estadisticas y la semilla para una nueva partida
void Motor::ReiniciarPartida(Jugador& Datos, int& Semilla)
{
    Datos = Jugador{};
    Semilla = (int)time(NULL);
}

Motor::Motor()
{
    // se cambia la semilla para que los colores no sean siempre iguales
    ReiniciarPartida(DatosPartida, Semilla);


    // Prepara los enemigos de la primera oleada
    EnemigosPendientes = GeneradorOleadas.Generar(Semilla, Oleada);
    TickOleada = -1;

}

// guarda lo que lleva cada jugador
void Motor::GuardarJugador()
{
    DatosPartida.puntaje = Puntaje;
    DatosPartida.combo = Combo;
    DatosPartida.carga_pulso = CargaPulso;
    DatosPartida.vida_nucleo = VidaNucleo;
    DatosPartida.numero_oleada = Oleada;
    DatosJugadores[IndiceActual] = DatosPartida;
}

void Motor::RegistrarEvento(TipoEvento tipo, TipoEnemigo enemigo, int color, int valor)
{
    if (IndiceActual < 0 || IndiceActual >= Registrados) return;
    Evento nuevo{};
    nuevo.tick = TickOleada;
    nuevo.oleada = Oleada;
    nuevo.jugador = &DatosJugadores[IndiceActual];
    nuevo.tipo_evento = tipo;
    nuevo.tipo_enemigo = enemigo;
    nuevo.color = color;
    nuevo.valor = valor;
    EventoPtr puntero = &nuevo;
    Bitacora.AgregarFinal(puntero);
}

void Motor::RegistrarBaja(TipoEnemigo tipo, int color, int puntos)
{
    const int indiceColor = color >= 0 && color < 3 ? color : 0;
    if (tipo == TipoEnemigo::Dron) DatosPartida.bajas_dron++;
    else if (tipo == TipoEnemigo::Girador) DatosPartida.bajas_girador++;
    else if (tipo == TipoEnemigo::Divisor) DatosPartida.bajas_divisor++;
    else if (tipo == TipoEnemigo::Mini) DatosPartida.bajas_mini++;
    else if (tipo == TipoEnemigo::Tanque) DatosPartida.bajas_tanque++;
    else if (tipo == TipoEnemigo::Camaleon) DatosPartida.bajas_camaleon++;
    DatosPartida.bajas_color[indiceColor]++;
    RegistrarEvento(TipoEvento::BAJA, tipo, indiceColor, puntos);
}

void Motor::RegistrarDano(int dano, TipoEnemigo tipo, TipoEvento evento)
{
    VidaNucleo -= dano;
    DatosPartida.dano_total += dano;
    const int indiceTipo = IndiceTipo(tipo);
    if (indiceTipo >= 0 && indiceTipo < 6) DatosPartida.dano_por_tipo[indiceTipo] += dano;
    RegistrarEvento(evento, tipo, -1, dano);
}

// prepara el turno del que sigue
void Motor::PrepararTurno()
{
    NodoJugadorPtr Nodo = Ronda.VerActual();
    if (Nodo == nullptr) return;
    IndiceActual = int(Nodo->dato - DatosJugadores);
    DatosPartida = DatosJugadores[IndiceActual];
    Puntaje = DatosPartida.puntaje;
    Combo = 0;
    CargaPulso = DatosPartida.carga_pulso;
    VidaNucleo = DatosPartida.vida_nucleo;
    Angulo = static_cast<float>(3.0 * Pi / 2.0);
    ColorSeleccionado = 0;
    PulsoActivo = false;
    RadioPulso = 20;
    TickOleada = -1;
    SiguienteId = 1;
    OleadaTerminada = false;
    EsperaOleada = 0;
    Izquierda = false;
    Derecha = false;
    EnPausa = false;
    // limpia las listas antes del siguiente turno
    NodoBalaPtr BalaVieja = Balas.primero();
    while (BalaVieja != nullptr)
    {
        BalaVieja = Balas.Eliminar(BalaVieja);
    }
    NodoEnemigoPtr EnemigoViejo = Enemigos.primero();
    while (EnemigoViejo != nullptr)
    {
        EnemigoViejo = Enemigos.Eliminar(EnemigoViejo);
    }
    EnemigosPendientes = ColaOriginal; // copia la cola para este jugador
    EnTurno = true;
}

void Motor::AvanzarTick(AccionesTick Acciones)
{
    Izquierda = Acciones.Izquierda;
    Derecha = Acciones.Derecha;
    ColorSeleccionado = Acciones.color;
    Angulo = ActualizarAngulo(Acciones.angulo);
    RadioPulso = Acciones.Radio;
    AvanzarTick();
}

float Motor::ActualizarAngulo(double anguloObjetivo)
{
    // Calcula la diferencia entre el angulo objetivo y el actual
    double diferencia = anguloObjetivo - Angulo;

    double restante = remainder(diferencia, 2.0 * Pi);

    // Actualiza el angulo actual hacia el objetivo
    Angulo += std::max(-0.10472, std::min(0.10472, restante));
    return Angulo;
}

// empieza cuando ya se anotaron todos
void Motor::IniciarMultijugador()
{
    Oleada = 1;
    Horario = true;
    TurnosJugados = 0;
    JugadoresTurno = Ronda.tamano();
    ColaOriginal = GeneradorOleadas.Generar(Semilla, Oleada);
    PrepararTurno();
}

// pasa al otro jugador o a otra oleada
void Motor::TerminarTurno(bool Eliminado)
{
    GuardarJugador();
    if (Eliminado)
    {
        RegistrarEvento(TipoEvento::ELIMINACION, TipoEnemigo::Dron, -1, 0);
        DatosJugadores[IndiceActual].posicion = Ronda.tamano();
        Ronda.EliminarActual(Horario);
    }
    else if (TurnosJugados + 1 < JugadoresTurno)
    {
        Ronda.AvanzarJugador(Horario);
    }

    TurnosJugados++;
    if (TurnosJugados >= JugadoresTurno)
    {
        if (Ronda.tamano() == 0 || (Registrados > 1 && Ronda.tamano() == 1) ||
            Oleada >= TotalOleadas)
        {
            GameOver = Ronda.tamano() == 0;
            Victoria = !GameOver;
            EnTurno = false;
            return;
        }
        Oleada++;
        Horario = !Horario;
        JugadoresTurno = Ronda.tamano();
        TurnosJugados = 0;
        ColaOriginal = GeneradorOleadas.Generar(Semilla, Oleada);
    }
    if (Ronda.tamano() == 0)
    {
        GameOver = true;
        EnTurno = false;
        return;
    }
    PrepararTurno();
}

void Motor::AvanzarTick()
{
    // solo actualizamos la partida cuando no esta en pausa
    if (!EnPausa && !GameOver && !Victoria)
    {
        DatosPartida.ticks++;

        // se mueve la nave con las flechas o con el mouse
        if (Izquierda)
        {
            Angulo += static_cast<float>(6.0 * Pi / 180.0);
        }
        else if (Derecha)
        {
            Angulo -= static_cast<float>(6.0 * Pi / 180.0);
        }
        else if (UsarMouse)
        {
            Angulo = ActualizarAngulo(AnguloMouse);
        }

        Angulo = std::fmod(Angulo + static_cast<float>(2.0 * Pi),static_cast<float>(2.0 * Pi));

        // La oleada se revisa despues de las colisiones
        // Asi tambien se puede comprobar si ya se gana

        // cada jugador juega una oleada completa antes del siguiente turno
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
            if (TickOleada % 4 == 0)
            {
                Bala NuevaBala{}; // Crea una bala nueva

                // La bala aparece un poco adelante de la nave
                NuevaBala.x =
                    400 + cos(Angulo) * (Radio + 15);

                NuevaBala.y =
                            310 - sin(Angulo) * (Radio + 15);

                // se indica hacia donde se va a mover
                NuevaBala.vx = cos(Angulo) * 9;
                NuevaBala.vy = -sin(Angulo) * 9;

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

            // Las primeras oleadas van mas despacio para practicar
            float Velocidad = static_cast<float>((0.8 + 0.08 * Oleada) * FactorVelocidad(Actual->tipo));
            Actual->r -= Velocidad;

            if (Actual->tipo == TipoEnemigo::Mini || Actual->tipo == TipoEnemigo::Girador)
                Actual->theta += Actual->omega;

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
            float EnemigoY = 310 - sin(Actual->theta) * Actual->r;

            // Se calcula la posicion de la nave
            float NaveX = 400 + cos(Angulo) * Radio;
            float NaveY = 310 - sin(Angulo) * Radio;

            // se calcula la distancia entre el enemigo y la nave
            float DiferenciaX = EnemigoX - NaveX;
            float DiferenciaY = EnemigoY - NaveY;

            float DistanciaCuadrada = DiferenciaX * DiferenciaX + DiferenciaY * DiferenciaY;

            float RadioChoque = Actual->radio + 5;

            // se revisa si el enemigo choco con la nave
            if (DistanciaCuadrada < RadioChoque * RadioChoque)
            {
                RegistrarDano(12, Actual->tipo, TipoEvento::CHOQUE_NAVE);
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
                    RegistrarDano(22, Actual->tipo, TipoEvento::DANO_NUCLEO);
                }
                else
                {
                    RegistrarDano(10, Actual->tipo, TipoEvento::DANO_NUCLEO);
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

                float EnemigoY = 310 - sin(Objetivo->theta) * Objetivo->r;

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
                        DatosPartida.correctos_color[Actual->color]++;

                        // si se quedo sin vida, eliminamos al enemigo
                        if (Objetivo->vida <= 0)
                        {
                            // Un Divisor se divide sin importar si lo destruyo bala o pulso.
                            if (Objetivo->tipo == TipoEnemigo::Divisor)
                                CrearMinisDivisor(Enemigos, EnemigoRevisado, SiguienteId);

                            const int PuntosBaja = PuntosTipo(Objetivo->tipo);
                            Combo++; // La baja actual completa la racha antes de calcular el multiplicador.
                            int Multiplicador = 1 + Combo / 6;
                            if (Multiplicador > 8) Multiplicador = 8;
                            Puntaje += PuntosBaja * Multiplicador;
                            CargaPulso += Objetivo->tipo == TipoEnemigo::Tanque ? 14 : 5;
                            if (CargaPulso > 100) CargaPulso = 100;
                            RegistrarBaja(Objetivo->tipo, Actual->color, PuntosBaja * Multiplicador);
                            if (Combo > DatosPartida.combo_max) DatosPartida.combo_max = Combo;

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
                        DatosPartida.erroneos_color[Actual->color]++;
                        RegistrarEvento(TipoEvento::IMPACTO_ERRONEO, Objetivo->tipo,
                                        Actual->color, 0);
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
                    const TipoEnemigo tipo = Actual->tipo;
                    const int color = Actual->color;
                    if (tipo == TipoEnemigo::Divisor)
                        CrearMinisDivisor(Enemigos, EnemigoOnda, SiguienteId);
                    const int PuntosBaja = PuntosTipo(tipo);
                    int Multiplicador = 1 + Combo / 6;
                    if (Multiplicador > 8) Multiplicador = 8;
                    Puntaje += PuntosBaja * Multiplicador;
                    RegistrarBaja(tipo, color, PuntosBaja * Multiplicador);
                    EnemigoOnda = Enemigos.Eliminar(EnemigoOnda);
                }
                else EnemigoOnda = EnemigoOnda->siguiente;
            }
            if (RadioPulso > 320) PulsoActivo = false;
        }

        if (GameOver)
        {
            GameOver = false;
            TerminarTurno(true);
        }
        else if (!OleadaTerminada && Enemigos.vacia() && EnemigosPendientes.isVacia())
        {
            VidaNucleo += 8;
            if (VidaNucleo > 100) VidaNucleo = 100;
            RegistrarEvento(TipoEvento::FIN_OLEADA, TipoEnemigo::Dron, -1, Puntaje);
            TerminarTurno(false);
        }
    } // Se cierra la actualizacion de la partida
}
bool Motor::CanActivarPulso()
{
    if (CargaPulso == 100 && !PulsoActivo)
    {
        return true;
    }
    return false;
}

void Motor::ActivarPulso()
{
    if (CargaPulso == 100 && !PulsoActivo)
    {
        RegistrarEvento(TipoEvento::PULSO, TipoEnemigo::Dron, ColorSeleccionado, 100);
        PulsoActivo = true;
        RadioPulso = 20;
        CargaPulso = 0;
        DatosPartida.pulsos++;
    }
}

void Motor::CalcularPosiciones()
{
    // revisa los puntos y las otras cosas para las posiciones
    if (Ronda.tamano() > 0) GuardarJugador();
    for (int i = 0; i < Registrados; i++)
    {
        int Posicion = 1;
        for (int j = 0; j < Registrados; j++)
        {
            if (i == j) continue;
            const Jugador& Uno = DatosJugadores[i];
            const Jugador& Otro = DatosJugadores[j];
            int TotalUno = Uno.correctos + Uno.erroneos;
            int TotalOtro = Otro.correctos + Otro.erroneos;
            double PrecisionUno = TotalUno ? double(Uno.correctos) / TotalUno : 0;
            double PrecisionOtro = TotalOtro ? double(Otro.correctos) / TotalOtro : 0;
            if (Otro.puntaje > Uno.puntaje ||
                (Otro.puntaje == Uno.puntaje && Otro.numero_oleada > Uno.numero_oleada) ||
                (Otro.puntaje == Uno.puntaje && Otro.numero_oleada == Uno.numero_oleada && PrecisionOtro > PrecisionUno) ||
                (Otro.puntaje == Uno.puntaje && Otro.numero_oleada == Uno.numero_oleada && PrecisionOtro == PrecisionUno && j < i))
                Posicion++;
        }
        DatosJugadores[i].posicion = Posicion;
    }
}

bool Motor::RegistrarNombre(std::string& NombreJugador, std::string& AvisoRegistro)
{
    const std::size_t inicio = NombreJugador.find_first_not_of(" \t");
    if (inicio == std::string::npos) {
        AvisoRegistro = "Escriba un nombre antes de agregar.";
        return false;
    }
    const std::size_t final = NombreJugador.find_last_not_of(" \t");
    const std::string alias = NombreJugador.substr(inicio, final - inicio + 1);
    if (Registrados >= 6) {
        AvisoRegistro = "Solo se permiten seis jugadores.";
        return false;
    }
    for (int indice = 0; indice < Registrados; indice++) {
        if (AliasIguales(DatosJugadores[indice].Alias, alias)) {
            AvisoRegistro = "Ese nombre ya esta registrado.";
            return false;
        }
    }
    Jugador& Nuevo = DatosJugadores[Registrados];
    Nuevo = Jugador{};
    std::strncpy(Nuevo.Alias, alias.c_str(), sizeof(Nuevo.Alias) - 1);
    Nuevo.Alias[sizeof(Nuevo.Alias) - 1] = '\0';
    Nuevo.vida_nucleo = 100;
    Nuevo.isBot = alias.size() >= 3 &&
        std::toupper(static_cast<unsigned char>(alias[0])) == 'B' &&
        std::toupper(static_cast<unsigned char>(alias[1])) == 'O' &&
        std::toupper(static_cast<unsigned char>(alias[2])) == 'T';
    JugadorPtr Puntero = &Nuevo;
    Ronda.AgregarJugador(Puntero);
    Registrados++;
    NombreJugador.clear();
    AvisoRegistro.clear();
    return true;
}

void Motor::ReiniciarRegistro()
{
    while (Ronda.VerActual() != nullptr) Ronda.EliminarActual(true);
    Registrados = 0;
    IndiceActual = 0;
    Bitacora.Limpiar();
    EnTurno = false;
}

void Motor::ReiniciarDesdeVictoria()
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

    ReiniciarPartida(DatosPartida, Semilla);
    VidaNucleo = 100; // La nueva partida empieza con vida completa
    Combo = 0; // se reinicia el combo
    CargaPulso = 0; PulsoActivo = false; RadioPulso = 20; Puntaje = 0; // Reinicia el pulso
    ColorSeleccionado = 0; // Se empieza con cian
    Angulo = static_cast<float>(3.0 * Pi / 2.0);

    Izquierda = false; // se deja de mover la nave
    Derecha = false;
    UsarMouse = false;
    EnPausa = false;

    GameOver = false; // se quitan la derrota
    Victoria = false; // se quitan la victoria
    // Se preparan la primera oleada de la nueva partida
    EnemigosPendientes = GeneradorOleadas.Generar(Semilla, Oleada);
    TickOleada = -1;
}

void Motor::ReiniciarDesdeMenu()
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
    Angulo = static_cast<float>(3.0 * Pi / 2.0);
    ColorSeleccionado = 0; // Se vuelve al cian
    CargaPulso = 0; PulsoActivo = false; RadioPulso = 20; Puntaje = 0; // reinicia el pulso
    ReiniciarPartida(DatosPartida, Semilla);
    VidaNucleo = 100; // La nueva partida empieza con vida completa
    Combo = 0; // Se reinicia el combo
    Izquierda = false;
    Derecha = false;

    // Se preparan la primera oleada de la nueva partida
    EnemigosPendientes = GeneradorOleadas.Generar(Semilla, Oleada);
    TickOleada = -1;

}

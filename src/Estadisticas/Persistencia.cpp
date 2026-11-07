#include "Persistencia.h"

#include <cstring>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#ifdef _WIN32
#include <windows.h>
#endif
#include <filesystem>
#include <system_error>

const char* const Persistencia::FORMATO_CSV =
    "id_partida,semilla,modo,jugador,es_bot,posicion,puntaje,oleada,disparos,"
    "correctos,erroneos,combo_max,pulsos,dano_total,ticks,bajas_dron,"
    "bajas_girador,bajas_divisor,bajas_mini,bajas_tanque,bajas_camaleon,"
    "correctos_cian,correctos_magenta,correctos_ambar,erroneos_cian,erroneos_magenta,erroneos_ambar,"
    "bajas_cian,bajas_magenta,bajas_ambar,dano_dron,dano_girador,dano_divisor,dano_mini,dano_tanque,dano_camaleon";

namespace {
const char* const FORMATO_CSV_ANTIGUO =
    "id_partida,semilla,modo,jugador,es_bot,posicion,puntaje,oleada,disparos,"
    "correctos,erroneos,combo_max,pulsos,dano_total,ticks,bajas_dron,"
    "bajas_girador,bajas_divisor,bajas_mini,bajas_tanque,bajas_camaleon";

bool separarCSV(const std::string& linea, std::string (&campos)[36], int esperados) {
    std::string campo;
    bool entreComillas = false;
    bool comillasCerradas = false;
    int cantidad = 0;

    for (std::size_t i = 0; i < linea.size(); i++) {
        const char caracter = linea[i];
        if (caracter == '"') {
            if (entreComillas && i + 1 < linea.size() && linea[i + 1] == '"') {
                campo += '"';
                i++;
            } else if (entreComillas) {
                entreComillas = false;
                comillasCerradas = true;
            } else {
                if (!campo.empty() || comillasCerradas) return false;
                entreComillas = true;
            }
        } else if (caracter == ',' && !entreComillas) {
            if (cantidad == esperados) return false;
            campos[cantidad++] = campo;
            campo.clear();
            comillasCerradas = false;
        } else {
            if (comillasCerradas) return false;
            campo += caracter;
        }
    }

    if (entreComillas) return false;
    if (cantidad == esperados) return false;
    campos[cantidad++] = campo;
    return cantidad == esperados;
}

bool convertirEntero(const std::string& texto, int& valor) {
    std::istringstream entrada(texto);
    if (!(entrada >> valor)) return false;
    entrada >> std::ws;
    return entrada.eof();
}

std::string escaparCSV(const std::string& texto) {
    if (texto.find_first_of(",\"\r\n") == std::string::npos) return texto;
    std::string resultado = "\"";
    for (char caracter : texto) {
        if (caracter == '"') resultado += '"';
        resultado += caracter;
    }
    return resultado + '"';
}

bool copiarCampo(char* destino, std::size_t capacidad, const std::string& valor) {
    if (valor.size() >= capacidad || valor.find('\0') != std::string::npos) return false;
    std::memcpy(destino, valor.c_str(), valor.size() + 1);
    return true;
}

bool registroValido(const HistorialCSV& registro) {
    if (registro.jugador[0] == '\0' || registro.modo[0] == '\0' ||
        registro.id_partida < 1 || registro.posicion < 1 || registro.posicion > 6 ||
        (registro.es_bot != 0 && registro.es_bot != 1)) return false;
    const int valores[] = {registro.semilla, registro.puntaje, registro.oleada, registro.disparos,
        registro.correctos, registro.erroneos, registro.combo_max, registro.pulsos,
        registro.dano_total, registro.ticks, registro.bajas_dron, registro.bajas_girador,
        registro.bajas_divisor, registro.bajas_mini, registro.bajas_tanque, registro.bajas_camaleon};
    for (int valor : valores) if (valor < 0) return false;
    for (int color = 0; color < 3; color++) {
        if (registro.correctos_color[color] < 0 || registro.erroneos_color[color] < 0 ||
            registro.bajas_color[color] < 0) return false;
    }
    for (int tipo = 0; tipo < 6; tipo++) if (registro.dano_por_tipo[tipo] < 0) return false;
    return true;
}

bool convertirRegistro(const std::string (&campos)[36], int cantidadCampos, HistorialCSVPtr registro) {
    if (!copiarCampo(registro->modo, sizeof(registro->modo), campos[2]) ||
        !copiarCampo(registro->jugador, sizeof(registro->jugador), campos[3])) {
        return false;
    }

    const bool datosBase = convertirEntero(campos[0], registro->id_partida) &&
           convertirEntero(campos[1], registro->semilla) &&
           convertirEntero(campos[4], registro->es_bot) &&
           convertirEntero(campos[5], registro->posicion) &&
           convertirEntero(campos[6], registro->puntaje) &&
           convertirEntero(campos[7], registro->oleada) &&
           convertirEntero(campos[8], registro->disparos) &&
           convertirEntero(campos[9], registro->correctos) &&
           convertirEntero(campos[10], registro->erroneos) &&
           convertirEntero(campos[11], registro->combo_max) &&
           convertirEntero(campos[12], registro->pulsos) &&
           convertirEntero(campos[13], registro->dano_total) &&
           convertirEntero(campos[14], registro->ticks) &&
           convertirEntero(campos[15], registro->bajas_dron) &&
           convertirEntero(campos[16], registro->bajas_girador) &&
           convertirEntero(campos[17], registro->bajas_divisor) &&
           convertirEntero(campos[18], registro->bajas_mini) &&
           convertirEntero(campos[19], registro->bajas_tanque) &&
           convertirEntero(campos[20], registro->bajas_camaleon);
    if (!datosBase) return false;
    if (cantidadCampos == 36) {
        for (int color = 0; color < 3; color++) {
            if (!convertirEntero(campos[21 + color], registro->correctos_color[color]) ||
                !convertirEntero(campos[24 + color], registro->erroneos_color[color]) ||
                !convertirEntero(campos[27 + color], registro->bajas_color[color])) return false;
        }
        for (int tipo = 0; tipo < 6; tipo++) {
            if (!convertirEntero(campos[30 + tipo], registro->dano_por_tipo[tipo])) return false;
        }
    }
    return registroValido(*registro);
}

void escribirRegistroCSV(std::ostream& archivo, const HistorialCSV& registro) {
    archivo << registro.id_partida << ',' << registro.semilla << ',' << escaparCSV(registro.modo) << ','
            << escaparCSV(registro.jugador) << ',' << registro.es_bot << ',' << registro.posicion << ','
            << registro.puntaje << ',' << registro.oleada << ',' << registro.disparos << ','
            << registro.correctos << ',' << registro.erroneos << ',' << registro.combo_max << ','
            << registro.pulsos << ',' << registro.dano_total << ',' << registro.ticks << ','
            << registro.bajas_dron << ',' << registro.bajas_girador << ',' << registro.bajas_divisor << ','
            << registro.bajas_mini << ',' << registro.bajas_tanque << ',' << registro.bajas_camaleon;
    for (int color = 0; color < 3; color++) archivo << ',' << registro.correctos_color[color];
    for (int color = 0; color < 3; color++) archivo << ',' << registro.erroneos_color[color];
    for (int color = 0; color < 3; color++) archivo << ',' << registro.bajas_color[color];
    for (int tipo = 0; tipo < 6; tipo++) archivo << ',' << registro.dano_por_tipo[tipo];
    archivo << '\n';
}

bool migrarCSVAntiguo(const std::string& ruta) {
    std::ifstream entrada(ruta);
    if (!entrada.is_open()) return true;
    std::string encabezado;
    if (!std::getline(entrada, encabezado)) return true;
    if (!encabezado.empty() && encabezado.back() == '\r') encabezado.pop_back();
    if (encabezado == Persistencia::FORMATO_CSV) return true;
    if (encabezado != FORMATO_CSV_ANTIGUO) return false;

    const std::string temporal = ruta + ".tmp";
    std::ofstream salida(temporal, std::ios::trunc);
    if (!salida.is_open()) return false;
    salida << Persistencia::FORMATO_CSV << '\n';
    std::string linea;
    std::size_t numeroLinea = 1;
    while (std::getline(entrada, linea)) {
        numeroLinea++;
        if (!linea.empty() && linea.back() == '\r') linea.pop_back();
        std::string campos[36];
        HistorialCSV registro{};
        if (!separarCSV(linea, campos, 21) || !convertirRegistro(campos, 21, &registro)) {
            std::cerr << "Fila mal formada en línea " << numeroLinea << "; se omite durante la migración.\n";
            continue;
        }
        escribirRegistroCSV(salida, registro);
    }
    
    entrada.close();
    salida.close();

    if (!salida) {
        std::remove(temporal.c_str());
        return false;
    }

    #ifdef _WIN32
        bool reemplazado = MoveFileExA(
            temporal.c_str(),
            ruta.c_str(),
            MOVEFILE_REPLACE_EXISTING
        ) != 0;
    #else
        bool reemplazado = std::rename(
            temporal.c_str(),
            ruta.c_str()
        ) == 0;
    #endif

    if (!reemplazado) {
        std::remove(temporal.c_str());
        return false;
    }

    return true;
    }
}

bool Persistencia::cargarHistorial(const std::string& ruta, Historial* historial) {
    if (historial == nullptr) {
        std::cerr << "No se puede cargar el historial: la lista es nula.\n";
        return false;
    }

    std::ifstream archivo;
    archivo.open(ruta);
    if (!archivo.is_open()) {
        std::cerr << "No se pudo abrir el archivo de historial: " << ruta << '\n';
        return false;
    }

    std::string linea;
    if (!std::getline(archivo, linea)) {
        const bool valido = !archivo.bad();
        archivo.close();
        return valido;
    }
    if (!linea.empty() && linea.back() == '\r') linea.pop_back();
    const bool formatoAntiguo = linea == FORMATO_CSV_ANTIGUO;
    if (linea != FORMATO_CSV && !formatoAntiguo) {
        std::cerr << "Encabezado CSV incorrecto en linea 1.\n";
        archivo.close();
        return false;
    }

    std::size_t numeroLinea = 1;
    while (std::getline(archivo, linea)) {
        numeroLinea++;
        if (!linea.empty() && linea.back() == '\r') linea.pop_back();
        std::string campos[36];
        HistorialCSVPtr registro = new HistorialCSV{};
        const int cantidadCampos = formatoAntiguo ? 21 : 36;
        const bool valido = separarCSV(linea, campos, cantidadCampos) &&
                    convertirRegistro(campos, cantidadCampos, registro);
        if (!valido) {
            std::cerr << "Fila mal formada en línea " << numeroLinea << "; se omite.\n";
            delete registro;
            continue;
        }
        try {
            historial->InsertarFinal(registro);
        } catch (...) {
            delete registro;
            throw;
        }
    }

    const bool errorLectura = archivo.bad();
    archivo.close();
    if (errorLectura) {
        std::cerr << "Error al leer el archivo de historial: " << ruta << '\n';
        return false;
    }
    return true;
}

bool Persistencia::guardarPartida(const std::string& ruta, JugadorPtr jugador, int idPartida, int semilla, const std::string& modo) {
    HistorialCSV registro{};
    if (!crearRegistro(jugador, idPartida, semilla, modo, registro)) {
        std::cerr << "No se puede guardar la partida: datos invalidos.\n";
        return false;
    }
    if (!migrarCSVAntiguo(ruta)) {
        std::cerr << "No se pudo migrar el historial CSV antiguo.\n";
        return false;
    }

    std::ifstream existente;
    existente.open(ruta);
    std::string encabezado;
    const bool escribirEncabezado = !std::getline(existente, encabezado);
    if (!encabezado.empty() && encabezado.back() == '\r') encabezado.pop_back();
    if (!escribirEncabezado && encabezado != FORMATO_CSV) {
        std::cerr << "No se puede agregar a un archivo con encabezado incorrecto.\n";
        existente.close();
        return false;
    }
    bool faltaSalto = false;
    if (!escribirEncabezado) {
        existente.clear();
        existente.seekg(-1, std::ios::end);
        faltaSalto = existente.get() != '\n';
    }
    existente.close();

    std::ofstream archivo;
    archivo.open(ruta, std::ios::app);
    if (!archivo.is_open()) {
        std::cerr << "No se pudo abrir el archivo de historial: " << ruta << '\n';
        return false;
    }
    if (escribirEncabezado) archivo << FORMATO_CSV << '\n';
    else if (faltaSalto) archivo << '\n';

    escribirRegistroCSV(archivo, registro);

    archivo.close();
    if (!archivo) {
        std::cerr << "Error al escribir en el archivo de historial: " << ruta << '\n';
        return false;
    }
    return true;
}

bool Persistencia::crearRegistro(JugadorPtr jugador, int idPartida, int semilla,
                                 const std::string& modo, HistorialCSV& registro) {
    if (jugador == nullptr || std::memchr(jugador->Alias, '\0', sizeof(jugador->Alias)) == nullptr ||
        modo.find_first_of("\r\n") != std::string::npos ||
        std::string(jugador->Alias).find_first_of("\r\n") != std::string::npos) return false;
    registro = HistorialCSV{};
    if (!copiarCampo(registro.modo, sizeof(registro.modo), modo) ||
        !copiarCampo(registro.jugador, sizeof(registro.jugador), jugador->Alias)) return false;
    registro.id_partida = idPartida;
    registro.semilla = semilla;
    registro.es_bot = jugador->isBot ? 1 : 0;
    registro.posicion = jugador->posicion;
    registro.puntaje = jugador->puntaje;
    registro.oleada = jugador->numero_oleada;
    registro.disparos = jugador->disparos;
    registro.correctos = jugador->correctos;
    registro.erroneos = jugador->erroneos;
    registro.combo_max = jugador->combo_max;
    registro.pulsos = jugador->pulsos;
    registro.dano_total = jugador->dano_total;
    registro.ticks = jugador->ticks;
    registro.bajas_dron = jugador->bajas_dron;
    registro.bajas_girador = jugador->bajas_girador;
    registro.bajas_divisor = jugador->bajas_divisor;
    registro.bajas_mini = jugador->bajas_mini;
    registro.bajas_tanque = jugador->bajas_tanque;
    registro.bajas_camaleon = jugador->bajas_camaleon;
    for (int color = 0; color < 3; color++) {
        registro.correctos_color[color] = jugador->correctos_color[color];
        registro.erroneos_color[color] = jugador->erroneos_color[color];
        registro.bajas_color[color] = jugador->bajas_color[color];
    }
    for (int tipo = 0; tipo < 6; tipo++) registro.dano_por_tipo[tipo] = jugador->dano_por_tipo[tipo];
    return registroValido(registro);
}

bool Persistencia::guardarPartidaCompleta(const std::string& ruta, JugadorPtr jugadores[], int cantidad,
                                          int semilla, const std::string& modo, Historial* historial) {
    if (historial == nullptr || jugadores == nullptr || cantidad < 1 || cantidad > 6) return false;
    const int id = historial->SiguienteIdPartida();
    HistorialCSV registros[6]{};
    for (int indice = 0; indice < cantidad; indice++) {
        if (!crearRegistro(jugadores[indice], id, semilla, modo, registros[indice])) return false;
        for (int anterior = 0; anterior < indice; anterior++) {
            if (std::strcmp(registros[indice].jugador, registros[anterior].jugador) == 0 ||
                registros[indice].posicion == registros[anterior].posicion) return false;
        }
        if (registros[indice].posicion > cantidad) return false;
    }

    if (!migrarCSVAntiguo(ruta)) {
        std::cerr << "No se pudo preparar el historial CSV para guardar.\n";
        return false;
    }

    const std::string temporal = ruta + ".tmp";
    std::ifstream entrada(ruta);
    std::ofstream salida(temporal, std::ios::trunc);
    if (!salida.is_open()) return false;

    std::string encabezado;
    const bool archivoConDatos = entrada.is_open() && static_cast<bool>(std::getline(entrada, encabezado));
    if (archivoConDatos) {
        if (!encabezado.empty() && encabezado.back() == '\r') encabezado.pop_back();
        if (encabezado != FORMATO_CSV) {
            entrada.close();
            salida.close();
            std::remove(temporal.c_str());
            return false;
        }
        salida << encabezado << '\n';
        std::string linea;
        while (std::getline(entrada, linea)) salida << linea << '\n';
    } else {
        salida << FORMATO_CSV << '\n';
    }
    entrada.close();
    for (int indice = 0; indice < cantidad; indice++) escribirRegistroCSV(salida, registros[indice]);
    salida.close();

    #ifdef _WIN32
        // En Windows reemplazamos el archivo anterior
        if (!salida || !MoveFileExA(temporal.c_str(), ruta.c_str(),
                                MOVEFILE_REPLACE_EXISTING)) {
    #else
        // En Linux usamos rename
        if (!salida || std::rename(temporal.c_str(), ruta.c_str()) != 0) {
    #endif
            std::remove(temporal.c_str());
            std::cerr << "No se pudo confirmar el historial completo\n";
            return false;
        }

    for (int indice = 0; indice < cantidad; indice++) {
        HistorialCSVPtr copia = new HistorialCSV(registros[indice]);
        try {
            historial->InsertarFinal(copia);
        } catch (...) {
            delete copia;
            throw;
        }
    }
    return true;
}
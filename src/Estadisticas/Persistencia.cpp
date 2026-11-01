#include "Persistencia.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

const char* const Persistencia::FORMATO_CSV =
    "id_partida,semilla,modo,jugador,es_bot,posicion,puntaje,oleada,disparos,"
    "correctos,erroneos,combo_max,pulsos,dano_total,ticks,bajas_dron,"
    "bajas_girador,bajas_divisor,bajas_mini,bajas_tanque,bajas_camaleon";

namespace {
bool separarCSV(const std::string& linea, std::string (&campos)[21]) {
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
            if (cantidad == 21) return false;
            campos[cantidad++] = campo;
            campo.clear();
            comillasCerradas = false;
        } else {
            if (comillasCerradas) return false;
            campo += caracter;
        }
    }

    if (entreComillas) return false;
    if (cantidad == 21) return false;
    campos[cantidad++] = campo;
    return cantidad == 21;
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
    return true;
}

bool convertirRegistro(const std::string (&campos)[21], HistorialCSVPtr registro) {
    if (!copiarCampo(registro->modo, sizeof(registro->modo), campos[2]) ||
        !copiarCampo(registro->jugador, sizeof(registro->jugador), campos[3])) {
        return false;
    }

    return convertirEntero(campos[0], registro->id_partida) &&
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
           convertirEntero(campos[20], registro->bajas_camaleon) && registroValido(*registro);
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
    if (linea != FORMATO_CSV) {
        std::cerr << "Encabezado CSV incorrecto en linea 1.\n";
        archivo.close();
        return false;
    }

    std::size_t numeroLinea = 1;
    while (std::getline(archivo, linea)) {
        numeroLinea++;
        if (!linea.empty() && linea.back() == '\r') linea.pop_back();
        std::string campos[21];
        HistorialCSVPtr registro = new HistorialCSV{};
        const bool valido = separarCSV(linea, campos) && convertirRegistro(campos, registro);
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

    archivo << idPartida << ',' << semilla << ',' << escaparCSV(modo) << ','
            << escaparCSV(jugador->Alias) << ',' << (jugador->isBot ? 1 : 0) << ','
            << jugador->posicion << ',' << jugador->puntaje << ',' << jugador->numero_oleada << ','
            << jugador->disparos << ',' << jugador->correctos << ',' << jugador->erroneos << ','
            << jugador->combo_max << ',' << jugador->pulsos << ',' << jugador->dano_total << ','
            << jugador->ticks << ',' << jugador->bajas_dron << ',' << jugador->bajas_girador << ','
            << jugador->bajas_divisor << ',' << jugador->bajas_mini << ',' << jugador->bajas_tanque
            << ',' << jugador->bajas_camaleon << '\n';

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
    for (int indice = 0; indice < cantidad; indice++) {
        HistorialCSVPtr copia = new HistorialCSV(registros[indice]);
        if (!guardarPartida(ruta, jugadores[indice], id, semilla, modo)) {
            delete copia;
            return false;
        }
        try {
            historial->InsertarFinal(copia);
        } catch (...) {
            delete copia;
            throw;
        }
    }
    return true;
}
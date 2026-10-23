#include "Persistencia.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <vector>

const char* const Persistencia::FORMATO_CSV =
    "id_partida,semilla,modo,jugador,es_bot,posicion,puntaje,oleada,disparos,"
    "correctos,erroneos,combo_max,pulsos,dano_total,ticks,bajas_dron,"
    "bajas_girador,bajas_divisor,bajas_mini,bajas_tanque,bajas_camaleon";

namespace {
bool separarCSV(const std::string& linea, std::vector<std::string>& campos) {
    std::string campo;
    bool entreComillas = false;

    for (std::size_t i = 0; i < linea.size(); i++) {
        const char caracter = linea[i];
        if (caracter == '"') {
            if (entreComillas && i + 1 < linea.size() && linea[i + 1] == '"') {
                campo += '"';
                i++;
            } else {
                entreComillas = !entreComillas;
            }
        } else if (caracter == ',' && !entreComillas) {
            campos.push_back(campo);
            campo.clear();
        } else {
            campo += caracter;
        }
    }

    if (entreComillas) return false;
    campos.push_back(campo);
    return true;
}

bool convertirEntero(const std::string& texto, int& valor) {
    if (texto.empty()) return false;
    errno = 0;
    char* fin = nullptr;
    const long convertido = std::strtol(texto.c_str(), &fin, 10);
    if (errno == ERANGE || fin == texto.c_str() || *fin != '\0' ||
        convertido < std::numeric_limits<int>::min() ||
        convertido > std::numeric_limits<int>::max()) return false;
    valor = static_cast<int>(convertido);
    return true;
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
    if (valor.size() >= capacidad) return false;
    std::memcpy(destino, valor.c_str(), valor.size() + 1);
    return true;
}

bool convertirRegistro(const std::vector<std::string>& campos, HistorialCSVPtr registro) {
    if (campos.size() != 21 ||
        !copiarCampo(registro->modo, sizeof(registro->modo), campos[2]) ||
        !copiarCampo(registro->jugador, sizeof(registro->jugador), campos[3])) return false;

    int* valores[] = {
        &registro->id_partida, &registro->semilla, nullptr, nullptr,
        &registro->es_bot, &registro->posicion, &registro->puntaje,
        &registro->oleada, &registro->disparos, &registro->correctos,
        &registro->erroneos, &registro->combo_max, &registro->pulsos,
        &registro->dano_total, &registro->ticks, &registro->bajas_dron,
        &registro->bajas_girador, &registro->bajas_divisor, &registro->bajas_mini,
        &registro->bajas_tanque, &registro->bajas_camaleon
    };

    for (std::size_t i = 0; i < 21; i++) {
        if (valores[i] != nullptr && !convertirEntero(campos[i], *valores[i])) return false;
    }
    return true;
}
}

bool Persistencia::cargarHistorial(const std::string& ruta, Historial* historial) {
    if (historial == nullptr) {
        std::cerr << "No se puede cargar el historial: la lista es nula.\n";
        return false;
    }

    std::ifstream archivo(ruta);
    if (!archivo.is_open()) {
        std::cerr << "No se pudo abrir el archivo de historial: " << ruta << '\n';
        return false;
    }

    std::string linea;
    if (!std::getline(archivo, linea)) return true;

    std::size_t numeroLinea = 1;
    while (std::getline(archivo, linea)) {
        numeroLinea++;
        std::vector<std::string> campos;
        HistorialCSVPtr registro = new HistorialCSV{};
        const bool valido = separarCSV(linea, campos) && convertirRegistro(campos, registro);
        if (!valido) {
            std::cerr << "Fila mal formada en línea " << numeroLinea << "; se omite.\n";
            delete registro;
            continue;
        }
        historial->InsertarFinal(registro);
    }

    if (archivo.bad()) {
        std::cerr << "Error al leer el archivo de historial: " << ruta << '\n';
        return false;
    }
    return true;
}

bool Persistencia::guardarPartida(const std::string& ruta, JugadorPtr jugador,
                                  int idPartida, int semilla, const std::string& modo) {
    if (jugador == nullptr || modo.size() >= sizeof(HistorialCSV::modo) ||
        std::memchr(jugador->Alias, '\0', sizeof(jugador->Alias)) == nullptr) {
        std::cerr << "No se puede guardar la partida: datos inválidos.\n";
        return false;
    }

    std::ifstream existente(ruta, std::ios::binary);
    const bool escribirEncabezado = !existente.is_open() || existente.peek() == std::ifstream::traits_type::eof();
    existente.close();

    std::ofstream archivo(ruta, std::ios::app);
    if (!archivo.is_open()) {
        std::cerr << "No se pudo abrir el archivo de historial: " << ruta << '\n';
        return false;
    }
    if (escribirEncabezado) archivo << FORMATO_CSV << '\n';

    archivo << idPartida << ',' << semilla << ',' << escaparCSV(modo) << ','
            << escaparCSV(jugador->Alias) << ',' << (jugador->isBot ? 1 : 0) << ','
            << jugador->posicion << ',' << jugador->puntaje << ',' << jugador->numero_oleada << ','
            << jugador->disparos << ',' << jugador->correctos << ',' << jugador->erroneos << ','
            << jugador->combo_max << ',' << jugador->pulsos << ',' << jugador->dano_total << ','
            << jugador->ticks << ',' << jugador->bajas_dron << ',' << jugador->bajas_girador << ','
            << jugador->bajas_divisor << ',' << jugador->bajas_mini << ',' << jugador->bajas_tanque
            << ',' << jugador->bajas_camaleon << '\n';

    if (!archivo) {
        std::cerr << "Error al escribir en el archivo de historial: " << ruta << '\n';
        return false;
    }
    return true;
}
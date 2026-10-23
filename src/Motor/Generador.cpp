#include "Generador.h"

std::uint32_t Generador::siguiente() {
    estado = (1103515245u * estado + 12345u) & 0x7FFFFFFFu;
    return estado;
}
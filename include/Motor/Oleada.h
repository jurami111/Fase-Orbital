#pragma once

#include <cstdint>
#include "../Estructuras/ColaSpawn.h"

class Oleada {
    public:
        ColaSpawn Generar(std::uint32_t semillaPartida, int numeroOleada) const;
};
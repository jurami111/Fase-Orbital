#pragma once

#include <cstdint>

class Generador {
    private:
        std::uint32_t estado;

    public:
        explicit Generador(std::uint32_t semilla) : estado(semilla) {}

        std::uint32_t siguiente();
};
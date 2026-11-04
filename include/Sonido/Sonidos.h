#pragma once
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <iostream>

//Cada efecto se carga una sola vez
struct Sonidos {
    ALLEGRO_SAMPLE* disparo = nullptr;
    ALLEGRO_SAMPLE* error = nullptr;
    ALLEGRO_SAMPLE* nucleo = nullptr;
    ALLEGRO_SAMPLE* pulso = nullptr;

    bool Iniciar() {
        if (!al_install_audio()) {
            std::cerr << "No se pudo iniciar el audio.\n";
            return false;
        }
        if (!al_init_acodec_addon() || !al_reserve_samples(16)) {
            std::cerr << "No se pudo preparar el audio.\n";
            return false;
        }
        disparo = al_load_sample("sonidos/disparo.wav");
        error = al_load_sample("sonidos/error.wav");
        nucleo = al_load_sample("sonidos/nucleo.wav");
        pulso = al_load_sample("sonidos/pulso.wav");
        if (!disparo || !error || !nucleo || !pulso) {
            std::cerr << "Faltan archivos WAV en la carpeta sonidos.\n";
            Liberar();
            return false;
        }
        return true;
    }

    // Reproduce cada sonido con un volumen diferente
    void Reproducir(ALLEGRO_SAMPLE* sonido) {
        if (sonido == nullptr) {
            return;
        }

        float volumen = 0.7f;

        if (sonido == disparo) {
            volumen = 0.08f; // Disparo muy bajito
        } else if (sonido == error) {
            volumen = 0.8f; // Impacto incorrecto
        } else if (sonido == nucleo) {
            volumen = 0.9f; // Daño al núcleo
        } else if (sonido == pulso) {
            volumen = 1.0f; // Activación del pulso
        }

        al_play_sample(sonido, volumen, 0.0f, 1.0f, ALLEGRO_PLAYMODE_ONCE, nullptr);
    }

    void Liberar() {
        if (disparo) al_destroy_sample(disparo);
        if (error) al_destroy_sample(error);
        if (nucleo) al_destroy_sample(nucleo);
        if (pulso) al_destroy_sample(pulso);
        disparo = error = nucleo = pulso = nullptr;
    }
};

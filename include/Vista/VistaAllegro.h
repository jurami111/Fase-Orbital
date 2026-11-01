#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include "../Estadisticas/Estadisticas.h"
#include "../Estadisticas/Ranking.h"

enum class Pantalla { Menu, Estadisticas, Ranking, VistaPrevia };

class VistaAllegro {
	ALLEGRO_DISPLAY* ventana = nullptr;
	ALLEGRO_EVENT_QUEUE* eventos = nullptr;
	ALLEGRO_TIMER* temporizador = nullptr;
	ALLEGRO_FONT* fuente = nullptr;
	bool recursosPropios = true;
	Pantalla pantalla = Pantalla::Menu;
	Historial* historial = nullptr;
	Ranking* ranking = nullptr;
	Estadisticas estadisticas;
	NodoRegistroPtr seleccionado = nullptr;
	ResumenHistorico resumen{};
	ResultadoHistograma histograma;
	bool salir = false;
	bool izquierda = false;
	bool derecha = false;
	int ticks = 0;
	double angulo = 270.0;

	void LiberarRecursos();
	void CambiarPantalla(Pantalla nueva);
	void CambiarJugador(bool siguiente);
	void ActualizarResumen();
	void ProcesarEvento(const ALLEGRO_EVENT& evento);
	void Dibujar(const std::string& aviso);
	void DibujarMenu();
	void DibujarEstadisticas();
	void DibujarRanking();
	void DibujarVistaPrevia();

public:
	VistaAllegro() = default;
	~VistaAllegro();
	VistaAllegro(const VistaAllegro&) = delete;
	VistaAllegro& operator=(const VistaAllegro&) = delete;
	bool Ejecutar(Historial& datos, Ranking& tabla, const std::string& aviso = "",
				  const std::string& carpetaCapturas = "");
	bool MostrarReporte(ALLEGRO_DISPLAY* ventanaPrincipal, ALLEGRO_EVENT_QUEUE* colaPrincipal,
						Historial& datos, Ranking& tabla, Pantalla inicial,
						const std::string& aviso = "", const std::string& rutaCaptura = "");
};

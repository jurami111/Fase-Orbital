#include "VistaAllegro.h"
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace {
ALLEGRO_COLOR textoColor() { return al_map_rgb(235, 242, 246); }
ALLEGRO_COLOR acento() { return al_map_rgb(110, 217, 241); }
ALLEGRO_COLOR fondo() { return al_map_rgb(20, 17, 43); }
ALLEGRO_COLOR borde() { return al_map_rgb(58, 52, 90); }
ALLEGRO_COLOR cian() { return al_map_rgb(68, 210, 225); }
ALLEGRO_COLOR magenta() { return al_map_rgb(238, 91, 176); }
ALLEGRO_COLOR ambar() { return al_map_rgb(245, 183, 73); }
ALLEGRO_COLOR panel() { return al_map_rgb(33, 29, 62); }
ALLEGRO_COLOR panelAlterno() { return al_map_rgb(42, 36, 72); }

void texto(ALLEGRO_FONT* fuente, float x, float y, const std::string& contenido, float escala = 1.5f) {
	const int ancho = al_get_text_width(fuente, contenido.c_str());
	if (ancho > 0 && ancho * escala > 760 - x) escala = (760 - x) / ancho;
	ALLEGRO_TRANSFORM transformacion;
	al_identity_transform(&transformacion);
	al_scale_transform(&transformacion, escala, escala);
	al_use_transform(&transformacion);
	al_draw_text(fuente, textoColor(), x / escala, y / escala, 0, contenido.c_str());
	al_identity_transform(&transformacion);
	al_use_transform(&transformacion);
}

std::string decimal(double valor) {
	std::ostringstream salida;
	salida << std::fixed << std::setprecision(2) << valor;
	return salida.str();
}

bool dentro(float mouseX, float mouseY, float x, float y, float ancho, float alto) {
	return mouseX >= x && mouseX <= x + ancho && mouseY >= y && mouseY <= y + alto;
}

void boton(ALLEGRO_FONT* fuente, float x, float y, float ancho, const std::string& titulo) {
	al_draw_filled_rectangle(x, y, x + ancho, y + 36, panel());
	al_draw_rectangle(x, y, x + ancho, y + 36, acento(), 1);
	const float textoX = x + (ancho - al_get_text_width(fuente, titulo.c_str()) * 1.5f) / 2;
	texto(fuente, textoX, y + 12, titulo);
}
}

VistaAllegro::~VistaAllegro() { LiberarRecursos(); }

void VistaAllegro::LiberarRecursos() {
	if (recursosPropios && eventos != nullptr) al_destroy_event_queue(eventos);
	if (recursosPropios && temporizador != nullptr) al_destroy_timer(temporizador);
	if (fuente != nullptr) al_destroy_font(fuente);
	if (recursosPropios && ventana != nullptr) al_destroy_display(ventana);
	eventos = nullptr;
	temporizador = nullptr;
	fuente = nullptr;
	ventana = nullptr;
}

bool VistaAllegro::Ejecutar(Historial& datos, Ranking& tabla, const std::string& aviso,
						  const std::string& carpetaCapturas) {
	recursosPropios = true;
	if (!al_init() || !al_install_keyboard() || !al_install_mouse() || !al_init_primitives_addon()) {
		std::cerr << "No se pudo inicializar Allegro, teclado, mouse o primitivas.\n";
		return false;
	}
	al_init_font_addon();
	if (!carpetaCapturas.empty() && !al_init_image_addon()) {
		std::cerr << "No se pudo inicializar el addon de imagen.\n";
		return false;
	}
	ventana = al_create_display(800, 600);
	eventos = al_create_event_queue();
	temporizador = al_create_timer(1.0 / 30.0);
	fuente = al_create_builtin_font();
	if (ventana == nullptr || eventos == nullptr || temporizador == nullptr || fuente == nullptr) {
		std::cerr << "No se pudo crear ventana, cola, temporizador o fuente.\n";
		LiberarRecursos();
		return false;
	}
	al_set_window_title(ventana, "Fase Orbital");
	al_register_event_source(eventos, al_get_display_event_source(ventana));
	al_register_event_source(eventos, al_get_keyboard_event_source());
	al_register_event_source(eventos, al_get_mouse_event_source());
	al_register_event_source(eventos, al_get_timer_event_source(temporizador));
	historial = &datos;
	ranking = &tabla;
	seleccionado = ranking->primero();
	salir = false;
	CambiarPantalla(Pantalla::Menu);
	al_start_timer(temporizador);
	bool redibujar = true;
	int capturas = 0;
	bool correcto = true;
	while (!salir) {
		ALLEGRO_EVENT evento;
		al_wait_for_event(eventos, &evento);
		ProcesarEvento(evento);
		redibujar = true;
		if (redibujar && al_is_event_queue_empty(eventos) && !salir) {
			Dibujar(aviso);
			if (!carpetaCapturas.empty()) {
				const char* nombres[] = {"menu.png", "estadisticas.png", "ranking.png", "vista-previa.png"};
				const std::string ruta = carpetaCapturas + "/" + nombres[capturas];
				correcto = al_save_bitmap(ruta.c_str(), al_get_backbuffer(ventana));
				if (!correcto) {
					std::cerr << "No se pudo guardar la captura: " << ruta << '\n';
					salir = true;
				}
				capturas++;
				ALLEGRO_EVENT prueba{};
				prueba.type = ALLEGRO_EVENT_MOUSE_BUTTON_DOWN;
				prueba.mouse.button = 1;
				if (capturas == 1) {
					prueba.mouse.x = 300;
					prueba.mouse.y = 240;
					ProcesarEvento(prueba);
					correcto = correcto && pantalla == Pantalla::Estadisticas;
				} else if (capturas == 2 || capturas == 3) {
					if (capturas == 2) {
						NodoRegistroPtr original = seleccionado;
						prueba.type = ALLEGRO_EVENT_KEY_DOWN;
						prueba.keyboard.keycode = ALLEGRO_KEY_RIGHT;
						ProcesarEvento(prueba);
						prueba.keyboard.keycode = ALLEGRO_KEY_LEFT;
						ProcesarEvento(prueba);
						correcto = correcto && seleccionado == original;
					} else if (!ranking->isVacia()) {
						prueba.mouse.x = 30;
						prueba.mouse.y = 170;
						ProcesarEvento(prueba);
						correcto = correcto && pantalla == Pantalla::Estadisticas && seleccionado == ranking->primero();
					}
					prueba = ALLEGRO_EVENT{};
					prueba.type = ALLEGRO_EVENT_MOUSE_BUTTON_DOWN;
					prueba.mouse.button = 1;
					prueba.mouse.x = 40;
					prueba.mouse.y = 550;
					ProcesarEvento(prueba);
					correcto = correcto && pantalla == Pantalla::Menu;
					prueba.mouse.x = 300;
					prueba.mouse.y = capturas == 2 ? 290 : 340;
					ProcesarEvento(prueba);
					correcto = correcto && pantalla == (capturas == 2 ? Pantalla::Ranking : Pantalla::VistaPrevia);
				} else {
					const double original = angulo;
					prueba.type = ALLEGRO_EVENT_KEY_DOWN;
					prueba.keyboard.keycode = ALLEGRO_KEY_LEFT;
					ProcesarEvento(prueba);
					prueba.type = ALLEGRO_EVENT_TIMER;
					ProcesarEvento(prueba);
					correcto = correcto && angulo == std::fmod(original + 6.0, 360.0);
					salir = true;
				}
				if (!correcto) {
					std::cerr << "Fallo al comprobar captura o navegacion.\n";
					salir = true;
				}
			}
			al_flip_display();
			redibujar = false;
		}
	}
	LiberarRecursos();
	return correcto;
}

bool VistaAllegro::MostrarReporte(ALLEGRO_DISPLAY* ventanaPrincipal, ALLEGRO_EVENT_QUEUE* colaPrincipal,
								 Historial& datos, Ranking& tabla, Pantalla inicial,
								 const std::string& aviso, const std::string& rutaCaptura) {
	if (ventanaPrincipal == nullptr || colaPrincipal == nullptr) return false;
	if (inicial != Pantalla::Estadisticas && inicial != Pantalla::Ranking) return false;
	recursosPropios = false;
	ventana = ventanaPrincipal;
	eventos = colaPrincipal;
	fuente = al_create_builtin_font();
	if (fuente == nullptr) {
		std::cerr << "No se pudo crear la fuente del reporte.\n";
		LiberarRecursos();
		return false;
	}
	historial = &datos;
	ranking = &tabla;
	seleccionado = ranking->primero();
	salir = false;
	CambiarPantalla(inicial);
	bool correcto = true;
	while (!salir && pantalla != Pantalla::Menu) {
		ALLEGRO_EVENT evento;
		al_wait_for_event(eventos, &evento);
		ProcesarEvento(evento);
		if (!salir && pantalla != Pantalla::Menu && al_is_event_queue_empty(eventos)) {
			Dibujar(aviso);
			if (!rutaCaptura.empty()) {
				correcto = al_save_bitmap(rutaCaptura.c_str(), al_get_backbuffer(ventana));
				ALLEGRO_EVENT volver{};
				volver.type = ALLEGRO_EVENT_MOUSE_BUTTON_DOWN;
				volver.mouse.button = 1;
				volver.mouse.x = 40;
				volver.mouse.y = 550;
				ProcesarEvento(volver);
				correcto = correcto && pantalla == Pantalla::Menu;
			}
			al_flip_display();
		}
	}
	const bool continuar = !salir && correcto;
	LiberarRecursos();
	return continuar;
}

void VistaAllegro::ActualizarResumen() {
	resumen = ResumenHistorico{};
	histograma = ResultadoHistograma{};
	if (seleccionado == nullptr) return;
	resumen = estadisticas.calcularResumenHistorico(historial, seleccionado->dato->Alias);
	histograma = estadisticas.calcularHistograma(historial, seleccionado->dato->Alias);
}

void VistaAllegro::CambiarPantalla(Pantalla nueva) {
	pantalla = nueva;
	izquierda = false;
	derecha = false;
	if (pantalla == Pantalla::Estadisticas) ActualizarResumen();
}

void VistaAllegro::CambiarJugador(bool siguiente) {
	if (seleccionado == nullptr) return;
	if (siguiente) {
		seleccionado = seleccionado->siguiente == nullptr ? ranking->primero() : seleccionado->siguiente;
	} else {
		NodoRegistroPtr anterior = nullptr;
		for (NodoRegistroPtr nodo = ranking->primero(); nodo != seleccionado; nodo = nodo->siguiente) anterior = nodo;
		seleccionado = anterior == nullptr ? ranking->ultimo() : anterior;
	}
	ActualizarResumen();
}

void VistaAllegro::ProcesarEvento(const ALLEGRO_EVENT& evento) {
	if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) salir = true;
	if (evento.type == ALLEGRO_EVENT_TIMER) {
		ticks++;
		if (pantalla == Pantalla::VistaPrevia) {
			if (izquierda) angulo += 6.0;
			if (derecha) angulo -= 6.0;
			angulo = std::fmod(angulo + 360.0, 360.0);
		}
	}
	if (evento.type == ALLEGRO_EVENT_KEY_DOWN) {
		const int tecla = evento.keyboard.keycode;
		if (tecla == ALLEGRO_KEY_ESCAPE) {
			if (pantalla == Pantalla::Menu) salir = true;
			else CambiarPantalla(Pantalla::Menu);
		}
		if (pantalla == Pantalla::Menu) {
			if (tecla == ALLEGRO_KEY_E) CambiarPantalla(Pantalla::Estadisticas);
			if (tecla == ALLEGRO_KEY_R) CambiarPantalla(Pantalla::Ranking);
			if (tecla == ALLEGRO_KEY_ENTER) CambiarPantalla(Pantalla::VistaPrevia);
		} else if (pantalla == Pantalla::Estadisticas) {
			if (tecla == ALLEGRO_KEY_LEFT) CambiarJugador(false);
			if (tecla == ALLEGRO_KEY_RIGHT) CambiarJugador(true);
		} else if (pantalla == Pantalla::VistaPrevia) {
			if (tecla == ALLEGRO_KEY_LEFT) izquierda = true;
			if (tecla == ALLEGRO_KEY_RIGHT) derecha = true;
		}
	}
	if (evento.type == ALLEGRO_EVENT_KEY_UP) {
		if (evento.keyboard.keycode == ALLEGRO_KEY_LEFT) izquierda = false;
		if (evento.keyboard.keycode == ALLEGRO_KEY_RIGHT) derecha = false;
	}
	if (evento.type != ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || evento.mouse.button != 1) return;
	const float x = evento.mouse.x;
	const float y = evento.mouse.y;
	if (pantalla == Pantalla::Menu) {
		if (dentro(x, y, 280, 230, 240, 36)) CambiarPantalla(Pantalla::Estadisticas);
		else if (dentro(x, y, 280, 282, 240, 36)) CambiarPantalla(Pantalla::Ranking);
		else if (dentro(x, y, 280, 334, 240, 36)) CambiarPantalla(Pantalla::VistaPrevia);
		else if (dentro(x, y, 280, 386, 240, 36)) salir = true;
	} else {
		if (dentro(x, y, 24, 540, 120, 36)) CambiarPantalla(Pantalla::Menu);
		else if (pantalla == Pantalla::Estadisticas) {
			if (dentro(x, y, 610, 96, 50, 36)) CambiarJugador(false);
			if (dentro(x, y, 674, 96, 50, 36)) CambiarJugador(true);
		} else if (pantalla == Pantalla::Ranking) {
			NodoRegistroPtr nodo = ranking->primero();
			for (int posicion = 0; nodo != nullptr && posicion < 10; posicion++, nodo = nodo->siguiente) {
				if (dentro(x, y, 24, 158 + posicion * 30, 752, 28)) {
					seleccionado = nodo;
					CambiarPantalla(Pantalla::Estadisticas);
					break;
				}
			}
		}
	}
}

void VistaAllegro::Dibujar(const std::string& aviso) {

    // Pintamos el fondo oscuro de la pantalla
    // Ya no ponemos circulos de adorno ni lineas
    al_clear_to_color(fondo());

    // Revisamos en cual pantalla estamos para dibujarla
    if (pantalla == Pantalla::Menu)
        DibujarMenu();

    if (pantalla == Pantalla::Estadisticas)
        DibujarEstadisticas();

    if (pantalla == Pantalla::Ranking)
        DibujarRanking();

    if (pantalla == Pantalla::VistaPrevia)
        DibujarVistaPrevia();

    // Ponemos el boton para regresar al menu
    if (pantalla != Pantalla::Menu)
        boton(fuente, 24, 540, 120, "Volver");

    // Mostramos algun mensaje si hace falta
    if (!aviso.empty())
        texto(fuente, 170, 552, aviso, 1.0f);
}

void VistaAllegro::DibujarMenu() {
	// Menu con los mismos colores del juego
	texto(fuente, 256, 70, "FASE ORBITAL", 3.0f);
	al_draw_circle(400, 165, 30, borde(), 2);
	al_draw_filled_circle(400, 165, 10, cian());
	boton(fuente, 280, 230, 240, "Estadisticas");
	boton(fuente, 280, 282, 240, "Ranking");
	boton(fuente, 280, 334, 240, "Vista previa");
	boton(fuente, 280, 386, 240, "Salir");
	texto(fuente, 280, 468, "Registros: " + std::to_string(historial->tamano()));
}

void VistaAllegro::DibujarEstadisticas() {
	// Titulo grande y las estadisticas del jugador
	texto(fuente, 40, 32, "ESTADISTICAS", 3.0f);
	if (seleccionado == nullptr) {
		texto(fuente, 40, 150, "Sin partidas registradas");
		return;
	}
	texto(fuente, 40, 104, resumen.alias);
	boton(fuente, 610, 96, 50, "<");
	boton(fuente, 674, 96, 50, ">");
	texto(fuente, 40, 158, "Partidas: " + std::to_string(resumen.partidas_jugadas) +
		  "    Victorias: " + std::to_string(resumen.cantidad_victorias));
	texto(fuente, 40, 188, "Mejor: " + std::to_string(resumen.mejor_puntaje) +
		  "    Peor: " + std::to_string(resumen.peor_puntaje) + "    Oleada: " + std::to_string(resumen.oleada_maxima));
	texto(fuente, 40, 218, "Media: " + decimal(resumen.media) + "    Mediana: " + decimal(resumen.mediana));
	texto(fuente, 40, 248, "Desviacion: " + decimal(resumen.desviacion_estandar) +
		  "    Precision: " + decimal(resumen.precision_promedio) + "%");
	texto(fuente, 40, 298, "Puntajes");
	int mayor = 1;
	for (int frecuencia : histograma.frecuencias) if (frecuencia > mayor) mayor = frecuencia;
	al_draw_line(40, 480, 760, 480, borde(), 1);
	for (int indice = 0; indice < 10; indice++) {
		const float x = 48 + indice * 71;
		const float alto = 135.0f * histograma.frecuencias[indice] / mayor;
		const ALLEGRO_COLOR coloresBarras[] = {cian(), magenta(), ambar()};
		al_draw_filled_rectangle(x, 480 - alto, x + 48, 480, coloresBarras[indice % 3]);
		texto(fuente, x, 462 - alto, std::to_string(histograma.frecuencias[indice]), 1.0f);
	}
	texto(fuente, 40, 500, std::to_string(histograma.minimo) + " - " + std::to_string(histograma.maximo));
}

void VistaAllegro::DibujarRanking() {
	// Tabla simple, no tocamos los datos del ranking
	texto(fuente, 40, 32, "TOP 10 HISTORICO", 2.7f);
	texto(fuente, 40, 88, "Mejores puntajes");
	if (ranking->isVacia()) {
		texto(fuente, 40, 150, "Sin partidas registradas");
		return;
	}
	al_draw_filled_rectangle(24, 117, 776, 153, panel());
	texto(fuente, 40, 124, "Pos.");
	texto(fuente, 110, 124, "Jugador");
	texto(fuente, 440, 124, "Mejor puntaje");
	texto(fuente, 640, 124, "Precision");
	NodoRegistroPtr nodo = ranking->primero();
	for (int posicion = 0; nodo != nullptr && posicion < 10; posicion++, nodo = nodo->siguiente) {
		const float y = 164 + posicion * 30;
		al_draw_filled_rectangle(24, y - 5, 776, y + 23,
			posicion % 2 == 0 ? panel() : panelAlterno());
		texto(fuente, 40, y, std::to_string(posicion + 1));
		texto(fuente, 110, y, nodo->dato->Alias);
		texto(fuente, 440, y, std::to_string(nodo->dato->puntaje));
		texto(fuente, 640, y, decimal(nodo->dato->presicion) + "%");
		al_draw_line(24, y + 22, 776, y + 22, panel(), 1);
	}
}

void VistaAllegro::DibujarVistaPrevia() {
	texto(fuente, 40, 32, "Vista previa", 3.0f);
	al_draw_circle(400, 300, 100, borde(), 2);
	al_draw_filled_circle(400, 300, 20, cian());
	const double radianes = angulo * 3.14159265358979323846 / 180.0;
	const float naveX = 400 + 100 * std::cos(radianes);
	const float naveY = 300 - 100 * std::sin(radianes);
	al_draw_filled_circle(naveX, naveY, 8, acento());
	for (int indice = 0; indice < 6; indice++) {
		const double theta = (indice * 60.0 + ticks * 0.3) * 3.14159265358979323846 / 180.0;
		const float x = 400 + 210 * std::cos(theta);
		const float y = 300 - 210 * std::sin(theta);
		const ALLEGRO_COLOR colores[] = {cian(), magenta(), ambar()};
		const ALLEGRO_COLOR color = colores[indice % 3];
		if (indice == 0) al_draw_filled_circle(x, y, 6, color);
		if (indice == 1) { al_draw_circle(x, y, 8, color, 2); al_draw_line(x - 10, y, x + 10, y, color, 2); }
		if (indice == 2) al_draw_filled_triangle(x, y - 9, x - 8, y + 8, x + 8, y + 8, color);
		if (indice == 3) al_draw_filled_circle(x, y, 4, color);
		if (indice == 4) al_draw_filled_rectangle(x - 10, y - 10, x + 10, y + 10, color);
		if (indice == 5) al_draw_filled_ellipse(x, y, 10, 6, color);
	}
}

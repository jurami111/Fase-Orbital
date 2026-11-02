CXX = c++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic
INCLUDES = -Iinclude -Iinclude/Estructuras -Iinclude/Motor -Iinclude/Estadisticas -Iinclude/Vista
CORE = $(wildcard src/Estructuras/*.cpp src/Motor/*.cpp src/Estadisticas/*.cpp) src/Vista/Controlador.cpp
HEADERS = $(wildcard include/Estructuras/*.h include/Motor/*.h include/Estadisticas/*.h include/Vista/*.h)
ALLEGRO = allegro-5 allegro_font-5 allegro_ttf-5 allegro_primitives-5 allegro_image-5
ifeq ($(shell uname -s),Darwin)
ALLEGRO += allegro_main-5
endif

.PHONY: all consola pruebas ejecutar

all: build/faseorbital

build:
	mkdir -p build

build/faseorbital: src/main.cpp src/Vista/VistaAllegro.cpp $(CORE) $(HEADERS) Makefile | build
	$(CXX) $(CXXFLAGS) $(INCLUDES) src/main.cpp src/Vista/VistaAllegro.cpp $(CORE) -o $@ $$(pkg-config --cflags --libs $(ALLEGRO))

consola: build/faseorbital-consola

build/faseorbital-consola: src/main.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CXXFLAGS) $(INCLUDES) -DFASEORBITAL_CONSOLA src/main.cpp $(CORE) -o $@

build/pruebas: tests/main_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CXXFLAGS) $(INCLUDES) tests/main_tests.cpp $(CORE) -o $@

pruebas: build/pruebas
	directorio=$$(mktemp -d); ./build/pruebas "$$directorio/historial.csv"

ejecutar: build/faseorbital
	./build/faseorbital $(ARGS)
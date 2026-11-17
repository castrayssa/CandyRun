# Compila o CandyRun no Windows, Linux ou Mac.
#
#   cmake -B build
#   cmake --build build
#
# Na primeira vez o CMake baixa e compila o SFML 3.1.0 sozinho (demora alguns
# minutos), com o mesmo compilador do jogo. Nao precisa instalar o SFML.
# O candyrun.exe fica na pasta do projeto, ao lado de assets/.

cmake_minimum_required(VERSION 3.28)
project(CandyRun LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

# SFML baixado do GitHub e compilado junto, como biblioteca estatica:
# o jogo nao precisa das DLLs do SFML do lado.
include(FetchContent)
set(BUILD_SHARED_LIBS OFF)
set(SFML_BUILD_NETWORK OFF)     # o jogo nao usa internet (e assim nao baixa o mbedtls)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.1.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

# o jogo
file(GLOB FONTES CONFIGURE_DEPENDS src/candyrun/*.cpp)
add_executable(candyrun ${FONTES})
target_compile_features(candyrun PRIVATE cxx_std_20)
target_link_libraries(candyrun PRIVATE SFML::Graphics SFML::Audio)
if(MSVC)
    # o compilador do Visual Studio so le os acentos ("Algodão", "Ímã") certo assim
    target_compile_options(candyrun PRIVATE /utf-8)
endif()

if(WIN32)
    target_sources(candyrun PRIVATE src/candyrun/icone.rc)   # icone do .exe
    # o icone.rc fala de "assets/icone.ico": procura a partir da pasta do projeto
    set_source_files_properties(src/candyrun/icone.rc PROPERTIES INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}")
    target_link_libraries(candyrun PRIVATE dwmapi)            # cantos retos da janela
endif()
if(MINGW)
    # junta as bibliotecas do proprio GCC no .exe: roda em PC sem o MinGW
    target_link_options(candyrun PRIVATE -static)
endif()

# o .exe vai pra pasta do projeto, porque o jogo procura assets/ a partir dela
set_target_properties(candyrun PROPERTIES RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_SOURCE_DIR}>")

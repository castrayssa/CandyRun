//personaliza a janela
#include "janela_sistema.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <dwmapi.h>

void prepararJanela(sf::WindowHandle janela) {
    // sem isso, clicar no icone da barra de tarefas nao minimiza a janela
    LONG estilo = GetWindowLongW(janela, GWL_STYLE);
    SetWindowLongW(janela, GWL_STYLE, estilo | WS_MINIMIZEBOX | WS_SYSMENU);

    // deixa tudo quadrado
    const DWORD DWMWA_CANTOS = 33;       
    const DWORD NAO_ARREDONDAR = 1;         
    DwmSetWindowAttribute(janela, DWMWA_CANTOS, &NAO_ARREDONDAR, sizeof NAO_ARREDONDAR);
}

void minimizarJanela(sf::WindowHandle janela) { ShowWindow(janela, SW_MINIMIZE); }

#else

void prepararJanela(sf::WindowHandle) {}
void minimizarJanela(sf::WindowHandle) {}

#endif

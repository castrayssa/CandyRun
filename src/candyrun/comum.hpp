// Coisas usadas por todos os arquivos
#pragma once

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <random>
#include <string>

//tamanhos
const int TILE = 40;            // lado de cada casa do labirinto, em pixels
const int HUD_ALTURA = 72;      // faixa de cima
const int BARRA_ALTURA = 34;    // barra de titulo desenhada pelo jogo (logo, minimizar, fechar)
const int NUM_FASES = 3;

// formato das sprite sheets geradas por tools/fatiar_sprites.py
const int CELULA_L = 160;
const int CELULA_A = 144;
const int CELULA_MARGEM = 4;  
const float ESCALA_PERSONAGEM = 0.40f;

//direcoes
enum Direcao { NENHUMA = -1, BAIXO = 0, ESQUERDA = 1, DIREITA = 2, CIMA = 3 };

inline int dxDe(Direcao d) { return d == ESQUERDA ? -1 : (d == DIREITA ? 1 : 0); }
inline int dyDe(Direcao d) { return d == CIMA ? -1 : (d == BAIXO ? 1 : 0); }

inline Direcao oposta(Direcao d) {
    switch (d) {
        case BAIXO:    return CIMA;
        case CIMA:     return BAIXO;
        case ESQUERDA: return DIREITA;
        case DIREITA:  return ESQUERDA;
        default:       return NENHUMA;
    }
}

inline sf::Vector2i passo(sf::Vector2i casa, Direcao d) {
    return {casa.x + dxDe(d), casa.y + dyDe(d)};
}

// utilidades
// converte texto do codigo-fonte (UTF-8) pro formato do SFML
inline sf::String texto(const std::string& s) {
    return sf::String::fromUtf8(s.begin(), s.end());
}

inline std::mt19937& gerador() {
    static std::mt19937 g(std::random_device{}());
    return g;
}
inline float sortear(float a, float b) {
    return std::uniform_real_distribution<float>(a, b)(gerador());
}
inline int sortearInt(int a, int b) {   // inclui a e b
    return std::uniform_int_distribution<int>(a, b)(gerador());
}

inline float distancia(sf::Vector2f a, sf::Vector2f b) {
    return std::hypot(a.x - b.x, a.y - b.y);
}


// idioma escolhido
inline bool emIngles = false;
inline std::string T(const std::string& pt, const std::string& en) { return emIngles ? en : pt; }

// mistura duas cores (t = 0 -> a, t = 1 -> b)
inline sf::Color misturar(sf::Color a, sf::Color b, float t) {
    auto m = [t](std::uint8_t x, std::uint8_t y) {
        return std::uint8_t(x + (y - x) * t);
    };
    return sf::Color(m(a.r, b.r), m(a.g, b.g), m(a.b, b.b), m(a.a, b.a));
}

// A fonte pixel nao tem letras acentuadas
inline sf::String adaptarParaFonte(const sf::Font& fonte, const std::string& s) {
    static const std::u32string comAcento = U"áàâãäéèêëíìîïóòôõöúùûüçÁÀÂÃÄÉÈÊËÍÌÎÏÓÒÔÕÖÚÙÛÜÇ";
    static const std::u32string semAcento = U"aaaaaeeeeiiiiooooouuuucAAAAAEEEEIIIIOOOOOUUUUC";
    sf::String r = texto(s);
    for (std::size_t i = 0; i < r.getSize(); i++) {
        char32_t c = r[i];
        if (fonte.hasGlyph(c)) continue;
        std::size_t pos = comAcento.find(c);
        if (pos != std::u32string::npos) r[i] = semAcento[pos];
    }
    return r;
}

// cor do arco-iris
inline sf::Color corArcoIris(float h) {
    h = (h - std::floor(h)) * 6.f;
    float x = 1.f - std::abs(std::fmod(h, 2.f) - 1.f);
    float r = 0, g = 0, b = 0;
    if (h < 1) { r = 1; g = x; }
    else if (h < 2) { r = x; g = 1; }
    else if (h < 3) { g = 1; b = x; }
    else if (h < 4) { g = x; b = 1; }
    else if (h < 5) { r = x; b = 1; }
    else { r = 1; b = x; }
    return sf::Color(std::uint8_t(255 * r), std::uint8_t(255 * g), std::uint8_t(255 * b));
}

// texto centralizado (usado em todas as telas)
inline void escreverCentro(sf::RenderTarget& alvo, const sf::Font& fonte, const std::string& s,
                           unsigned tamanho, sf::Vector2f centro, sf::Color cor,
                           float escala = 1.f, sf::Color contorno = sf::Color(40, 20, 60)) {
    sf::Text t(fonte, adaptarParaFonte(fonte, s), tamanho);
    t.setFillColor(cor);
    t.setOutlineColor(contorno);
    t.setOutlineThickness(3.f);
    sf::FloatRect caixa = t.getLocalBounds();
    t.setOrigin({caixa.position.x + caixa.size.x / 2.f, caixa.position.y + caixa.size.y / 2.f});
    t.setPosition(centro);
    t.setScale({escala, escala});
    alvo.draw(t);
}

#pragma once

#include "comum.hpp"
#include <string>
#include <vector>

struct Mapa {
    std::vector<std::string> grade;   
    std::vector<std::string> doces;   
    int largura = 0, altura = 0;

    sf::Vector2i inicioJogadora, posChave, posGatinho, posPortaGaiola, posPortaCasa;
    std::vector<sf::Vector2i> vagasCasa;   

    bool carregar(const std::string& arquivo);

    char em(int x, int y) const;
    char em(sf::Vector2i p) const { return em(p.x, p.y); }
    sf::Vector2i normalizar(sf::Vector2i p) const;   // aplica o tunel

    bool livreParaJogadora(sf::Vector2i p, bool gaiolaAberta) const;
    bool livreParaRival(sf::Vector2i p, bool podeUsarPortaDaCasa) const;

    sf::Vector2f centro(sf::Vector2i p) const;
    sf::Vector2f centro(float x, float y) const;

    int contarDoces() const;

    void desenharLabirinto(sf::RenderTarget& alvo, sf::Color corTopo, sf::Color corLado,
                           sf::Color corChao) const;
};

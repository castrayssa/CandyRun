//  particulas, textos que sobem e tela tremendo
#pragma once

#include "comum.hpp"
#include <vector>

struct Particula {
    sf::Vector2f pos, vel;
    sf::Color cor;
    float vida, vidaMax, tamanho;
    bool coracao;           
};

struct TextoFlutuante {
    sf::Vector2f pos;
    std::string conteudo;
    sf::Color cor;
    float vida;
};

struct Efeitos {
    std::vector<Particula> particulas;
    std::vector<TextoFlutuante> textos;
    float tremor = 0;      

    void explosao(sf::Vector2f pos, int quantidade, sf::Color cor, float forca, bool coracoes = false);
    void anel(sf::Vector2f pos, int quantidade, sf::Color cor, float raio);
    void textoSubindo(sf::Vector2f pos, const std::string& s, sf::Color cor);
    void tremer(float intensidade) { if (intensidade > tremor) tremor = intensidade; }

    void atualizar(float dt);
    void desenhar(sf::RenderTarget& alvo, const sf::Font& fonte) const;
    sf::Vector2f deslocamentoTremor() const;
    void limpar();
};

// Desenhos simples usados em varios lugares
void desenharCoracao(sf::RenderTarget& alvo, sf::Vector2f centro, float tamanho, sf::Color cor);
void desenharEstrela(sf::RenderTarget& alvo, sf::Vector2f centro, float raio, sf::Color cor, float giro);

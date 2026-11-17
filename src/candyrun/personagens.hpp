#pragma once
#include "comum.hpp"
#include "mapa.hpp"

struct Andarilho {
    sf::Vector2i casa;
    sf::Vector2i proxima;        // pode ficar fora do mapa no tunel 
    Direcao dir = BAIXO;
    float progresso = 0;
    bool andando = false;
    float distanciaAndada = 0;   // serve pra trocar o quadro da animacao

    void colocarEm(sf::Vector2i c, Direcao d);
    void andarPara(Direcao d);   
    void inverter();  
    bool avancar(float velocidade, float dt, const Mapa& mapa);
    sf::Vector2f posicao(const Mapa& mapa) const;   // centro em pixels
    int quadroAnimacao() const; 
};

struct Jogadora : Andarilho {
    Direcao desejada = NENHUMA;  // ultima seta apertada
};

enum class Personalidade { PERSEGUE, EMBOSCA, CERCA, TIMIDA };
enum class EstadoRival { EM_CASA, SAINDO, ATIVA, VOLTANDO };

struct Rival : Andarilho {
    std::string nome;
    std::string descricao;
    int sprite = 0;              
    sf::Color cor; //cor da sombra
    Personalidade jeito = Personalidade::PERSEGUE;
    sf::Vector2i canto;

    EstadoRival estado = EstadoRival::EM_CASA;
    bool assustada = false;
    float tempoParaSair = 0;
    sf::Vector2i vagaNaCasa;
};

Direcao escolherDirecao(const Rival& r, sf::Vector2i alvo, const Mapa& mapa, bool aleatoria);


void desenharPersonagem(sf::RenderTarget& alvo, const sf::Texture& sheet, sf::Vector2f pe,
                        Direcao olhando, int quadro, sf::Color tinta = sf::Color::White,
                        float escala = 1.f, float giro = 0.f);

void desenharSombra(sf::RenderTarget& alvo, sf::Vector2f pe, sf::Color cor, float escala = 1.f);

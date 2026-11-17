//telas, regras, pontuacao, fases e desenho de tudo.
#pragma once

#include "comum.hpp"
#include "efeitos.hpp"
#include "mapa.hpp"
#include "personagens.hpp"
#include "sons.hpp"
#include <array>
#include <optional>

//  sprite sheets dos animais (era 3 gatos)
enum AnimGato { GATO_CORRENDO = 0, GATO_SENTADO = 1, GATO_DORMINDO = 2 };

// pirulitos
enum Pirulito { PIRULITO_ARCO_IRIS = 0, PIRULITO_ROSA = 1, PIRULITO_VERMELHO = 2 };

const int SPRITE_JOGADORA = 4;   

enum class Tela { MENU, PRONTO, JOGANDO, PAUSA, MORRENDO, FASE_COMPLETA, VITORIA, FIM_DE_JOGO };

// o que muda de uma fase pra outra
struct Dificuldade {
    float velJogadora;    
    float velRival;
    float velAssustada;
    float tempoPoder;      
    float sairDaCasa[4];    
    float modos[7];       
    sf::Color paredeTopo, paredeLado, chao, ceuCima, ceuBaixo;
};

struct EstrelaDeFundo {
    sf::Vector2f pos;
    float fase, tamanho;
};

struct Jogo {
    // recursos
    sf::RenderWindow janela;
    sf::Vector2f tamJogo;             // area do jogo, sem a barra de titulo
    bool arrastando = false;          // segurando a barra de titulo pra mover a janela
    sf::Vector2i pegaDoArraste;       // onde o mouse pegou a janela
    sf::Font fonte;
    bool temFonte = false;
    sf::Texture sheets[9];
    bool temSheet[9] = {};
    sf::Texture texGato[3]; // os pets (originalmente eram 3 gatos)
    bool temGato[3] = {};
    sf::Texture texPirulitos;
    bool temPirulitos = false;
    Mapa mapa;
    sf::RenderTexture labirinto;
    Efeitos efeitos;
    Sons sons;
    std::vector<EstrelaDeFundo> estrelas;

    // personagens
    Jogadora jogadora;
    std::array<Rival, 4> rivais;

    // estado geral
    Tela tela = Tela::MENU;
    Tela telaAntesDaPausa = Tela::JOGANDO;
    float tempoNaTela = 0;
    float tempoTotal = 0;
    float escurecer = 1; //  transicao: vai clareando
    bool prontoLongo = true;  

    int fase = 1, vidas = 3, pontos = 0, recorde = 0;
    bool novoRecorde = false, ganhouVidaExtra = false;
    float popPontos = 0;    // placar "pula" quando ganha ponto

    // estado da fase
    int docesRestantes = 0, docesTotal = 0;
    bool chaveVisivel = false, gaiolaAberta = false;
    float tempoPoder = 0;
    int comboRivais = 0;
    int indiceModo = 0;
    float tempoModo = 0;
    bool modoPerseguir = false;
    float congelado = 0;          // pausa quando come uma rival
    //poderes
    bool pirulitoVisivel = false;
    int pirulitoTipo = 0;
    sf::Vector2i pirulitoPos;
    float tempoPirulito = 8;      // ate aparecer (ou ate sumir, se ja esta visivel)
    float tempoTurbo = 0;         // arco-iris: invencivel
    float tempoIma = 0;           // rosa: puxa os docinhos de perto
    float tempoHipnose = 0;       // vermelho: rivais param, tontas
    float tempoCorrida = 0;
    float recargaCorrida = 0;
    int notaDoce = 0;             // qual nota o proximo docinho toca
    float tempoDesdeDoce = 0;
    bool efeitoMorteFeito = false;
    std::string aviso;
    float tempoAviso = 0;

    Jogo();
    void rodar();

    // regras
    const Dificuldade& dif() const;
    void novoJogo();
    void carregarFase(int n);
    void reiniciarPosicoes();
    void irPara(Tela t);
    void processarEventos();
    void teclaApertada(sf::Keyboard::Scancode tecla);
    void atualizar(float dt);
    void atualizarJogadora(float dt);
    void chegouNaCasa();
    void comerDoce(sf::Vector2i casa);
    void atualizarPoderes(float dt);
    void sortearPirulito();
    void pegarPirulito();
    void usarCorrida();
    float velocidadeJogadora() const;
    void atualizarRivais(float dt);
    void atualizarRival(Rival& r, float dt);
    void decidirCaminho(Rival& r);
    sf::Vector2i alvoDa(const Rival& r) const;
    void verificarEncontros();
    void comerRival(Rival& r);
    void perderVida();
    void ganharPontos(int n);
    void ativarPoder();
    void mostrarAviso(const std::string& s);
    void carregarRecorde();
    void carregarIdioma();
    void trocarIdioma();
    void aplicarIdioma();      
    void salvarRecorde();

    // desenho
    void desenhar();
    void desenharFundo();
    void desenharFase();
    void desenharPersonagens();
    void desenharHUD();
    void desenharMenu();
    void desenharTelaFinal(bool venceu);
    void desenharBoba(sf::Vector2f c, float t);
    void desenharChave(sf::Vector2f c, float t);
    void desenharGato(int cor, sf::Vector2f pe, float t, AnimGato anim, float escala = 1.f,
                      bool olhandoDireita = false);
    void desenharPirulito(int tipo, sf::Vector2f centro, float tamanho, float giro = 0.f);
    void desenharBala(sf::Vector2f centro, int variedade, float t, float escala = 1.f);
    void desenharPainel(sf::Vector2f centro, sf::Vector2f tamanho, sf::Color fundo, sf::Color borda);
    sf::View vistaJogo() const;       // desenha o jogo abaixo da barra de titulo
    void desenharBarraTitulo();
    void cliqueNaBarra(sf::Vector2i pos);
    void desenharGaiola();
    void desenharSprite(int sheet, sf::Vector2f pe, Direcao d, int quadro,
                        sf::Color tinta = sf::Color::White, float escala = 1.f, float giro = 0.f);
    void escrever(const std::string& s, unsigned tamanho, sf::Vector2f centro, sf::Color cor,
                  float escala = 1.f);
    sf::Vector2f posGatinhoAgora() const;
};

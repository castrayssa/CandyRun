#include "personagens.hpp"

void Andarilho::colocarEm(sf::Vector2i c, Direcao d) {
    casa = proxima = c;
    dir = d;
    progresso = 0;
    andando = false;
}

void Andarilho::andarPara(Direcao d) {
    dir = d;
    proxima = passo(casa, d);
    andando = true;
}

void Andarilho::inverter() {
    if (!andando) return;
    std::swap(casa, proxima);
    progresso = 1.f - progresso;
    dir = oposta(dir);
}

bool Andarilho::avancar(float velocidade, float dt, const Mapa& mapa) {
    if (!andando) return false;
    progresso += velocidade * dt;
    distanciaAndada += velocidade * dt;
    if (progresso < 1.f) return false;

    progresso -= 1.f;
    casa = mapa.normalizar(proxima);     // volta pro outro lado
    proxima = casa;
    return true;
}

sf::Vector2f Andarilho::posicao(const Mapa& mapa) const {
    sf::Vector2f a = mapa.centro(casa);
    sf::Vector2f b = mapa.centro(proxima);
    if (std::abs(casa.x - proxima.x) > 1) {
        if (proxima.x > casa.x) b.x = a.x - TILE;
        else b.x = a.x + TILE;
    }
    return a + (b - a) * progresso;
}

int Andarilho::quadroAnimacao() const {
    if (!andando) return 1;                     // parada quando comeca
    const int ciclo[4] = {0, 1, 2, 1};
    return ciclo[int(distanciaAndada * 4.f) % 4];
}

Direcao escolherDirecao(const Rival& r, sf::Vector2i alvo, const Mapa& mapa, bool aleatoria) {
    bool podePorta = r.estado == EstadoRival::SAINDO || r.estado == EstadoRival::VOLTANDO;
    const Direcao ordem[4] = {CIMA, ESQUERDA, BAIXO, DIREITA};   // desempate

    std::vector<Direcao> opcoes;
    for (Direcao d : ordem) {
        if (d == oposta(r.dir)) continue;                  // nao da meia-volta
        if (mapa.livreParaRival(passo(r.casa, d), podePorta)) opcoes.push_back(d);
    }
    if (opcoes.empty()) return oposta(r.dir);            
    if (aleatoria) return opcoes[sortearInt(0, int(opcoes.size()) - 1)];

    Direcao melhor = opcoes[0];
    long melhorDist = -1;
    for (Direcao d : opcoes) {
        sf::Vector2i p = passo(r.casa, d);
        long dx = p.x - alvo.x, dy = p.y - alvo.y;
        long dist = dx * dx + dy * dy;
        if (melhorDist < 0 || dist < melhorDist) {
            melhorDist = dist;
            melhor = d;
        }
    }
    return melhor;
}

void desenharPersonagem(sf::RenderTarget& alvo, const sf::Texture& sheet, sf::Vector2f pe,
                        Direcao olhando, int quadro, sf::Color tinta, float escala, float giro) {
    int linha = (olhando == NENHUMA) ? BAIXO : int(olhando);
    sf::Sprite s(sheet);
    s.setTextureRect(sf::IntRect({quadro * CELULA_L, linha * CELULA_A}, {CELULA_L, CELULA_A}));
    s.setOrigin({CELULA_L / 2.f, float(CELULA_A - CELULA_MARGEM)});    // origem nos pes
    s.setScale({ESCALA_PERSONAGEM * escala, ESCALA_PERSONAGEM * escala});
    s.setPosition(pe);
    s.setRotation(sf::degrees(giro));
    s.setColor(tinta);
    alvo.draw(s);
}

void desenharSombra(sf::RenderTarget& alvo, sf::Vector2f pe, sf::Color cor, float escala) {
    sf::CircleShape sombra(15.f * escala);
    sombra.setOrigin({sombra.getRadius(), sombra.getRadius()});
    sombra.setScale({1.f, 0.38f});
    sombra.setPosition(pe);
    sombra.setFillColor(cor);
    alvo.draw(sombra);
}

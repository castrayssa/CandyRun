#include "efeitos.hpp"

void Efeitos::explosao(sf::Vector2f pos, int quantidade, sf::Color cor, float forca, bool coracoes) {
    for (int i = 0; i < quantidade; i++) {
        float ang = sortear(0.f, 6.2832f);
        float vel = sortear(0.3f, 1.f) * forca;
        Particula p;
        p.pos = pos;
        p.vel = {std::cos(ang) * vel, std::sin(ang) * vel - forca * 0.3f};
        p.cor = misturar(cor, sf::Color::White, sortear(0.f, 0.35f));
        p.vidaMax = p.vida = sortear(0.35f, 0.8f);
        p.tamanho = coracoes ? sortear(5.f, 8.f) : sortear(2.f, 4.5f);
        p.coracao = coracoes;
        particulas.push_back(p);
    }
}

void Efeitos::anel(sf::Vector2f pos, int quantidade, sf::Color cor, float raio) {
    for (int i = 0; i < quantidade; i++) {
        float ang = 6.2832f * i / quantidade;
        Particula p;
        p.pos = pos;
        p.vel = {std::cos(ang) * raio * 2.5f, std::sin(ang) * raio * 2.5f};
        p.cor = cor;
        p.vidaMax = p.vida = 0.45f;
        p.tamanho = 3.5f;
        p.coracao = false;
        particulas.push_back(p);
    }
}

void Efeitos::textoSubindo(sf::Vector2f pos, const std::string& s, sf::Color cor) {
    textos.push_back({pos, s, cor, 1.0f});
}

void Efeitos::atualizar(float dt) {
    for (auto& p : particulas) {
        p.pos += p.vel * dt;
        p.vel.y += 260.f * dt;          // gravidade leve
        p.vel *= 0.97f;
        p.vida -= dt;
    }
    std::erase_if(particulas, [](const Particula& p) { return p.vida <= 0; });

    for (auto& t : textos) {
        t.pos.y -= 40.f * dt;
        t.vida -= dt;
    }
    std::erase_if(textos, [](const TextoFlutuante& t) { return t.vida <= 0; });

    tremor = std::max(0.f, tremor - 30.f * dt);
}

sf::Vector2f Efeitos::deslocamentoTremor() const {
    if (tremor <= 0) return {0, 0};
    return {sortear(-tremor, tremor), sortear(-tremor, tremor)};
}

void Efeitos::desenhar(sf::RenderTarget& alvo, const sf::Font& fonte) const {
    for (auto& p : particulas) {
        float t = p.vida / p.vidaMax;          // 1 -> 0
        sf::Color c = p.cor;
        c.a = std::uint8_t(255 * std::min(1.f, t * 1.5f));
        if (p.coracao) {
            desenharCoracao(alvo, p.pos, p.tamanho * (0.6f + 0.4f * t), c);
        } else {
            sf::CircleShape bola(p.tamanho * (0.5f + 0.5f * t));
            bola.setOrigin({bola.getRadius(), bola.getRadius()});
            bola.setPosition(p.pos);
            bola.setFillColor(c);
            alvo.draw(bola);
        }
    }
    for (auto& t : textos) {
        sf::Color c = t.cor;
        c.a = std::uint8_t(255 * std::min(1.f, t.vida * 2.f));
        float pop = 1.f + 0.4f * std::max(0.f, t.vida - 0.8f) * 5.f;   // comeca maior
        escreverCentro(alvo, fonte, t.conteudo, 18, t.pos, c, pop, sf::Color(40, 20, 60, c.a));
    }
}

void Efeitos::limpar() {
    particulas.clear();
    textos.clear();
    tremor = 0;
}


void desenharCoracao(sf::RenderTarget& alvo, sf::Vector2f c, float t, sf::Color cor) {
    sf::CircleShape bola(t * 0.55f);
    bola.setFillColor(cor);
    bola.setOrigin({bola.getRadius(), bola.getRadius()});
    bola.setPosition({c.x - t * 0.45f, c.y - t * 0.25f});
    alvo.draw(bola);
    bola.setPosition({c.x + t * 0.45f, c.y - t * 0.25f});
    alvo.draw(bola);

    sf::ConvexShape ponta(3);
    ponta.setPoint(0, {c.x - t * 0.98f, c.y - t * 0.05f});
    ponta.setPoint(1, {c.x + t * 0.98f, c.y - t * 0.05f});
    ponta.setPoint(2, {c.x, c.y + t * 0.95f});
    ponta.setFillColor(cor);
    alvo.draw(ponta);
}

void desenharEstrela(sf::RenderTarget& alvo, sf::Vector2f c, float r, sf::Color cor, float giro) {
    sf::ConvexShape estrela(8);
    for (int i = 0; i < 8; i++) {
        float ang = giro + i * 3.14159f / 4.f;
        float raio = (i % 2 == 0) ? r : r * 0.35f;
        estrela.setPoint(i, {c.x + std::cos(ang) * raio, c.y + std::sin(ang) * raio});
    }
    estrela.setFillColor(cor);
    alvo.draw(estrela);
}

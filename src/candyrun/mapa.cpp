#include "mapa.hpp"
#include <fstream>

bool Mapa::carregar(const std::string& arquivo) {
    std::ifstream f(arquivo);
    if (!f) return false;

    grade.clear();
    vagasCasa.clear();
    std::string linha;
    while (std::getline(f, linha)) {
        if (!linha.empty() && linha.back() == '\r') linha.pop_back();
        if (!linha.empty()) grade.push_back(linha);
    }
    if (grade.empty()) return false;

    altura = int(grade.size());
    largura = int(grade[0].size());
    doces = grade;

    for (int y = 0; y < altura; y++) {
        grade[y].resize(largura, '#');     // garante linhas do mesmo tamanho
        doces[y].resize(largura, ' ');
        for (int x = 0; x < largura; x++) {
            char c = grade[y][x];
            if (c == 'J') inicioJogadora = {x, y};
            if (c == 'K') posChave = {x, y};
            if (c == 'G') posGatinho = {x, y};
            if (c == 'P') posPortaGaiola = {x, y};
            if (c == '-') posPortaCasa = {x, y};
            if (c == 'C') vagasCasa.push_back({x, y});
            doces[y][x] = (c == '.' || c == 'o') ? c : ' ';
        }
    }
    return true;
}

char Mapa::em(int x, int y) const {
    if (y < 0 || y >= altura) return '#';
    x = ((x % largura) + largura) % largura;
    return grade[y][x];
}

sf::Vector2i Mapa::normalizar(sf::Vector2i p) const {
    return {((p.x % largura) + largura) % largura, p.y};
}

bool Mapa::livreParaJogadora(sf::Vector2i p, bool gaiolaAberta) const {
    char c = em(p);
    if (c == 'G' || c == 'P') return gaiolaAberta;
    return c != '#' && c != '-' && c != 'C';
}

bool Mapa::livreParaRival(sf::Vector2i p, bool podeUsarPortaDaCasa) const {
    char c = em(p);
    if (c == '-' || c == 'C') return podeUsarPortaDaCasa;
    return c != '#' && c != 'G' && c != 'P';
}

sf::Vector2f Mapa::centro(float x, float y) const {
    return {x * TILE + TILE / 2.f, HUD_ALTURA + y * TILE + TILE / 2.f};
}
sf::Vector2f Mapa::centro(sf::Vector2i p) const { return centro(float(p.x), float(p.y)); }

int Mapa::contarDoces() const {
    int n = 0;
    for (auto& l : doces)
        for (char c : l)
            if (c == '.' || c == 'o') n++;
    return n;
}


// parede
void Mapa::desenharLabirinto(sf::RenderTarget& alvo, sf::Color corTopo, sf::Color corLado,
                             sf::Color corChao) const {
    const float m = 5.f;          // recuo da parede quando nao tem vizinho
    const float altura3D = 6.f;  

    auto ehParede = [&](int x, int y) {
        if (y < 0 || y >= altura) return false;
        if (x < 0 || x >= largura) return false;
        char c = grade[y][x];
        return c == '#';
    };

    // chao dos corredores
    for (int y = 0; y < altura; y++)
        for (int x = 0; x < largura; x++)
            if (!ehParede(x, y)) {
                sf::RectangleShape chao({float(TILE), float(TILE)});
                chao.setPosition({float(x * TILE), float(y * TILE)});
                chao.setFillColor((x + y) % 2 ? corChao : misturar(corChao, sf::Color::White, 0.04f));
                alvo.draw(chao);
            }

    auto bloco = [&](int x, int y, float deslocY, sf::Color cor) {
        float esq = ehParede(x - 1, y) ? 0 : m;
        float dir = ehParede(x + 1, y) ? 0 : m;
        float cim = ehParede(x, y - 1) ? 0 : m;
        float bai = ehParede(x, y + 1) ? 0 : m;
        sf::RectangleShape r({TILE - esq - dir, TILE - cim - bai});
        r.setPosition({x * TILE + esq, y * TILE + cim + deslocY});
        r.setFillColor(cor);
        alvo.draw(r);
    };

    for (int y = 0; y < altura; y++)        // sombra/lateral
        for (int x = 0; x < largura; x++)
            if (ehParede(x, y)) bloco(x, y, altura3D, corLado);

    for (int y = 0; y < altura; y++)        // topo
        for (int x = 0; x < largura; x++)
            if (ehParede(x, y)) {
                bloco(x, y, 0, corTopo);
                if (!ehParede(x, y - 1)) {     // brilho na borda de cima
                    float esq = ehParede(x - 1, y) ? 0 : m + 3;
                    float dir = ehParede(x + 1, y) ? 0 : m + 3;
                    sf::RectangleShape brilho({TILE - esq - dir, 3.f});
                    brilho.setPosition({x * TILE + esq, y * TILE + m + 2});
                    brilho.setFillColor(sf::Color(255, 255, 255, 90));
                    alvo.draw(brilho);
                }
            }

    // porta de onde as rivais saem
    for (int i = 0; i < 8; i++) {
        sf::RectangleShape listra({TILE / 8.f, 7.f});
        listra.setPosition({posPortaCasa.x * TILE + i * TILE / 8.f, posPortaCasa.y * TILE + TILE / 2.f - 3.5f});
        listra.setFillColor(i % 2 ? sf::Color(255, 250, 250) : sf::Color(255, 120, 170));
        alvo.draw(listra);
    }
}

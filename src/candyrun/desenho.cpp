// fundo, labirinto, itens, personagens, HUD e telas
#include "jogo.hpp"
#include "janela_sistema.hpp"
#include <cctype>

//ajudantes

void Jogo::escrever(const std::string& s, unsigned tamanho, sf::Vector2f centro, sf::Color cor, float escala) {
    if (temFonte) escreverCentro(janela, fonte, s, tamanho, centro, cor, escala);
}

// se a imagem da personagem nao carregou, desenha uma bolinha no lugar
void Jogo::desenharSprite(int sheet, sf::Vector2f pe, Direcao d, int quadro, sf::Color tinta, float escala,
                          float giro) {
    if (temSheet[sheet]) {
        desenharPersonagem(janela, sheets[sheet], pe, d, quadro, tinta, escala, giro);
    } else {
        sf::CircleShape bola(16.f * escala);
        bola.setOrigin({bola.getRadius(), bola.getRadius() * 2});
        bola.setPosition(pe);
        bola.setFillColor(tinta);
        janela.draw(bola);
    }
}

// nome do pet de cada fase 
static std::string nomePet(int fase) {
    if (fase == 1) return T("cachorrinho", "puppy");
    if (fase == 2) return T("gatinho", "kitty");
    return T("sapinho", "froggy");
}

static std::string comMaiuscula(std::string s) {
    if (!s.empty()) s[0] = char(std::toupper(static_cast<unsigned char>(s[0])));
    return s;
}

sf::Vector2f Jogo::posGatinhoAgora() const {
    sf::Vector2f naGaiola = mapa.centro(mapa.posGatinho) + sf::Vector2f(0, 16);
    if (tela != Tela::FASE_COMPLETA) return naGaiola;

    sf::Vector2f destino = jogadora.posicao(mapa) + sf::Vector2f(34, 14);
    float t = std::min(1.f, tempoNaTela / 0.8f);
    return naGaiola + (destino - naGaiola) * t;
}

// desenho principal

void Jogo::desenhar() {
    janela.clear();

    sf::View camera = vistaJogo();
    camera.move(efeitos.deslocamentoTremor());
    janela.setView(camera);

    desenharFundo();

    switch (tela) {
        case Tela::MENU:
            desenharMenu();
            break;
        case Tela::VITORIA:
            desenharTelaFinal(true);
            break;
        case Tela::FIM_DE_JOGO:
            desenharTelaFinal(false);
            break;
        default:
            desenharFase();
            desenharHUD();
            break;
    }

    efeitos.desenhar(janela, fonte);

    // textos por cima de tudo
    sf::Vector2f meio = {janela.getSize().x / 2.f, HUD_ALTURA + mapa.altura * TILE / 2.f};
    if (tela == Tela::PRONTO) {
        float pop = 1.f + 0.25f * std::max(0.f, 1.f - tempoNaTela * 4.f);
        bool vai = tempoNaTela > (prontoLongo ? 1.6f : 0.8f);
        escrever(vai ? T("Vai!", "Go!") : T("Prontos?", "Ready?"), 34, meio + sf::Vector2f(0, 2.f * TILE),
                 vai ? sf::Color(255, 230, 120) : sf::Color::White, pop);
        if (prontoLongo) escrever(T("Fase ", "Level ") + std::to_string(fase), 26, meio + sf::Vector2f(0, -2.6f * TILE),
                                  sf::Color(255, 190, 220));
    }
    if (tempoAviso > 0 && (tela == Tela::JOGANDO || tela == Tela::PRONTO)) {
        float pop = 1.f + 0.3f * std::max(0.f, (tempoAviso - 1.8f) * 5.f);
        sf::Color cor(255, 240, 180, std::uint8_t(255 * std::min(1.f, tempoAviso * 2.f)));
        escrever(aviso, 22, meio + sf::Vector2f(0, 2.f * TILE), cor, pop);
    }
    if (tela == Tela::FASE_COMPLETA) {
        float pop = 1.f + 0.4f * std::max(0.f, 1.f - tempoNaTela * 3.f);
        escrever(comMaiuscula(nomePet(fase)) + T(" salvo!", " saved!"), 40, meio + sf::Vector2f(0, -1.5f * TILE), sf::Color(255, 200, 230), pop);
        escrever(fase < NUM_FASES ? T("Próxima fase...", "Next level...") : T("Último bichinho!", "Last pet!"), 20, meio + sf::Vector2f(0, 1.5f * TILE),
                 sf::Color::White);
    }
    if (tela == Tela::PAUSA) {
        janela.setView(vistaJogo());
        sf::RectangleShape veu(tamJogo);
        veu.setFillColor(sf::Color(20, 10, 40, 170));
        janela.draw(veu);
        escrever(T("Pausado", "Paused"), 44, meio + sf::Vector2f(0, -30), sf::Color(255, 200, 230));
        escrever(T("P: continuar     Q: voltar ao menu     M: som     L: idioma",
                   "P: resume     Q: back to menu     M: sound     L: language"), 16, meio + sf::Vector2f(0, 40), sf::Color::White);
    }

    // transicao: tela escura clareando
    if (escurecer > 0) {
        janela.setView(vistaJogo());
        sf::RectangleShape preto(tamJogo);
        preto.setFillColor(sf::Color(10, 5, 20, std::uint8_t(255 * escurecer)));
        janela.draw(preto);
    }

    janela.setView(janela.getDefaultView());
    desenharBarraTitulo();
    janela.display();
}

void Jogo::desenharFundo() {
    sf::Vector2f tam = tamJogo;
    const Dificuldade& d = dif();     // cada fase tem um ceu de uma cor
    sf::VertexArray ceu(sf::PrimitiveType::TriangleStrip, 4);
    ceu[0].position = {-20, -20};
    ceu[1].position = {tam.x + 20, -20};
    ceu[2].position = {-20, tam.y + 20};
    ceu[3].position = {tam.x + 20, tam.y + 20};
    ceu[0].color = ceu[1].color = d.ceuCima;
    ceu[2].color = ceu[3].color = d.ceuBaixo;
    janela.draw(ceu);

    for (auto& e : estrelas) {
        float brilho = 0.5f + 0.5f * std::sin(tempoTotal * 1.5f + e.fase);
        sf::CircleShape c(e.tamanho);
        c.setOrigin({e.tamanho, e.tamanho});
        c.setPosition(e.pos);
        c.setFillColor(sf::Color(255, 240, 255, std::uint8_t(40 + 120 * brilho)));
        janela.draw(c);
    }
}

// fase

void Jogo::desenharFase() {
    sf::Sprite paredes(labirinto.getTexture());
    paredes.setPosition({0, float(HUD_ALTURA)});
    // na fase completa as paredes piscam de alegria
    if (tela == Tela::FASE_COMPLETA && std::fmod(tempoNaTela, 0.3f) < 0.15f && tempoNaTela < 1.5f)
        paredes.setColor(sf::Color(255, 255, 255, 170));
    janela.draw(paredes);

    // docinhos (balas embrulhadas e balas de hortela) e bobas
    for (int y = 0; y < mapa.altura; y++)
        for (int x = 0; x < mapa.largura; x++) {
            char c = mapa.doces[y][x];
            sf::Vector2f p = mapa.centro(sf::Vector2i{x, y});
            if (c == '.') {
                int variedade = ((x * 3 + y * 5) % 6 == 0) ? 4 : (x * 7 + y * 3) % 4;
                desenharBala(p, variedade, tempoTotal + x * 0.7f + y * 1.3f);
            } else if (c == 'o') {
                desenharBoba(p, tempoTotal + x);
            }
        }

    // pirulito (poder especial): pisca quando esta quase sumindo
    if (pirulitoVisivel && (tempoPirulito > 2.5f || std::fmod(tempoTotal, 0.3f) < 0.2f)) {
        sf::Vector2f p = mapa.centro(pirulitoPos);
        p.y -= 4.f + std::abs(std::sin(tempoTotal * 4.f)) * 5.f;
        sf::CircleShape brilho(19.f + std::sin(tempoTotal * 6.f) * 2.f);
        brilho.setOrigin({brilho.getRadius(), brilho.getRadius()});
        brilho.setPosition(p);
        brilho.setFillColor(sf::Color(255, 255, 255, 50));
        janela.draw(brilho);
        desenharPirulito(pirulitoTipo, p, 44.f, std::sin(tempoTotal * 3.f) * 10.f);
        desenharEstrela(janela, p + sf::Vector2f(15, -14), 4.f + 2.f * std::sin(tempoTotal * 8.f),
                        sf::Color::White, tempoTotal * 2.f);
    }

    if (chaveVisivel) desenharChave(mapa.centro(mapa.posChave), tempoTotal);
    desenharGaiola();
    desenharPersonagens();
}

void Jogo::desenharPersonagens() {
    // quem esta mais embaixo na tela e desenhado por ultimo (fica na frente)
    struct Item { float y; int quem; };   // quem: -1 = jogadora, 0..3 = rivais
    std::vector<Item> ordem;
    sf::Vector2f pj = jogadora.posicao(mapa);
    ordem.push_back({pj.y, -1});

    bool rivaisVisiveis = !(tela == Tela::FASE_COMPLETA || (tela == Tela::MORRENDO && tempoNaTela > 0.4f));
    if (rivaisVisiveis)
        for (int i = 0; i < 4; i++) ordem.push_back({rivais[i].posicao(mapa).y, i});
    std::sort(ordem.begin(), ordem.end(), [](const Item& a, const Item& b) { return a.y < b.y; });

    const float pePraBaixo = TILE * 0.32f;     // o pe fica um pouco abaixo do centro da casa

    for (const Item& it : ordem) {
        if (it.quem == -1) {
            sf::Vector2f pe = pj + sf::Vector2f(0, pePraBaixo);
            if (tela == Tela::MORRENDO) {
                // gira olhando pra todos os lados e some
                const Direcao giro[4] = {BAIXO, ESQUERDA, CIMA, DIREITA};
                float t = tempoNaTela;
                Direcao d = giro[int(t / 0.07f) % 4];
                float escala = std::clamp(1.f - (t - 0.3f) / 1.1f, 0.f, 1.f);
                if (escala > 0) {
                    desenharSombra(janela, pe, sf::Color(0, 0, 0, 80), escala);
                    desenharSprite(SPRITE_JOGADORA, pe, d, 1, sf::Color(255, 255, 255, std::uint8_t(255 * escala)), escala);
                }
                continue;
            }
            desenharSombra(janela, pe, sf::Color(0, 0, 0, 90));
            sf::Color tinta = sf::Color::White;
            if (tempoTurbo > 0) {  // turbo: brilha nas cores do arco-iris
                tinta = misturar(sf::Color::White, corArcoIris(tempoTotal * 2.f), 0.5f);
                sf::CircleShape aura(24.f);
                aura.setOrigin({24.f, 24.f});
                aura.setScale({0.9f, 1.25f});
                aura.setPosition(pe + sf::Vector2f(0, -26));
                sf::Color ca = corArcoIris(tempoTotal * 2.f);
                ca.a = 90;
                aura.setFillColor(ca);
                janela.draw(aura);
            }
            if (tempoIma > 0) {   // ima: anel rosa mostrando o alcance
                float raio = 2.6f * TILE;
                sf::CircleShape anel(raio);
                anel.setOrigin({raio, raio});
                anel.setPosition(pj);
                anel.setFillColor(sf::Color(255, 150, 200, 14));
                anel.setOutlineColor(sf::Color(255, 150, 200, std::uint8_t(90 + 50 * std::sin(tempoTotal * 8.f))));
                anel.setOutlineThickness(2.f);
                janela.draw(anel);
            }
            desenharSprite(SPRITE_JOGADORA, pe, jogadora.dir, jogadora.quadroAnimacao(), tinta);
            continue;
        }

        Rival& r = rivais[it.quem];
        sf::Vector2f pe = r.posicao(mapa) + sf::Vector2f(0, pePraBaixo);
        sf::Color tinta = sf::Color::White;
        sf::Color sombra = sf::Color(r.cor.r, r.cor.g, r.cor.b, 150);
        Direcao olhando = r.dir;
        int quadro = r.quadroAnimacao();

        if (r.estado == EstadoRival::EM_CASA) {
            pe.y += std::sin(tempoTotal * 5.f + it.quem) * 3.f;  // pulinho esperando
            olhando = BAIXO;
            quadro = int(tempoTotal * 4 + it.quem) % 3;
        }
        if (r.estado == EstadoRival::VOLTANDO) {
            tinta = sf::Color(255, 255, 255, 80); // vira fantasminha
            sombra.a = 40;
        } else if (r.assustada) {
            bool piscando = tempoPoder < 2.f && std::fmod(tempoTotal, 0.3f) < 0.15f;
            tinta = piscando ? sf::Color(255, 255, 255) : sf::Color(165, 190, 255);
            sombra = sf::Color(90, 120, 255, 150);
            pe.x += std::sin(tempoTotal * 25.f) * 1.5f;   // tremendo de medo

            sf::CircleShape aura(20.f);  // brilho azul em volta
            aura.setOrigin({20.f, 20.f});
            aura.setScale({0.9f, 1.2f});
            aura.setPosition(pe + sf::Vector2f(0, -24));
            aura.setFillColor(piscando ? sf::Color(255, 255, 255, 70) : sf::Color(110, 150, 255, 90));
            janela.draw(aura);
        }

        bool tonta = tempoHipnose > 0 && r.estado == EstadoRival::ATIVA && !r.assustada;
        if (tonta) {  // hipnotizada: gira no lugar
            const Direcao giro[4] = {BAIXO, ESQUERDA, CIMA, DIREITA};
            olhando = giro[int(tempoTotal * 5.f + it.quem) % 4];
            quadro = 1;
            tinta = sf::Color(255, 185, 200);
        }

        desenharSombra(janela, pe, sombra);
        desenharSprite(r.sprite, pe, olhando, quadro, tinta);
        if (tonta)
            for (int k = 0; k < 3; k++) {      // estrelinhas rodando na cabeca
                float ang = tempoTotal * 5.f + k * 2.094f;
                desenharEstrela(janela, pe + sf::Vector2f(std::cos(ang) * 14.f, -62.f + std::sin(ang) * 4.f), 4.f,
                                sf::Color(255, 240, 150), ang);
            }
        if (r.assustada && r.estado != EstadoRival::VOLTANDO) {    // gota de suor
            sf::CircleShape gota(3.f);
            gota.setPosition(pe + sf::Vector2f(12, -50 - std::abs(std::sin(tempoTotal * 6.f)) * 3));
            gota.setFillColor(sf::Color(150, 210, 255));
            janela.draw(gota);
        }
    }
}

void Jogo::desenharBoba(sf::Vector2f c, float t) {
    c.y -= std::abs(std::sin(t * 4.f)) * 3.f;

    sf::CircleShape brilho(14.f + std::sin(t * 6.f) * 2.f);
    brilho.setOrigin({brilho.getRadius(), brilho.getRadius()});
    brilho.setPosition(c);
    brilho.setFillColor(sf::Color(255, 180, 220, 60));
    janela.draw(brilho);

    sf::RectangleShape canudo({3.f, 16.f});
    canudo.setOrigin({1.5f, 16.f});
    canudo.setPosition(c + sf::Vector2f(2, -6));
    canudo.setRotation(sf::degrees(15));
    canudo.setFillColor(sf::Color(255, 110, 170));
    janela.draw(canudo);

    sf::ConvexShape copo(4);
    copo.setPoint(0, c + sf::Vector2f(-8, -7));
    copo.setPoint(1, c + sf::Vector2f(8, -7));
    copo.setPoint(2, c + sf::Vector2f(6, 10));
    copo.setPoint(3, c + sf::Vector2f(-6, 10));
    copo.setFillColor(sf::Color(238, 205, 165));
    copo.setOutlineColor(sf::Color(255, 255, 255, 220));
    copo.setOutlineThickness(1.5f);
    janela.draw(copo);

    sf::RectangleShape tampa({18.f, 3.f});
    tampa.setPosition(c + sf::Vector2f(-9, -9));
    tampa.setFillColor(sf::Color(255, 255, 255));
    janela.draw(tampa);

    for (int i = 0; i < 3; i++) {                 // bolinhas de tapioca
        sf::CircleShape perola(2.2f);
        perola.setPosition(c + sf::Vector2f(-5.f + i * 3.6f, 4.f + (i % 2)));
        perola.setFillColor(sf::Color(70, 40, 40));
        janela.draw(perola);
    }
}

void Jogo::desenharChave(sf::Vector2f c, float t) {
    c.y += std::sin(t * 3.f) * 3.f;

    sf::CircleShape brilho(16.f + std::sin(t * 5.f) * 3.f);
    brilho.setOrigin({brilho.getRadius(), brilho.getRadius()});
    brilho.setPosition(c);
    brilho.setFillColor(sf::Color(255, 220, 90, 70));
    janela.draw(brilho);

    sf::Color ouro(255, 210, 70);
    sf::CircleShape argola(6.f);
    argola.setOrigin({6.f, 6.f});
    argola.setPosition(c + sf::Vector2f(-7, 0));
    argola.setFillColor(sf::Color::Transparent);
    argola.setOutlineColor(ouro);
    argola.setOutlineThickness(3.f);
    janela.draw(argola);

    sf::RectangleShape haste({14.f, 4.f});
    haste.setPosition(c + sf::Vector2f(-1, -2));
    haste.setFillColor(ouro);
    janela.draw(haste);
    sf::RectangleShape dente({3.f, 5.f});
    dente.setFillColor(ouro);
    dente.setPosition(c + sf::Vector2f(8, 1));
    janela.draw(dente);
    dente.setPosition(c + sf::Vector2f(4, 1));
    janela.draw(dente);

    desenharEstrela(janela, c + sf::Vector2f(10, -10), 4.f + 2.f * std::sin(t * 8.f), sf::Color::White, t * 2.f);
}

// eram 3 gatos, mas reaproveitei
void Jogo::desenharGato(int cor, sf::Vector2f pe, float t, AnimGato anim, float e, bool olhandoDireita) {
    cor = std::clamp(cor, 0, 2);
    if (!temGato[cor]) {                         // reserva: sem a imagem, vira uma bolinha
        const sf::Color cores[3] = {{246, 160, 80}, {252, 248, 240}, {40, 36, 50}};
        sf::CircleShape bola(12.f * e);
        bola.setOrigin({12.f * e, 24.f * e});
        bola.setPosition(pe);
        bola.setFillColor(cores[cor]);
        bola.setOutlineColor(sf::Color(214, 200, 240));
        bola.setOutlineThickness(1.5f);
        janela.draw(bola);
        return;
    }
    int celL = int(texGato[cor].getSize().x) / 4;
    int celA = int(texGato[cor].getSize().y) / 3;
    const float quadrosPorSegundo[3] = {10.f, 3.f, 1.5f};
    int quadro = int(t * quadrosPorSegundo[anim]) % 4;

    sf::Sprite s(texGato[cor]);
    s.setTextureRect(sf::IntRect({quadro * celL, int(anim) * celA}, {celL, celA}));
    const float MARGEM_PET = 32.f;               // espaco vazio embaixo de cada quadro (FOLGA em fatiar_pets.py)
    s.setOrigin({celL / 2.f, celA - MARGEM_PET});
    float escala = 0.19f * e;
    s.setScale({olhandoDireita ? -escala : escala, escala});   // o desenho original olha pra esquerda
    s.setPosition(pe);
    janela.draw(s);
}

// tipo
void Jogo::desenharPirulito(int tipo, sf::Vector2f centro, float tamanho, float giro) {
    if (!temPirulitos) {
        const sf::Color cores[3] = {{120, 220, 255}, {255, 170, 200}, {235, 70, 100}};
        sf::CircleShape bola(tamanho * 0.3f);
        bola.setOrigin({bola.getRadius(), bola.getRadius()});
        bola.setPosition(centro);
        bola.setFillColor(cores[tipo % 3]);
        bola.setOutlineColor(sf::Color::White);
        bola.setOutlineThickness(2.f);
        janela.draw(bola);
        return;
    }
    int cel = int(texPirulitos.getSize().x) / 3;
    sf::Sprite s(texPirulitos);
    s.setTextureRect(sf::IntRect({(tipo % 3) * cel, 0}, {cel, cel}));
    s.setOrigin({cel / 2.f, cel / 2.f});
    s.setScale({tamanho / cel, tamanho / cel});
    s.setRotation(sf::degrees(giro));
    s.setPosition(centro);
    janela.draw(s);
}

// doce do chao, variedade e cores
void Jogo::desenharBala(sf::Vector2f centro, int variedade, float t, float e) {
    sf::Transform tr;
    tr.translate(centro);
    tr.scale({e, e});

    if (variedade == 4) {                        // hortela
        tr.rotate(sf::degrees(t * 40.f));
        sf::RenderStates st;
        st.transform = tr;
        sf::CircleShape disco(6.f);
        disco.setOrigin({6.f, 6.f});
        disco.setFillColor(sf::Color(255, 250, 250));
        disco.setOutlineColor(sf::Color(225, 70, 105));
        disco.setOutlineThickness(1.5f);
        janela.draw(disco, st);
        for (int k = 0; k < 3; k++) {
            sf::RectangleShape gomo({11.f, 2.4f});
            gomo.setOrigin({5.5f, 1.2f});
            gomo.setRotation(sf::degrees(k * 60.f));
            gomo.setFillColor(sf::Color(240, 85, 120));
            janela.draw(gomo, st);
        }
        sf::CircleShape miolo(2.f);
        miolo.setOrigin({2.f, 2.f});
        miolo.setFillColor(sf::Color(255, 250, 250));
        janela.draw(miolo, st);
        return;
    }

    const sf::Color cores[4] = {{255, 130, 180}, {120, 225, 205}, {255, 215, 110}, {180, 150, 255}};
    sf::Color cor = cores[variedade % 4];
    tr.rotate(sf::degrees(-20.f + 14.f * std::sin(t * 2.5f)));  
    sf::RenderStates st;
    st.transform = tr;

    for (float lado : {-1.f, 1.f}) {             // pontas do papel
        sf::ConvexShape ponta(3);
        ponta.setPoint(0, {lado * 4.f, 0.f});
        ponta.setPoint(1, {lado * 10.5f, -4.5f});
        ponta.setPoint(2, {lado * 10.5f, 4.5f});
        ponta.setFillColor(misturar(cor, sf::Color::White, 0.5f));
        janela.draw(ponta, st);
    }
    sf::CircleShape corpo(5.f);                  // a bala
    corpo.setOrigin({5.f, 5.f});
    corpo.setScale({1.15f, 0.95f});
    corpo.setFillColor(cor);
    corpo.setOutlineColor(misturar(cor, sf::Color(70, 30, 80), 0.45f));
    corpo.setOutlineThickness(1.f);
    janela.draw(corpo, st);
    sf::RectangleShape listra({2.2f, 8.f});      // listra do papel
    listra.setOrigin({1.1f, 4.f});
    listra.setFillColor(sf::Color(255, 255, 255, 150));
    janela.draw(listra, st);
    sf::CircleShape brilho(1.3f);
    brilho.setPosition({-4.f, -3.5f});
    brilho.setFillColor(sf::Color(255, 255, 255, 230));
    janela.draw(brilho, st);
}

// painel quadrado (usado no menu), no mesmo estilo reto do resto do jogo
void Jogo::desenharPainel(sf::Vector2f centro, sf::Vector2f tam, sf::Color fundo, sf::Color borda) {
    sf::RectangleShape forma(tam);
    forma.setOrigin(tam / 2.f);
    forma.setPosition(centro);
    forma.setFillColor(fundo);
    forma.setOutlineColor(borda);
    forma.setOutlineThickness(2.f);
    janela.draw(forma);
}

//barra de titulo

sf::View Jogo::vistaJogo() const {
    sf::View v(sf::FloatRect({0.f, 0.f}, tamJogo));
    float total = tamJogo.y + BARRA_ALTURA;
    v.setViewport(sf::FloatRect({0.f, BARRA_ALTURA / total}, {1.f, tamJogo.y / total}));
    return v;
}

// botoes da direita: 0 = minimizar, 1 = fechar
static sf::FloatRect botaoBarra(float larguraJanela, int i) {
    const float lado = BARRA_ALTURA * 1.4f;
    return sf::FloatRect({larguraJanela - lado * (2 - i), 0.f}, {lado, float(BARRA_ALTURA)});
}

void Jogo::cliqueNaBarra(sf::Vector2i pos) {
    float L = float(janela.getSize().x);
    sf::Vector2f p(pos);
    if (botaoBarra(L, 0).contains(p)) {
        minimizarJanela(janela.getNativeHandle());
    } else if (botaoBarra(L, 1).contains(p)) {
        janela.close();
    } else {
        arrastando = true;
        pegaDoArraste = sf::Mouse::getPosition() - janela.getPosition();
    }
}

void Jogo::desenharBarraTitulo() {
    float L = float(janela.getSize().x);
    const float B = float(BARRA_ALTURA);
    // barra
    sf::RectangleShape fundo({L, B});
    fundo.setFillColor(sf::Color(30, 18, 60));
    janela.draw(fundo);
    sf::RectangleShape listra({L, 2.f});
    listra.setPosition({0.f, B - 2.f});
    listra.setFillColor(sf::Color(255, 170, 210, 200));
    janela.draw(listra);

    // logo: pirulito arco-iris + o nome com as cores do titulo do menu
    desenharPirulito(PIRULITO_ARCO_IRIS, {20.f, B / 2 - 1}, 28.f, std::sin(tempoTotal * 2.f) * 8.f);
    if (temFonte) {
        const std::string nome = "CandyRun";
        const sf::Color cores[4] = {{255, 120, 170}, {255, 205, 110}, {130, 225, 210}, {190, 160, 255}};
        float x = 40.f;
        for (std::size_t i = 0; i < nome.size(); i++) {
            sf::Text letra(fonte, sf::String(nome[i]), 16);
            letra.setFillColor(cores[i % 4]);
            letra.setOutlineColor(sf::Color(60, 30, 90));
            letra.setOutlineThickness(1.5f);
            letra.setPosition({x, B / 2 - 11.f});
            janela.draw(letra);
            x += fonte.getGlyph(std::uint32_t(nome[i]), 16, false).advance;
        }
    }

    // botoes acendem quando o mouse passa por cima
    sf::Vector2f mouse(sf::Mouse::getPosition(janela));
    bool focada = janela.hasFocus();
    for (int i = 0; i < 2; i++) {
        sf::FloatRect r = botaoBarra(L, i);
        bool emCima = focada && r.contains(mouse);
        if (emCima) {
            sf::RectangleShape luz({r.size.x, r.size.y - 2.f});
            luz.setPosition(r.position);
            luz.setFillColor(i == 0 ? sf::Color(255, 170, 210, 80) : sf::Color(255, 90, 130));
            janela.draw(luz);
        }
        sf::Color cor = emCima ? sf::Color::White : sf::Color(255, 200, 230);
        sf::Vector2f c = r.position + r.size / 2.f - sf::Vector2f(0, 1);
        if (i == 0) {                                   // minimizar: tracinho
            sf::RectangleShape traco({12.f, 3.f});
            traco.setOrigin({6.f, 0.f});
            traco.setPosition(c + sf::Vector2f(0, 3));
            traco.setFillColor(cor);
            janela.draw(traco);
        } else {                                        // fechar
            for (float ang : {45.f, -45.f}) {
                sf::RectangleShape traco({15.f, 3.f});
                traco.setOrigin({7.5f, 1.5f});
                traco.setPosition(c);
                traco.setRotation(sf::degrees(ang));
                traco.setFillColor(cor);
                janela.draw(traco);
            }
        }
    }
}

void Jogo::desenharGaiola() {
    sf::Vector2f g = mapa.centro(mapa.posGatinho);
    bool livre = tela == Tela::FASE_COMPLETA;
    int cor = fase - 1;                               // cada fase tem um pet diferente
    const sf::Color ouro(255, 214, 120), ouroEscuro(200, 140, 60);

    // fundo da gaiola
    sf::RectangleShape fundo({TILE - 4.f, TILE - 2.f});
    fundo.setOrigin({(TILE - 4.f) / 2, (TILE - 2.f) / 2});
    fundo.setPosition(g);
    fundo.setFillColor(sf::Color(24, 16, 44, 225));
    janela.draw(fundo);

    sf::Vector2f pe = posGatinhoAgora();
    if (livre) {
        bool correndo = tempoNaTela < 0.8f;
        desenharGato(cor, pe, tempoTotal, correndo ? GATO_CORRENDO : GATO_SENTADO, 1.15f, correndo);
    } else if (gaiolaAberta) {
        pe.y -= std::abs(std::sin(tempoTotal * 8.f)) * 4.f;       // pula se vai ser salvo
        desenharGato(cor, pe, tempoTotal * 2.f, GATO_SENTADO, 0.95f);
    } else {
        desenharGato(cor, pe, tempoTotal, chaveVisivel ? GATO_SENTADO : GATO_DORMINDO, 0.95f);
    }

    if (!livre) {
        // grades
        for (int i = 0; i < 5; i++) {
            sf::RectangleShape barra({2.5f, TILE - 8.f});
            barra.setPosition(g + sf::Vector2f(-16.25f + i * 7.5f, -TILE / 2.f + 4));
            barra.setFillColor(ouro);
            janela.draw(barra);
        }
        sf::RectangleShape trilho({TILE - 2.f, 4.f});
        trilho.setOrigin({(TILE - 2.f) / 2, 0});
        trilho.setFillColor(ouroEscuro);
        trilho.setPosition(g + sf::Vector2f(0, -TILE / 2.f + 1));
        janela.draw(trilho);
        trilho.setPosition(g + sf::Vector2f(0, TILE / 2.f - 5));
        janela.draw(trilho);
        sf::CircleShape argola(4.f);
        argola.setOrigin({4.f, 4.f});
        argola.setPosition(g + sf::Vector2f(0, -TILE / 2.f - 2));
        argola.setFillColor(sf::Color::Transparent);
        argola.setOutlineColor(ouro);
        argola.setOutlineThickness(2.f);
        janela.draw(argola);
    }

    // portinha
    if (!gaiolaAberta) {
        sf::Vector2f p = mapa.centro(mapa.posPortaGaiola);
        float pulso = chaveVisivel ? 1.f + 0.12f * std::sin(tempoTotal * 8.f) : 1.f;

        for (int i = 0; i < 3; i++)                   // 3 travessas listradas
            for (int k = 0; k < 6; k++) {
                sf::RectangleShape listra({5.f, 6.f});
                listra.setPosition({p.x - 15.f + k * 5.f, p.y - 14.f + i * 11.f});
                listra.setFillColor((k + i) % 2 ? sf::Color(255, 250, 250) : sf::Color(240, 85, 125));
                janela.draw(listra);
            }
        for (float lado : {-1.f, 1.f}) {              // postes dourados com bolinha em cima
            sf::RectangleShape poste({5.f, TILE - 4.f});
            poste.setOrigin({2.5f, (TILE - 4.f) / 2});
            poste.setPosition(p + sf::Vector2f(lado * (TILE / 2.f - 3.f), 0));
            poste.setFillColor(ouroEscuro);
            janela.draw(poste);
            sf::CircleShape bola(4.f);
            bola.setOrigin({4.f, 4.f});
            bola.setPosition(p + sf::Vector2f(lado * (TILE / 2.f - 3.f), -TILE / 2.f + 3));
            bola.setFillColor(ouro);
            janela.draw(bola);
        }
        sf::CircleShape cadeado(9.f * pulso);         // cadeado redondo com coracao
        cadeado.setOrigin({9.f * pulso, 9.f * pulso});
        cadeado.setPosition(p + sf::Vector2f(0, 2));
        cadeado.setFillColor(ouro);
        cadeado.setOutlineColor(ouroEscuro);
        cadeado.setOutlineThickness(2.f);
        janela.draw(cadeado);
        desenharCoracao(janela, p + sf::Vector2f(0, 1.5f), 5.f * pulso, sf::Color(230, 60, 105));
    }
}

// HUD

void Jogo::desenharHUD() {
    float L = float(janela.getSize().x);

    sf::RectangleShape faixa({L + 40, float(HUD_ALTURA)});
    faixa.setPosition({-20, 0});
    faixa.setFillColor(sf::Color(20, 12, 40, 200));
    janela.draw(faixa);

    escrever(T("PONTOS", "SCORE"), 13, {110, 20}, sf::Color(255, 190, 220));
    escrever(std::to_string(pontos), 26, {110, 48}, sf::Color::White, 1.f + 0.25f * popPontos);

    escrever(T("Fase ", "Level ") + std::to_string(fase) + T(" de ", " of ") + std::to_string(NUM_FASES), 16, {L / 2, 18},
             sf::Color(255, 230, 160));

    // barrinha de docinhos comidos (ou aviso da chave)
    float largBarra = 260;
    sf::RectangleShape barra({largBarra, 10});
    barra.setPosition({L / 2 - largBarra / 2, 34});
    barra.setFillColor(sf::Color(60, 40, 90));
    janela.draw(barra);
    float progresso = docesTotal > 0 ? 1.f - float(docesRestantes) / docesTotal : 0.f;
    barra.setSize({largBarra * progresso, 10});
    barra.setFillColor(sf::Color(255, 160, 210));
    janela.draw(barra);
    std::string status = gaiolaAberta ? T("Salve o ", "Save the ") + nomePet(fase) + "!" :
                         chaveVisivel ? T("Pegue a chave!", "Grab the key!") :
                                        std::to_string(docesRestantes) + T(" docinhos", " candies");
    escrever(status, 12, {L / 2, 58}, sf::Color(220, 210, 240));

    // poderes ativos: icone com uma barra de tempo embaixo
    struct PoderAtivo { float resta, total; int icone; sf::Color cor; };   // icone -1 = boba tea
    const PoderAtivo ativos[4] = {{tempoPoder, dif().tempoPoder, -1, sf::Color(130, 160, 255)},
                                  {tempoTurbo, 5.f, PIRULITO_ARCO_IRIS, corArcoIris(tempoTotal)},
                                  {tempoIma, 7.f, PIRULITO_ROSA, sf::Color(255, 150, 195)},
                                  {tempoHipnose, 4.5f, PIRULITO_VERMELHO, sf::Color(240, 80, 115)}};
    float xPoder = 220;
    for (const PoderAtivo& a : ativos) {
        if (a.resta <= 0) continue;
        if (a.icone >= 0) desenharPirulito(a.icone, {xPoder, 28}, 34.f);
        else desenharBoba({xPoder, 30}, 0.f);
        sf::RectangleShape fundoBarra({38.f, 5.f});
        fundoBarra.setPosition({xPoder - 19, 52});
        fundoBarra.setFillColor(sf::Color(60, 40, 90));
        janela.draw(fundoBarra);
        fundoBarra.setSize({38.f * a.resta / a.total, 5.f});
        fundoBarra.setFillColor(a.cor);
        janela.draw(fundoBarra);
        xPoder += 52;
    }

    // corridinh: barra enche enquanto recarrega
    float carga = 1.f - recargaCorrida / 7.f;
    bool pronta = carga >= 1.f;
    escrever(T("ESPAÇO", "SPACE"), 11, {L - 250, 20}, pronta ? sf::Color(255, 240, 160) : sf::Color(170, 160, 200));
    sf::RectangleShape barraCorrida({80.f, 8.f});
    barraCorrida.setPosition({L - 290, 34});
    barraCorrida.setFillColor(sf::Color(60, 40, 90));
    janela.draw(barraCorrida);
    barraCorrida.setSize({80.f * carga, 8.f});
    barraCorrida.setFillColor(pronta ? sf::Color(255, 230, 120) : sf::Color(150, 140, 190));
    janela.draw(barraCorrida);
    escrever(pronta ? T("corridinha", "dash") : T("recarregando", "recharging"), 10, {L - 250, 56}, sf::Color(200, 190, 230));

    // vidas em coracoes
    for (int i = 0; i < std::min(vidas, 6); i++)
        desenharCoracao(janela, {L - 40.f - i * 30.f, 26}, 10.f, sf::Color(255, 100, 150));
    escrever(T("RECORDE ", "BEST ") + std::to_string(recorde), 12, {L - 100, 54}, sf::Color(200, 190, 230));
}

// telas

void Jogo::desenharMenu() {
    float L = float(janela.getSize().x);
    float t = tempoTotal;
    const int ciclo[4] = {0, 1, 2, 1};
    const sf::Color painelFundo(30, 18, 60, 165);

    // balas subindo no fundo
    for (int i = 0; i < 18; i++) {
        float x = std::fmod(i * 137.f + 60.f, L);
        float y = 710.f - std::fmod(t * (14.f + (i % 5) * 4.f) + i * 91.f, 760.f);
        desenharBala({x + std::sin(t * 0.8f + i) * 14.f, y}, i % 5, t + i, 1.7f);
    }

    //logotipo
    const std::string titulo = "CandyRun";
    const unsigned tam = 92;
    float largura = 520;
    if (temFonte) {
        largura = 0;
        for (char c : titulo) largura += fonte.getGlyph(std::uint32_t(c), tam, false).advance;

        sf::CircleShape brilho(170.f);
        brilho.setOrigin({170.f, 170.f});
        brilho.setScale({1.9f, 0.42f});
        brilho.setPosition({L / 2, 96});
        brilho.setFillColor(sf::Color(255, 150, 200, 34));
        janela.draw(brilho);

        const sf::Color cores[4] = {{255, 120, 170}, {255, 205, 110}, {130, 225, 210}, {190, 160, 255}};
        float x = L / 2 - largura / 2;
        for (std::size_t i = 0; i < titulo.size(); i++) {
            float y = 36 + std::sin(t * 3.f + i * 0.6f) * 7.f;
            sf::Text letra(fonte, sf::String(titulo[i]), tam);
            letra.setFillColor(sf::Color(50, 20, 80));           // sombra
            letra.setPosition({x + 5, y + 7});
            janela.draw(letra);
            letra.setFillColor(cores[i % 4]);
            letra.setOutlineColor(sf::Color::White);
            letra.setOutlineThickness(3.f);
            letra.setPosition({x, y});
            janela.draw(letra);
            x += fonte.getGlyph(std::uint32_t(titulo[i]), tam, false).advance;
        }
    }
    desenharPirulito(PIRULITO_ARCO_IRIS, {L / 2 - largura / 2 - 62, 96}, 96.f, std::sin(t * 2.f) * 12.f);
    desenharPirulito(PIRULITO_VERMELHO, {L / 2 + largura / 2 + 62, 96}, 96.f, -std::sin(t * 2.f + 1.f) * 12.f);
    desenharGato(0, {L / 2 + largura / 2 - 60, 62}, t, GATO_DORMINDO, 1.5f);   // cachorro dormindo no titulo

    desenharPainel({L / 2, 166}, {600, 30}, painelFundo, sf::Color(255, 170, 210, 170));
    escrever(T("Pegue os doces, ache a chave e salve os bichinhos!", "Grab the candy, find the key and save the pets!"), 14, {L / 2, 166}, sf::Color(255, 240, 248));

    desenharEstrela(janela, {40, 28}, 9.f, sf::Color(255, 225, 120), t);
    escrever(T("Recorde ", "Best ") + std::to_string(recorde), 14, {130, 28}, sf::Color(255, 235, 170));

    // botao de idioma (tecla L)
    desenharPainel({L - 125, 28}, {210, 30}, painelFundo, sf::Color(170, 230, 255, 170));
    escrever(T("L: Português", "L: English"), 13, {L - 125, 28}, sf::Color(220, 245, 255));

    // desfile: as rivais correm atras; depois o jogo vira
    {
        float u = std::fmod(t, 14.f) / 14.f;
        bool indo = u < 0.5f;
        float v = indo ? u * 2.f : (u - 0.5f) * 2.f;
        float x0 = indo ? -420.f + v * (L + 560.f) : L + 420.f - v * (L + 560.f);
        Direcao dir = indo ? DIREITA : ESQUERDA;
        float sinal = indo ? 1.f : -1.f;
        const float y = 272, e = 1.25f;

        desenharGato(1, {x0 + sinal * 330, y + 2}, t, GATO_CORRENDO, 1.3f, indo);
        sf::Vector2f pj = {x0 + sinal * (indo ? 250.f : -60.f), y};
        desenharSombra(janela, pj, sf::Color(0, 0, 0, 90), e);
        desenharSprite(SPRITE_JOGADORA, pj, dir, ciclo[int(t * 9) % 4],
                       indo ? sf::Color::White : misturar(sf::Color::White, corArcoIris(t * 2.f), 0.5f), e);
        for (int i = 0; i < 4; i++) {
            sf::Vector2f pr = {x0 + sinal * (indo ? 150.f - i * 62.f : 60.f + i * 62.f), y};
            desenharSombra(janela, pr, sf::Color(rivais[i].cor.r, rivais[i].cor.g, rivais[i].cor.b, 150), e);
            desenharSprite(rivais[i].sprite, pr, dir, ciclo[int(t * 9 + i) % 4],
                           indo ? sf::Color::White : sf::Color(165, 190, 255), e);
        }
    }

    //  cartoes das rivais
    for (int i = 0; i < 4; i++) {
        float x = L / 2 + (i - 1.5f) * 245.f;
        sf::Color cor = rivais[i].cor;
        desenharPainel({x, 388}, {222, 150}, painelFundo, sf::Color(cor.r, cor.g, cor.b, 210));
        sf::Vector2f pe = {x, 400};
        desenharSombra(janela, pe, sf::Color(cor.r, cor.g, cor.b, 150), 1.5f);
        Direcao olhar = (int(t / 1.2f + i) % 2 == 0) ? BAIXO : (i < 2 ? DIREITA : ESQUERDA);
        desenharSprite(rivais[i].sprite, pe, olhar, ciclo[int(t * 5 + i) % 4], sf::Color::White, 1.5f);
        escrever(rivais[i].nome, 20, {x, 422}, cor);
        escrever(rivais[i].descricao, 11, {x, 446}, sf::Color(225, 215, 245));
    }

    // legenda dos poderes
    desenharPainel({L / 2, 512}, {1000, 54}, painelFundo, sf::Color(255, 220, 140, 150));
    const std::string nomes[3] = {T("Turbo invencível", "Invincible turbo"), T("Ímã de doces", "Candy magnet"),
                                  T("Hipnose nas rivais", "Hypnotize rivals")};
    const float xs[3] = {70, 320, 545};
    for (int i = 0; i < 3; i++) {
        desenharPirulito(i, {xs[i], 512 - std::abs(std::sin(t * 3.f + i)) * 3.f}, 42.f);
        escrever(nomes[i], 12, {xs[i] + 125, 512}, sf::Color(255, 240, 248));
    }
    desenharBoba({815, 512}, t);
    escrever(T("Boba: assusta", "Boba: scares"), 12, {900, 512}, sf::Color(255, 240, 248));

    // botao de comecar
    float pulso = 1.f + 0.04f * std::sin(t * 5.f);
    desenharPainel({L / 2, 582}, {380 * pulso, 50 * pulso}, sf::Color(255, 110, 165, 235), sf::Color::White);
    escrever(T("ENTER para jogar", "Press ENTER to play"), 22, {L / 2, 582}, sf::Color::White, pulso);

    escrever(T("Setas ou WASD: andar    Espaço: corridinha    P: pausa    M: som    L: idioma    Esc: sair",
               "Arrows or WASD: move    Space: dash    P: pause    M: sound    L: language    Esc: quit"), 12, {L / 2, 642},
             sf::Color(200, 190, 230));
}

void Jogo::desenharTelaFinal(bool venceu) {
    float L = float(janela.getSize().x);
    float t = tempoNaTela;

    if (venceu) {
        escrever(T("Você salvou todos os bichinhos!", "You saved all the pets!"), 34, {L / 2, 120}, sf::Color(255, 200, 230),
                 1.f + 0.05f * std::sin(t * 4.f));
        sf::Vector2f pe = {L / 2, 380};
        desenharSombra(janela, pe, sf::Color(0, 0, 0, 90), 2.4f);
        Direcao danca[4] = {BAIXO, ESQUERDA, BAIXO, DIREITA};
        desenharSprite(SPRITE_JOGADORA, pe, danca[int(t * 2) % 4], 1 + int(std::sin(t * 8) > 0), sf::Color::White,
                       2.4f);

        desenharGato(0, {L / 2 - 170, 384 - std::abs(std::sin(t * 5.f)) * 10.f}, t, GATO_SENTADO, 2.2f, true);
        desenharGato(1, {L / 2 + 160, 384 - std::abs(std::sin(t * 5.f + 1.f)) * 10.f}, t + 0.3f, GATO_SENTADO, 2.2f);
        desenharGato(2, {L / 2 + 290, 396 - std::abs(std::sin(t * 5.f + 2.f)) * 10.f}, t + 0.6f, GATO_SENTADO, 2.2f);
    } else {
        escrever(T("Fim de jogo", "Game over"), 48, {L / 2, 150}, sf::Color(255, 160, 190));
        sf::Vector2f pe = {L / 2 - 40, 380};
        desenharSombra(janela, pe, sf::Color(0, 0, 0, 90), 2.4f);
        desenharSprite(SPRITE_JOGADORA, pe, BAIXO, 1, sf::Color(200, 190, 230), 2.4f);
        desenharGato(fase - 1, {L / 2 + 90, 384}, t, GATO_DORMINDO, 2.f);
        escrever(T("O " + nomePet(fase) + " ainda espera por você...", "The " + nomePet(fase) + " is still waiting for you..."), 18, {L / 2, 420}, sf::Color(220, 210, 240));
    }

    escrever(T("Pontos: ", "Score: ") + std::to_string(pontos), 28, {L / 2, 480}, sf::Color::White);
    if (novoRecorde && std::fmod(t, 0.8f) < 0.55f)
        escrever(T("Novo recorde!", "New best score!"), 24, {L / 2, 520}, sf::Color(255, 230, 120));
    else if (!novoRecorde)
        escrever(T("Recorde: ", "Best: ") + std::to_string(recorde), 18, {L / 2, 520}, sf::Color(200, 190, 230));
    if (t > 1.f) escrever(T("ENTER para voltar ao menu", "Press ENTER for the menu"), 18, {L / 2, 600}, sf::Color(255, 240, 160));
}

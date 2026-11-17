// movimento, rivais, pontos, fases e telas.
#include "jogo.hpp"
#include "janela_sistema.hpp"
#include <fstream>
#include <iostream>

// uma linha por fase
const Dificuldade DIFICULDADES[NUM_FASES] = {
    {6.5f, 5.4f, 3.3f, 7.0f, {0.f, 2.f, 6.f, 10.f}, {7, 20, 7, 20, 5, 20, 5},
     {196, 160, 255}, {116, 84, 186}, {44, 32, 72}, {58, 36, 96}, {22, 16, 44}},
    {7.0f, 6.2f, 3.5f, 5.0f, {0.f, 1.f, 4.f, 7.f}, {6, 20, 6, 20, 5, 25, 4},
     {140, 225, 200}, {66, 148, 138}, {28, 44, 60}, {26, 58, 84}, {12, 20, 40}},
    {7.5f, 7.0f, 3.7f, 3.5f, {0.f, 0.5f, 2.f, 4.f}, {5, 22, 5, 22, 3, 30, 2},
     {255, 165, 195}, {186, 90, 130}, {52, 28, 56}, {86, 34, 84}, {26, 12, 36}},
};

const Dificuldade& Jogo::dif() const { return DIFICULDADES[fase - 1]; }

//  inicio

Jogo::Jogo() {
    if (!mapa.carregar("assets/fases/fase1.txt")) {
        std::cerr << "Nao achei assets/fases/fase1.txt. Rode o jogo de dentro da pasta do projeto.\n";
        std::exit(1);
    }
    unsigned largura = unsigned(mapa.largura * TILE);
    unsigned altura = unsigned(HUD_ALTURA + mapa.altura * TILE + 8);
    tamJogo = {float(largura), float(altura)};
    // sem a barra do Windows: o jogo desenha a sua (desenharBarraTitulo)
    janela.create(sf::VideoMode({largura, altura + BARRA_ALTURA}), "CandyRun", sf::Style::None);
    prepararJanela(janela.getNativeHandle());
    sf::Image icone;
    if (icone.loadFromFile("assets/icone.png")) janela.setIcon(icone);
    janela.setKeyRepeatEnabled(false);
    janela.setFramerateLimit(60);

    temFonte = fonte.openFromFile("assets/fonte.ttf");
    for (int i = 0; i < 9; i++) {
        temSheet[i] = sheets[i].loadFromFile("assets/sprites/kawaii" + std::to_string(i + 1) + ".png");
        if (temSheet[i]) {
            sheets[i].setSmooth(true);
            (void)sheets[i].generateMipmap();     // fica bonito mesmo reduzido
        }
    }
    for (int i = 0; i < 3; i++) {
        temGato[i] = texGato[i].loadFromFile("assets/sprites/pet" + std::to_string(i + 1) + ".png");
        if (temGato[i]) {
            texGato[i].setSmooth(true);
            (void)texGato[i].generateMipmap();
        }
    }
    temPirulitos = texPirulitos.loadFromFile("assets/sprites/pirulitos.png");
    if (temPirulitos) {
        texPirulitos.setSmooth(true);
        (void)texPirulitos.generateMipmap();
    }
    if (!labirinto.resize({largura, unsigned(mapa.altura * TILE + 8)}))
        std::cerr << "Nao consegui criar a textura do labirinto\n";

    for (int i = 0; i < 80; i++)
        estrelas.push_back({{sortear(0.f, float(largura)), sortear(0.f, float(altura))},
                            sortear(0.f, 6.28f), sortear(1.f, 2.5f)});

    // As 4 rivais
    rivais[0].sprite = 6;
    rivais[0].cor = sf::Color(255, 80, 110);
    rivais[0].jeito = Personalidade::PERSEGUE;

    rivais[1].sprite = 1;
    rivais[1].cor = sf::Color(255, 150, 210);
    rivais[1].jeito = Personalidade::EMBOSCA;

    rivais[2].sprite = 5;
    rivais[2].cor = sf::Color(140, 210, 255);
    rivais[2].jeito = Personalidade::CERCA;

    rivais[3].sprite = 3;
    rivais[3].cor = sf::Color(180, 130, 255);
    rivais[3].jeito = Personalidade::TIMIDA;

    sons.carregar();
    carregarRecorde();
    carregarIdioma();
    irPara(Tela::MENU);
}

void Jogo::novoJogo() {
    fase = 1;
    vidas = 3;
    pontos = 0;
    novoRecorde = false;
    ganhouVidaExtra = false;
    carregarFase(1);
}

void Jogo::carregarFase(int n) {
    fase = n;
    if (!mapa.carregar("assets/fases/fase" + std::to_string(n) + ".txt")) {
        std::cerr << "Nao achei o mapa da fase " << n << "\n";
        std::exit(1);
    }
    docesTotal = docesRestantes = mapa.contarDoces();
    chaveVisivel = false;
    gaiolaAberta = false;

    // cantos pra onde cada rival vai no modo "espalhar" (fora do mapa de proposito)
    rivais[0].canto = {mapa.largura - 3, -3};
    rivais[1].canto = {2, -3};
    rivais[2].canto = {mapa.largura - 1, mapa.altura + 2};
    rivais[3].canto = {0, mapa.altura + 2};

    // desenha as paredes uma vez so numa textura (bem mais leve que redesenhar)
    labirinto.clear(sf::Color::Transparent);
    mapa.desenharLabirinto(labirinto, dif().paredeTopo, dif().paredeLado, dif().chao);
    labirinto.display();

    efeitos.limpar();
    reiniciarPosicoes();
    prontoLongo = true;
    irPara(Tela::PRONTO);
}

void Jogo::reiniciarPosicoes() {
    jogadora.colocarEm(mapa.inicioJogadora, BAIXO);
    jogadora.desejada = NENHUMA;

    sf::Vector2i acimaDaPorta = passo(mapa.posPortaCasa, CIMA);
    int meio = int(mapa.vagasCasa.size()) / 2;
    sf::Vector2i vagas[4] = {acimaDaPorta, mapa.vagasCasa[meio],
                             mapa.vagasCasa[std::max(0, meio - 2)],
                             mapa.vagasCasa[std::min(int(mapa.vagasCasa.size()) - 1, meio + 2)]};

    for (int i = 0; i < 4; i++) {
        Rival& r = rivais[i];
        r.vagaNaCasa = vagas[i];
        r.colocarEm(vagas[i], i == 0 ? ESQUERDA : BAIXO);
        r.estado = (i == 0) ? EstadoRival::ATIVA : EstadoRival::EM_CASA;
        r.assustada = false;
        r.tempoParaSair = dif().sairDaCasa[i];
    }

    tempoPoder = 0;
    comboRivais = 0;
    indiceModo = 0;
    tempoModo = 0;
    modoPerseguir = false;
    congelado = 0;
    efeitoMorteFeito = false;

    tempoTurbo = tempoIma = tempoHipnose = tempoCorrida = recargaCorrida = 0;
    pirulitoVisivel = false;
    tempoPirulito = 8;
}

void Jogo::irPara(Tela t) {
    tela = t;
    tempoNaTela = 0;
    if (t == Tela::MENU || t == Tela::PRONTO || t == Tela::VITORIA || t == Tela::FIM_DE_JOGO)
        escurecer = (t == Tela::PRONTO && !prontoLongo) ? 0.f : 1.f;

    bool comMusica = (t == Tela::MENU || t == Tela::PRONTO || t == Tela::JOGANDO);
    sons.musica(comMusica);
}

//laco principal

void Jogo::rodar() {
    sf::Clock relogio;
    while (janela.isOpen()) {
        processarEventos();
        // arrastando pela barra de titulo: a janela segue o mouse
        if (arrastando && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
            janela.setPosition(sf::Mouse::getPosition() - pegaDoArraste);
        else
            arrastando = false;
        float dt = relogio.restart().asSeconds();
        dt = std::min(dt, 1.f / 20.f);           // se o PC travar, nao teleporta ninguem
        atualizar(dt);
        desenhar();
    }
    salvarRecorde();
}

void Jogo::processarEventos() {
    while (const std::optional evento = janela.pollEvent()) {
        if (evento->is<sf::Event::Closed>()) janela.close();
        if (const auto* tecla = evento->getIf<sf::Event::KeyPressed>()) teclaApertada(tecla->scancode);
        if (const auto* clique = evento->getIf<sf::Event::MouseButtonPressed>())
            if (clique->button == sf::Mouse::Button::Left && clique->position.y < BARRA_ALTURA)
                cliqueNaBarra(clique->position);
    }
}

void Jogo::teclaApertada(sf::Keyboard::Scancode tecla) {
    using S = sf::Keyboard::Scancode;

    if (tecla == S::M) sons.alternarMudo();
    if (tecla == S::L && (tela == Tela::MENU || tela == Tela::PAUSA)) trocarIdioma();

    switch (tela) {
        case Tela::MENU:
            if (tecla == S::Enter || tecla == S::Space) {
                sons.tocar(Som::MENU);
                novoJogo();
            }
            if (tecla == S::Escape) janela.close();
            break;

        case Tela::PRONTO:
        case Tela::JOGANDO:
            if (tecla == S::Up || tecla == S::W) jogadora.desejada = CIMA;
            if (tecla == S::Down || tecla == S::S) jogadora.desejada = BAIXO;
            if (tecla == S::Left || tecla == S::A) jogadora.desejada = ESQUERDA;
            if (tecla == S::Right || tecla == S::D) jogadora.desejada = DIREITA;
            if (tecla == S::Space && tela == Tela::JOGANDO) usarCorrida();
            if (tecla == S::P || tecla == S::Escape) {
                telaAntesDaPausa = tela;
                float t = tempoNaTela;
                irPara(Tela::PAUSA);
                tempoNaTela = t;
            }
            break;

        case Tela::PAUSA:
            if (tecla == S::P || tecla == S::Escape) {
                float t = tempoNaTela;
                irPara(telaAntesDaPausa);
                tempoNaTela = t;
            }
            if (tecla == S::Q) {
                salvarRecorde();
                irPara(Tela::MENU);
            }
            break;

        case Tela::VITORIA:
        case Tela::FIM_DE_JOGO:
            if ((tecla == S::Enter || tecla == S::Space) && tempoNaTela > 1.f) irPara(Tela::MENU);
            break;

        default:
            break;
    }
}

void Jogo::atualizar(float dt) {
    tempoTotal += dt;
    if (tela != Tela::PAUSA) tempoNaTela += dt;
    escurecer = std::max(0.f, escurecer - dt * 2.5f);
    popPontos = std::max(0.f, popPontos - dt * 4.f);
    tempoAviso = std::max(0.f, tempoAviso - dt);
    if (tela != Tela::PAUSA) efeitos.atualizar(dt);

    switch (tela) {
        case Tela::PRONTO:
            if (tempoNaTela > (prontoLongo ? 2.2f : 1.3f)) irPara(Tela::JOGANDO);
            break;

        case Tela::JOGANDO:
            if (congelado > 0) {            // "hit stop" ao comer uma rival
                congelado -= dt;
                break;
            }
            atualizarPoderes(dt);
            atualizarJogadora(dt);
            if (tela != Tela::JOGANDO) break;     // pode ter completado a fase
            atualizarRivais(dt);
            verificarEncontros();
            break;

        case Tela::MORRENDO:
            if (tempoNaTela > 0.45f && !efeitoMorteFeito) {
                efeitoMorteFeito = true;
                sf::Vector2f p = jogadora.posicao(mapa);
                efeitos.explosao(p, 14, sf::Color(200, 190, 220), 140, true);
            }
            if (tempoNaTela > 2.0f) {
                if (vidas <= 0) {
                    salvarRecorde();
                    irPara(Tela::FIM_DE_JOGO);
                } else {
                    reiniciarPosicoes();
                    prontoLongo = false;
                    irPara(Tela::PRONTO);
                }
            }
            break;

        case Tela::FASE_COMPLETA:
            if (std::fmod(tempoNaTela, 0.25f) < dt) {      // coracoes saindo do gatinho
                efeitos.explosao(posGatinhoAgora() + sf::Vector2f(0, -22), 3, sf::Color(255, 130, 180), 90, true);
            }
            if (tempoNaTela > 3.4f) {
                if (fase < NUM_FASES) {
                    carregarFase(fase + 1);
                } else {
                    salvarRecorde();
                    irPara(Tela::VITORIA);
                }
            }
            break;

        case Tela::VITORIA:
            if (std::fmod(tempoNaTela, 0.4f) < dt)
                efeitos.explosao({sortear(100.f, janela.getSize().x - 100.f), sortear(100.f, 300.f)}, 8,
                                 misturar(sf::Color(255, 140, 190), sf::Color(160, 220, 255), sortear(0.f, 1.f)),
                                 160, sortearInt(0, 1) == 1);
            break;

        default:
            break;
    }
}

// jogadora

void Jogo::atualizarJogadora(float dt) {
    Jogadora& j = jogadora;
    auto livre = [&](Direcao d) {
        return d != NENHUMA && mapa.livreParaJogadora(passo(j.casa, d), gaiolaAberta);
    };

    if (!j.andando) {
        if (livre(j.desejada)) j.andarPara(j.desejada);
        else {
            if (j.desejada != NENHUMA) j.dir = j.desejada;  
            return;
        }
    }

    //  nao precisa esperar chegar na casa
    if (j.desejada == oposta(j.dir)) j.inverter();

    if (j.avancar(velocidadeJogadora(), dt, mapa)) {
        chegouNaCasa();
        if (tela != Tela::JOGANDO) return;

        if (livre(j.desejada)) j.andarPara(j.desejada);      // vira se der
        else if (livre(j.dir)) j.andarPara(j.dir);          // senao continua reto
        else {                                              // senao para na parede
            j.andando = false;
            j.progresso = 0;
        }
    }
}

// Come o docinho (ou boba tea) que estiver nessa casa
void Jogo::comerDoce(sf::Vector2i c) {
    sf::Vector2f p = mapa.centro(c);
    char& doce = mapa.doces[c.y][c.x];

    if (doce == '.') {
        doce = ' ';
        docesRestantes--;
        ganharPontos(10);
        if (tempoDesdeDoce > 0.6f) notaDoce = 0;     
        sons.tocarDoce(notaDoce++);
        tempoDesdeDoce = 0;
        const sf::Color cores[4] = {{255, 150, 190}, {150, 235, 215}, {255, 225, 130}, {190, 160, 255}};
        efeitos.explosao(p, 4, cores[(c.x * 7 + c.y * 3) % 4], 70);
    } else if (doce == 'o') {
        doce = ' ';
        docesRestantes--;
        ganharPontos(50);
        ativarPoder();
        sons.tocar(Som::BOBA);
        efeitos.explosao(p, 22, sf::Color(235, 195, 150), 170);
        efeitos.anel(p, 18, sf::Color(255, 255, 255), 30);
        efeitos.tremer(3);
        mostrarAviso(T("Boba tea! Hora do troco!", "Boba tea! Payback time!"));
    } else {
        return;
    }

    if (docesRestantes == 0 && !chaveVisivel && !gaiolaAberta) {
        chaveVisivel = true;
        sons.tocar(Som::PORTA);
        efeitos.explosao(mapa.centro(mapa.posChave), 26, sf::Color(255, 220, 90), 170);
        mostrarAviso(T("Uma chave apareceu!", "A key appeared!"));
    }
}

void Jogo::chegouNaCasa() {
    sf::Vector2i c = jogadora.casa;
    sf::Vector2f p = mapa.centro(c);

    comerDoce(c);
    if (pirulitoVisivel && c == pirulitoPos) pegarPirulito();

    if (chaveVisivel && c == mapa.posChave) {
        chaveVisivel = false;
        gaiolaAberta = true;
        ganharPontos(500);
        efeitos.textoSubindo(p + sf::Vector2f(0, -24), "+500", sf::Color(255, 220, 90));
        efeitos.explosao(mapa.centro(mapa.posPortaGaiola), 30, sf::Color(255, 220, 90), 200);
        efeitos.tremer(4);
        sons.tocar(Som::CHAVE);
        mostrarAviso(T("A gaiola abriu! Corre!", "The cage is open! Run!"));
    }

    if (gaiolaAberta && c == mapa.posGatinho) {
        int bonus = 1000 * fase;
        ganharPontos(bonus);
        efeitos.textoSubindo(p + sf::Vector2f(0, -30), "+" + std::to_string(bonus), sf::Color(255, 160, 200));
        for (auto& r : rivais) efeitos.explosao(r.posicao(mapa), 16, r.cor, 140);   // rivais somem num "puf"
        efeitos.explosao(mapa.centro(mapa.posGatinho), 24, sf::Color(255, 215, 120), 180);   // gaiola quebra
        tempoPoder = tempoTurbo = tempoIma = tempoHipnose = tempoCorrida = 0;
        pirulitoVisivel = false;
        sons.tocar(Som::FASE);
        jogadora.dir = BAIXO;
        jogadora.andando = false;
        irPara(Tela::FASE_COMPLETA);
    }
}

// poderes

float Jogo::velocidadeJogadora() const {
    float v = dif().velJogadora;
    if (tempoTurbo > 0) v *= 1.45f;
    if (tempoCorrida > 0) v *= 1.7f;
    return v;
}

// Corridinha:
void Jogo::usarCorrida() {
    if (recargaCorrida > 0) return;
    tempoCorrida = 0.9f;
    recargaCorrida = 7.f;
    sons.tocar(Som::CORRIDA);
    efeitos.anel(jogadora.posicao(mapa), 12, sf::Color(255, 255, 255), 22);
}

void Jogo::atualizarPoderes(float dt) {
    tempoTurbo = std::max(0.f, tempoTurbo - dt);
    tempoIma = std::max(0.f, tempoIma - dt);
    tempoHipnose = std::max(0.f, tempoHipnose - dt);
    tempoCorrida = std::max(0.f, tempoCorrida - dt);
    recargaCorrida = std::max(0.f, recargaCorrida - dt);
    tempoDesdeDoce += dt;

    // o pirulito aparece num lugar sorteado, fica um tempo e some
    tempoPirulito -= dt;
    if (tempoPirulito <= 0) {
        if (pirulitoVisivel) {
            pirulitoVisivel = false;
            tempoPirulito = 9.f;
        } else {
            sortearPirulito();
        }
    }

    sf::Vector2f pj = jogadora.posicao(mapa);

    // rastro de particulas enquanto corre
    if (jogadora.andando) {
        if (tempoTurbo > 0) efeitos.explosao(pj + sf::Vector2f(0, 8), 1, corArcoIris(tempoTotal * 2.f), 35);
        else if (tempoCorrida > 0) efeitos.explosao(pj + sf::Vector2f(0, 10), 1, sf::Color(255, 255, 255), 30);
    }

    // ima: come sozinha os docinhos que estiverem perto
    if (tempoIma > 0) {
        for (int y = jogadora.casa.y - 3; y <= jogadora.casa.y + 3; y++)
            for (int x = jogadora.casa.x - 3; x <= jogadora.casa.x + 3; x++) {
                if (y < 0 || y >= mapa.altura || x < 0 || x >= mapa.largura) continue;
                if (mapa.doces[y][x] != '.') continue;
                if (distancia(mapa.centro(sf::Vector2i{x, y}), pj) < 2.6f * TILE) comerDoce({x, y});
            }
    }
}

void Jogo::sortearPirulito() {
    std::vector<sf::Vector2i> livres;
    for (int y = 0; y < mapa.altura; y++)
        for (int x = 0; x < mapa.largura; x++) {
            sf::Vector2i p{x, y};
            if (!mapa.livreParaJogadora(p, false) || p == mapa.posChave) continue;
            if (std::abs(x - jogadora.casa.x) + std::abs(y - jogadora.casa.y) < 6) continue;   // nao nasce colado
            livres.push_back(p);
        }
    if (livres.empty()) return;
    pirulitoPos = livres[sortearInt(0, int(livres.size()) - 1)];
    pirulitoTipo = sortearInt(0, 2);
    pirulitoVisivel = true;
    tempoPirulito = 10.f;
    efeitos.anel(mapa.centro(pirulitoPos), 10, sf::Color(255, 200, 230), 18);
}

void Jogo::pegarPirulito() {
    pirulitoVisivel = false;
    tempoPirulito = 9.f;
    ganharPontos(100);
    sons.tocar(Som::PIRULITO);
    sf::Vector2f p = jogadora.posicao(mapa);
    efeitos.tremer(3);

    switch (pirulitoTipo) {
        case PIRULITO_ARCO_IRIS:
            tempoTurbo = 5.f;
            for (int i = 0; i < 24; i++) efeitos.explosao(p, 1, corArcoIris(i / 24.f), 190);
            mostrarAviso(T("Turbo arco-íris! Ninguém te pega!", "Rainbow turbo! Nobody can catch you!"));
            break;
        case PIRULITO_ROSA:
            tempoIma = 7.f;
            efeitos.explosao(p, 24, sf::Color(255, 170, 200), 190, true);
            mostrarAviso(T("Ímã de doces!", "Candy magnet!"));
            break;
        default:
            tempoHipnose = 4.5f;
            efeitos.anel(p, 26, sf::Color(255, 90, 120), 60);
            mostrarAviso(T("Hipnose! As rivais ficaram tontas!", "Hypnosis! The rivals are dizzy!"));
            break;
    }
}

void Jogo::ganharPontos(int n) {
    pontos += n;
    popPontos = 1;
    if (!ganhouVidaExtra && pontos >= 10000) {
        ganhouVidaExtra = true;
        vidas++;
        sons.tocar(Som::VIDA_EXTRA);
        mostrarAviso(T("Vida extra!", "Extra life!"));
    }
    if (pontos > recorde) {
        recorde = pontos;
        novoRecorde = true;
    }
}

void Jogo::ativarPoder() {
    tempoPoder = dif().tempoPoder;
    comboRivais = 0;
    for (auto& r : rivais) {
        if (r.estado == EstadoRival::VOLTANDO) continue;
        r.assustada = true;
        if (r.estado == EstadoRival::ATIVA) r.inverter();
    }
}

void Jogo::mostrarAviso(const std::string& s) {
    aviso = s;
    tempoAviso = 2.0f;
}

// rivais

void Jogo::atualizarRivais(float dt) {
    // enquanto o boba tea vale, o relogio dos modos fica parado
    if (tempoPoder > 0) {
        tempoPoder -= dt;
        if (tempoPoder <= 0)
            for (auto& r : rivais) r.assustada = false;
    } else if (indiceModo < 7) {
        tempoModo += dt;
        if (tempoModo >= dif().modos[indiceModo]) {
            tempoModo = 0;
            indiceModo++;
            modoPerseguir = (indiceModo % 2 == 1);
            for (auto& r : rivais)                      // trocou de modo: todas viram
                if (r.estado == EstadoRival::ATIVA) r.inverter();
        }
    }
    for (auto& r : rivais) atualizarRival(r, dt);
}

void Jogo::atualizarRival(Rival& r, float dt) {
    // hipnotizada:
    if (tempoHipnose > 0 && r.estado == EstadoRival::ATIVA && !r.assustada) return;

    if (r.estado == EstadoRival::EM_CASA) {
        r.tempoParaSair -= dt;
        if (r.tempoParaSair <= 0) {
            r.estado = EstadoRival::SAINDO;
            decidirCaminho(r);
        }
        return;
    }

    float vel = dif().velRival;
    if (r.estado == EstadoRival::VOLTANDO) vel = 12.f;
    else if (r.assustada) vel = dif().velAssustada;
    else if (r.estado == EstadoRival::SAINDO) vel = 4.f;
    else if (r.jeito == Personalidade::PERSEGUE && docesRestantes < 30) vel *= 1.1f;   // Cereja fica brava

    if (!r.andando) decidirCaminho(r);
    if (r.avancar(vel, dt, mapa)) decidirCaminho(r);
}

void Jogo::decidirCaminho(Rival& r) {
    sf::Vector2i porta = mapa.posPortaCasa;
    sf::Vector2i acimaDaPorta = passo(porta, CIMA);

    switch (r.estado) {
        case EstadoRival::SAINDO:
            // anda ate ficar embaixo da porta, sobe e sai
            if (r.casa == acimaDaPorta) {
                r.estado = EstadoRival::ATIVA;
                r.andarPara(escolherDirecao(r, alvoDa(r), mapa, r.assustada));
            } else if (r.casa.x < porta.x) r.andarPara(DIREITA);
            else if (r.casa.x > porta.x) r.andarPara(ESQUERDA);
            else r.andarPara(CIMA);
            break;

        case EstadoRival::VOLTANDO:
            // foi comida: volta correndo pra casa
            if (mapa.em(r.casa) == 'C') {
                r.estado = EstadoRival::SAINDO;
                r.assustada = false;
                decidirCaminho(r);
            } else if (r.casa == acimaDaPorta || r.casa == porta) {
                r.andarPara(BAIXO);
            } else {
                r.andarPara(escolherDirecao(r, acimaDaPorta, mapa, false));
            }
            break;

        case EstadoRival::ATIVA:
            r.andarPara(escolherDirecao(r, alvoDa(r), mapa, r.assustada));
            break;

        default:
            break;
    }
}

// cada rival mira num lugar diferente
sf::Vector2i Jogo::alvoDa(const Rival& r) const {
    bool brava = r.jeito == Personalidade::PERSEGUE && docesRestantes < 30;
    if (!modoPerseguir && !brava) return r.canto;

    sf::Vector2i pj = jogadora.casa;
    Direcao dj = jogadora.dir;

    switch (r.jeito) {
        case Personalidade::PERSEGUE:            // vai direto na jogadora
            return pj;
        case Personalidade::EMBOSCA:             // mira 4 casas na frente dela
            return {pj.x + 4 * dxDe(dj), pj.y + 4 * dyDe(dj)};
        case Personalidade::CERCA: {             // usa a Cereja pra fechar o outro lado
            sf::Vector2i frente = {pj.x + 2 * dxDe(dj), pj.y + 2 * dyDe(dj)};
            sf::Vector2i vetor = frente - rivais[0].casa;
            return frente + vetor;
        }
        case Personalidade::TIMIDA: {            // persegue de longe, foge de perto
            float d = std::hypot(float(r.casa.x - pj.x), float(r.casa.y - pj.y));
            return d > 8 ? pj : r.canto;
        }
    }
    return pj;
}

void Jogo::verificarEncontros() {
    sf::Vector2f pj = jogadora.posicao(mapa);
    for (auto& r : rivais) {
        if (r.estado == EstadoRival::EM_CASA || r.estado == EstadoRival::VOLTANDO) continue;
        if (distancia(pj, r.posicao(mapa)) > TILE * 0.6f) continue;

        if (r.assustada || tempoTurbo > 0) comerRival(r);     // no turbo ela atropela
        else if (tempoHipnose > 0) continue;                  // tonta nao pega ninguem
        else {
            perderVida();
            return;
        }
    }
}

void Jogo::comerRival(Rival& r) {
    comboRivais++;
    int valor = 200 << std::min(comboRivais - 1, 3);   // 200, 400, 800, 1600
    ganharPontos(valor);
    sf::Vector2f p = r.posicao(mapa);
    efeitos.textoSubindo(p + sf::Vector2f(0, -20), "+" + std::to_string(valor), r.cor);
    efeitos.explosao(p, 16, r.cor, 160, true);
    efeitos.tremer(4);
    sons.tocar(Som::COMER_RIVAL);
    r.estado = EstadoRival::VOLTANDO;
    r.assustada = false;
    congelado = 0.35f;
}

void Jogo::perderVida() {
    vidas--;
    sons.tocar(Som::MORTE);
    efeitos.tremer(10);
    irPara(Tela::MORRENDO);
}

// idioma
void Jogo::aplicarIdioma() {
    rivais[0].nome = T("Cereja", "Cherry");
    rivais[0].descricao = T("Persegue você", "Chases you");
    rivais[1].nome = T("Algodão", "Cotton");
    rivais[1].descricao = T("Corta pela frente", "Cuts you off");
    rivais[2].nome = "Chantilly";
    rivais[2].descricao = T("Cerca com a Cereja", "Flanks with Cherry");
    rivais[3].nome = T("Amora", "Berry");
    rivais[3].descricao = T("Tímida: foge de perto", "Shy: flees up close");
}

// o idioma escolhido fica guardado em idioma.txt
void Jogo::carregarIdioma() {
    std::ifstream f("idioma.txt");
    std::string codigo;
    f >> codigo;
    emIngles = (codigo == "en");
    aplicarIdioma();
}

void Jogo::trocarIdioma() {
    emIngles = !emIngles;
    aplicarIdioma();
    sons.tocar(Som::MENU);
    std::ofstream("idioma.txt") << (emIngles ? "en" : "pt") << "\n";
}

void Jogo::carregarRecorde() {
    std::ifstream f("recorde.txt");
    if (!(f >> recorde)) recorde = 0;
}

void Jogo::salvarRecorde() {
    int salvo = 0;
    std::ifstream entrada("recorde.txt");
    entrada >> salvo;
    entrada.close();
    if (recorde > salvo) std::ofstream("recorde.txt") << recorde << "\n";
}

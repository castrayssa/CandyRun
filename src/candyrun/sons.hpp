// sons do jogo, gerados por IA
#pragma once

#include <SFML/Audio.hpp>
#include <optional>
#include <vector>

enum class Som { BOBA, COMER_RIVAL, MORTE, CHAVE, PORTA, FASE, VIDA_EXTRA, MENU, PIRULITO, CORRIDA, TOTAL };

const int NOTAS_DOCE = 8;  

struct Sons {
    sf::SoundBuffer buffers[int(Som::TOTAL)];
    std::vector<sf::Sound> vozes;  
    sf::SoundBuffer buffersDoce[NOTAS_DOCE];
    std::vector<sf::Sound> vozesDoce;

    sf::SoundBuffer bufferMusica;
    std::optional<sf::Sound> musicaGerada;
    sf::Music musicaArquivo;
    bool usandoArquivo = false;
    bool mudo = false;
    bool musicaLigada = false;

    void carregar();
    void tocar(Som s);
    void tocarDoce(int nota); 
    void musica(bool ligar);
    void alternarMudo();
};

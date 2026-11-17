// sons gerados por IA

#include "sons.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
namespace {

const unsigned TAXA = 44100;    
enum class Onda { QUADRADA, TRIANGULO, SENO };

struct Nota {
    float freqInicio, freqFim, duracao;
};

float freqMidi(int nota) { return 440.f * std::pow(2.f, (nota - 69) / 12.f); }

float amostraOnda(Onda tipo, float fase) {        
    if (tipo == Onda::QUADRADA) return fase < 0.25f ? 1.f : -1.f;
    if (tipo == Onda::SENO) return std::sin(fase * 6.2831853f);
    return 4.f * std::abs(fase - 0.5f) - 1.f;    
}

std::vector<std::int16_t> sintetizar(const std::vector<Nota>& notas, Onda tipo, float volume) {
    std::vector<std::int16_t> amostras;
    float fase = 0;
    for (const Nota& n : notas) {
        int qtd = int(n.duracao * TAXA);
        for (int i = 0; i < qtd; i++) {
            float t = float(i) / qtd;
            float freq = n.freqInicio + (n.freqFim - n.freqInicio) * t;
            fase += freq / TAXA;
            fase -= std::floor(fase);
            float envelope = std::min(1.f, i / (0.004f * TAXA)) * (1.f - t);  
            float v = freq > 0 ? amostraOnda(tipo, fase) * envelope * volume : 0.f;
            amostras.push_back(std::int16_t(v * 32767));
        }
    }
    return amostras;
}

std::vector<Nota> arpejo(std::initializer_list<int> notasMidi, float duracao) {
    std::vector<Nota> r;
    for (int n : notasMidi) r.push_back({freqMidi(n), freqMidi(n), duracao});
    return r;
}

bool montar(sf::SoundBuffer& b, const std::vector<std::int16_t>& a) {
    return b.loadFromSamples(a.data(), a.size(), 1, TAXA, {sf::SoundChannel::Mono});
}

void somarNota(std::vector<float>& mix, float inicio, float duracao, float freq, Onda tipo, float volume) {
    int a = int(inicio * TAXA), qtd = int(duracao * TAXA);
    float fase = 0;
    for (int i = 0; i < qtd && a + i < int(mix.size()); i++) {
        float t = float(i) / qtd;
        fase += freq / TAXA;
        fase -= std::floor(fase);
        float envelope = std::min(1.f, i / (0.006f * TAXA)) * std::pow(1.f - t, 1.5f);
        mix[a + i] += amostraOnda(tipo, fase) * envelope * volume;
    }
}


std::vector<std::int16_t> gerarMusica() {
    const float colcheia = 0.21f;
    const int melodia[32] = {76, 79, 81, 79, 76, 74, 72, 74,
                             76, 76, 79, 76, 74, 72, 69, 0,
                             72, 74, 76, 79, 81, 79, 76, 79,
                             81, 79, 76, 74, 72, 74, 72, 0};
    const int baixo[16] = {48, 48, 43, 43, 45, 45, 41, 41, 48, 48, 43, 43, 41, 43, 48, 48};

    std::vector<float> mix(std::size_t(32 * colcheia * TAXA), 0.f);
    for (int k = 0; k < 32; k++)
        if (melodia[k]) somarNota(mix, k * colcheia, colcheia * 0.95f, freqMidi(melodia[k]), Onda::TRIANGULO, 0.30f);
    for (int k = 0; k < 16; k++) {
        somarNota(mix, k * 2 * colcheia, colcheia * 1.8f, freqMidi(baixo[k]), Onda::QUADRADA, 0.07f);
        somarNota(mix, k * 2 * colcheia, colcheia * 1.8f, freqMidi(baixo[k] + 12), Onda::TRIANGULO, 0.08f);
    }

    std::vector<std::int16_t> r(mix.size());
    for (std::size_t i = 0; i < mix.size(); i++)
        r[i] = std::int16_t(std::clamp(mix[i], -1.f, 1.f) * 32767);
    return r;
}

} 

void Sons::carregar() {
    auto& b = buffers;
    bool ok = true;
    const int escala[NOTAS_DOCE] = {72, 74, 76, 79, 81, 79, 76, 74};
    for (int i = 0; i < NOTAS_DOCE; i++) {
        float f = freqMidi(escala[i]);
        ok &= montar(buffersDoce[i], sintetizar({{f, f, 0.11f}}, Onda::SENO, 0.16f));
    }
    ok &= montar(b[int(Som::PIRULITO)], sintetizar(arpejo({79, 84, 88, 91, 96}, 0.07f), Onda::SENO, 0.4f));
    ok &= montar(b[int(Som::CORRIDA)], sintetizar({{300, 900, 0.16f}}, Onda::SENO, 0.3f));
    ok &= montar(b[int(Som::BOBA)], sintetizar(arpejo({72, 76, 79, 84, 88, 91}, 0.06f), Onda::TRIANGULO, 0.45f));
    ok &= montar(b[int(Som::COMER_RIVAL)], sintetizar({{300, 1500, 0.22f}, {1500, 1900, 0.10f}}, Onda::QUADRADA, 0.22f));
    ok &= montar(b[int(Som::MORTE)], sintetizar({{900, 600, 0.18f}, {700, 400, 0.18f}, {500, 250, 0.2f}, {300, 70, 0.5f}},
                                                Onda::TRIANGULO, 0.45f));
    ok &= montar(b[int(Som::CHAVE)], sintetizar(arpejo({84, 88, 91, 96, 100}, 0.055f), Onda::TRIANGULO, 0.4f));
    ok &= montar(b[int(Som::PORTA)], sintetizar({{220, 120, 0.12f}, {180, 90, 0.18f}}, Onda::QUADRADA, 0.2f));
    ok &= montar(b[int(Som::FASE)], sintetizar(arpejo({72, 76, 79, 84, 79, 84, 88}, 0.11f), Onda::TRIANGULO, 0.45f));
    ok &= montar(b[int(Som::VIDA_EXTRA)], sintetizar(arpejo({84, 88, 91, 96, 91, 96}, 0.07f), Onda::QUADRADA, 0.18f));
    ok &= montar(b[int(Som::MENU)], sintetizar({{660, 990, 0.07f}}, Onda::QUADRADA, 0.18f));
    (void)ok;   

    vozes.clear();
    vozes.reserve(int(Som::TOTAL));
    for (auto& buffer : buffers) vozes.emplace_back(buffer);
    vozesDoce.clear();
    vozesDoce.reserve(NOTAS_DOCE);
    for (auto& buffer : buffersDoce) {
        vozesDoce.emplace_back(buffer);
        vozesDoce.back().setVolume(45.f);
    }

    if (std::filesystem::exists("assets/sons/musica.ogg") && musicaArquivo.openFromFile("assets/sons/musica.ogg")) {
        usandoArquivo = true;
        musicaArquivo.setLooping(true);
        musicaArquivo.setVolume(45.f);
    } else if (montar(bufferMusica, gerarMusica())) {
        musicaGerada.emplace(bufferMusica);
        musicaGerada->setLooping(true);
        musicaGerada->setVolume(40.f);
    }
}
void Sons::tocar(Som s) {
    if (mudo || vozes.empty()) return;
    vozes[int(s)].play();
}

void Sons::tocarDoce(int nota) {
    if (mudo || vozesDoce.empty()) return;
    vozesDoce[nota % NOTAS_DOCE].play();
}

void Sons::musica(bool ligar) {
    musicaLigada = ligar;
    bool tocando = ligar && !mudo;
    sf::SoundSource* fonte = nullptr;
    if (usandoArquivo) fonte = &musicaArquivo;
    else if (musicaGerada) fonte = &*musicaGerada;
    if (!fonte) return;

    bool jaTocando = fonte->getStatus() == sf::SoundSource::Status::Playing;
    if (tocando && !jaTocando) fonte->play(); 
    if (!tocando && jaTocando) fonte->pause();
}

void Sons::alternarMudo() {
    mudo = !mudo;
    musica(musicaLigada);
}

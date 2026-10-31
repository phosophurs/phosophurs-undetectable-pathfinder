#include "ClickAudio.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <random>
#include <vector>

#include <Geode/Geode.hpp>

#pragma comment(lib, "winmm.lib")

namespace phosophurs::audio {
namespace {
    constexpr uint32_t SAMPLE_RATE = 22050;
    constexpr double PI = 3.14159265358979323846;

    enum class Shape { Sine, Triangle, SoftSquare, Saw };

    struct SoundBank {
        std::array<std::vector<char>, 4> wavs;
    };

    void append16(std::vector<char>& out, uint16_t value) {
        out.push_back(static_cast<char>(value & 0xff));
        out.push_back(static_cast<char>((value >> 8) & 0xff));
    }

    void append32(std::vector<char>& out, uint32_t value) {
        out.push_back(static_cast<char>(value & 0xff));
        out.push_back(static_cast<char>((value >> 8) & 0xff));
        out.push_back(static_cast<char>((value >> 16) & 0xff));
        out.push_back(static_cast<char>((value >> 24) & 0xff));
    }

    double wave(Shape shape, double phase) {
        const double cycle = phase - std::floor(phase);
        switch (shape) {
            case Shape::Sine: return std::sin(2.0 * PI * cycle);
            case Shape::Triangle: return 1.0 - 4.0 * std::abs(cycle - 0.5);
            case Shape::SoftSquare: return std::tanh(3.0 * std::sin(2.0 * PI * cycle));
            case Shape::Saw: return 2.0 * cycle - 1.0;
        }
        return 0.0;
    }

    std::vector<char> makeWav(bool mouse, Shape shape, unsigned variant) {
        const double duration = mouse ? 0.021 : 0.034;
        const size_t count = static_cast<size_t>(SAMPLE_RATE * duration);
        std::vector<int16_t> pcm(count);

        std::mt19937 rng(0x50484f53u + variant * 977u + (mouse ? 83u : 0u));
        std::uniform_real_distribution<double> noise(-1.0, 1.0);

        const double resonance = mouse ? 2350.0 + variant * 55.0 : 1680.0 + variant * 42.0;
        const double body = mouse ? 880.0 : 520.0;
        const double amplitude = mouse ? 0.46 : 0.42;

        for (size_t i = 0; i < count; ++i) {
            const double t = static_cast<double>(i) / SAMPLE_RATE;
            const double n = static_cast<double>(i) / count;
            const double fastDecay = std::exp(-n * (mouse ? 19.0 : 13.0));
            const double bodyDecay = std::exp(-n * (mouse ? 27.0 : 17.0));
            const double transient = noise(rng) * fastDecay * (mouse ? 0.30 : 0.23);
            const double toneA = wave(shape, t * resonance) * bodyDecay * 0.13;
            const double toneB = std::sin(2.0 * PI * body * t) * std::exp(-n * 10.0) * 0.11;
            const double sample = std::clamp(transient + toneA + toneB, -1.0, 1.0);
            pcm[i] = static_cast<int16_t>(sample * amplitude * 32767.0);
        }

        std::vector<char> wav;
        wav.reserve(44 + pcm.size() * sizeof(int16_t));
        wav.insert(wav.end(), {'R','I','F','F'});
        append32(wav, static_cast<uint32_t>(36 + pcm.size() * 2));
        wav.insert(wav.end(), {'W','A','V','E','f','m','t',' '});
        append32(wav, 16);
        append16(wav, 1);
        append16(wav, 1);
        append32(wav, SAMPLE_RATE);
        append32(wav, SAMPLE_RATE * 2);
        append16(wav, 2);
        append16(wav, 16);
        wav.insert(wav.end(), {'d','a','t','a'});
        append32(wav, static_cast<uint32_t>(pcm.size() * 2));
        for (auto sample : pcm) append16(wav, static_cast<uint16_t>(sample));
        return wav;
    }

    SoundBank makeBank(bool mouse) {
        SoundBank bank;
        const std::array<Shape, 4> shapes = {
            Shape::Sine, Shape::Triangle, Shape::SoftSquare, Shape::Saw
        };
        for (unsigned i = 0; i < shapes.size(); ++i) {
            bank.wavs[i] = makeWav(mouse, shapes[i], i);
        }
        return bank;
    }

    SoundBank& keyboardBank() {
        static SoundBank bank = makeBank(false);
        return bank;
    }

    SoundBank& mouseBank() {
        static SoundBank bank = makeBank(true);
        return bank;
    }

    unsigned nextVariant() {
        static std::mt19937 rng(std::random_device{}());
        static std::uniform_int_distribution<unsigned> pick(0, 3);
        return pick(rng);
    }

    void play(SoundBank& bank, unsigned variant) {
        auto const& wav = bank.wavs[variant];
        if (!wav.empty()) {
            // Static banks keep the in-memory WAV alive during async playback.
            PlaySoundA(wav.data(), nullptr, SND_ASYNC | SND_MEMORY | SND_NODEFAULT);
        }
    }
}

void playKeyboardClick() {
    auto mod = geode::Mod::get();
    if (!mod->getSettingValue<bool>("click-sounds-enabled")) return;
    if (!mod->getSettingValue<bool>("keyboard-clicks-enabled")) return;
    const bool vary = mod->getSettingValue<bool>("waveform-variation");
    play(keyboardBank(), vary ? nextVariant() : 0);
}

void playMouseClick() {
    auto mod = geode::Mod::get();
    if (!mod->getSettingValue<bool>("click-sounds-enabled")) return;
    if (!mod->getSettingValue<bool>("mouse-clicks-enabled")) return;
    const bool vary = mod->getSettingValue<bool>("waveform-variation");
    play(mouseBank(), vary ? nextVariant() : 0);
}
}

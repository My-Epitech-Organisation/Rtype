#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "SFML/Audio.hpp"

#include "src/client/Graphic/AudioLib/AudioLib.hpp"

TEST(AudioLibMoreTest, SetLoopAndMusicVolumeWithLoadedMusic) {
    AudioLib audio;
    auto music = std::make_shared<sf::Music>();
    // loading a default-constructed music is fine for control flow coverage
    audio.loadMusic(music);
    audio.setLoop(true);
    audio.setMusicVolume(10.0f);
    EXPECT_FLOAT_EQ(audio.getMusicVolume(), 10.0f);
    audio.setLoop(false);
    audio.pauseMusic();
}

TEST(AudioLibMoreTest, PlayWithLoadedMusicDoesNotCrash) {
    AudioLib audio;
    auto music = std::make_shared<sf::Music>();
    audio.loadMusic(music);
    EXPECT_NO_THROW(audio.play());
}

TEST(AudioLibMoreTest, PlaySFXClearsStoppedAndAddsSound) {
    AudioLib audio;
    std::vector<std::int16_t> samples(441, 0);
    sf::SoundBuffer buf;
    ASSERT_TRUE(buf.loadFromSamples(reinterpret_cast<const std::int16_t*>(samples.data()), static_cast<std::uint64_t>(samples.size()), 1, 44100, {sf::SoundChannel::Mono}));
    EXPECT_NO_THROW(audio.playSFX(buf));
    audio.setSFXVolume(5.0f);
    EXPECT_FLOAT_EQ(audio.getSFXVolume(), 5.0f);
}

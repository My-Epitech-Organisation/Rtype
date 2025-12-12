#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "SFML/Audio.hpp"

#include "src/client/Graphic/AudioLib/AudioLib.hpp"

TEST(AudioLibBranches, SetLoopAndPauseWithoutMusicDoesNotCrash) {
    AudioLib audio;
    // No music loaded - branches should be covered that guard on _currentMusic
    EXPECT_NO_THROW(audio.setLoop(true));
    EXPECT_NO_THROW(audio.pauseMusic());
    // Setting music volume without music should still update stored volume
    audio.setMusicVolume(12.5f);
    EXPECT_FLOAT_EQ(audio.getMusicVolume(), 12.5f);
}

TEST(AudioLibBranches, LoadMusicStopsPreviousMusicAndSetVolume) {
    AudioLib audio;
    auto music1 = std::make_shared<sf::Music>();
    auto music2 = std::make_shared<sf::Music>();

    // Load music1 and set a volume
    audio.loadMusic(music1);
    audio.setMusicVolume(33.0f);
    EXPECT_FLOAT_EQ(audio.getMusicVolume(), 33.0f);

    // Load music2 - this should stop music1 and set music2's volume
    audio.loadMusic(music2);
    // Changing the audio volume updates the active music as well
    audio.setMusicVolume(11.0f);
    EXPECT_FLOAT_EQ(audio.getMusicVolume(), 11.0f);
}

TEST(AudioLibBranches, SetSFXVolumeAndPlaySFXWhenNoExistingSounds) {
    AudioLib audio;
    std::vector<std::int16_t> samples(441, 0);
    sf::SoundBuffer buf;
    ASSERT_TRUE(buf.loadFromSamples(reinterpret_cast<const std::int16_t*>(samples.data()), static_cast<std::uint64_t>(samples.size()), 1, 44100, {sf::SoundChannel::Mono}));

    // No sounds present yet - set volume should be safe
    audio.setSFXVolume(7.5f);
    EXPECT_FLOAT_EQ(audio.getSFXVolume(), 7.5f);

    // Play SFX - remove_if will iterate over an empty list or removed items
    EXPECT_NO_THROW(audio.playSFX(buf));
}

TEST(AudioLibBranches, PlayWithMusicDoesNotCrash) {
    AudioLib audio;
    auto music = std::make_shared<sf::Music>();
    audio.loadMusic(music);
    EXPECT_NO_THROW(audio.play());
}

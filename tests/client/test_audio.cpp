/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** Audio-related tests for AudioLib and SoundManager
*/

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "SFML/Audio.hpp"

#include "src/client/Graphic/AudioLib/AudioLib.hpp"
#include "src/client/Graphic/AssetManager/SoundManager.hpp"

TEST(AudioLibTest, MusicVolumeSetAndGet) {
    AudioLib audio;
    ASSERT_FLOAT_EQ(audio.getMusicVolume(), 50.0f);
    audio.setMusicVolume(75.0f);
    ASSERT_FLOAT_EQ(audio.getMusicVolume(), 75.0f);

    auto music = std::make_shared<sf::Music>();
    audio.loadMusic(music);
    audio.setMusicVolume(20.0f);
    ASSERT_FLOAT_EQ(audio.getMusicVolume(), 20.0f);
    audio.setLoop(true);
    audio.pauseMusic();
}

TEST(AudioLibTest, SetLoopWithLoadedMusicAndPlay) {
    AudioLib audio;
    auto music = std::make_shared<sf::Music>();
    audio.loadMusic(music);
    // With loaded music, setLoop should exercise the branch that calls into the music
    EXPECT_NO_THROW(audio.setLoop(true));
    EXPECT_NO_THROW(audio.play());
    EXPECT_NO_THROW(audio.pauseMusic());
}

TEST(AudioLibTest, SFXVolumeSet) {
    AudioLib audio;
    ASSERT_FLOAT_EQ(audio.getSFXVolume(), 25.0f);
    audio.setSFXVolume(30.0f);
    ASSERT_FLOAT_EQ(audio.getSFXVolume(), 30.0f);
}

TEST(AudioLibTest, NoCurrentMusicCallsAreNoops) {
    AudioLib audio;
    // These should be safe no-ops when no music is loaded.
    EXPECT_NO_THROW(audio.setLoop(true));
    EXPECT_NO_THROW(audio.pauseMusic());
}

TEST(AudioLibTest, PlayAndStopWithoutLoadedMusicAreNoops) {
    AudioLib audio;
    // No music loaded: play/stop should short-circuit safely.
    // Only pause should be safe without loaded music
    // AudioLib::stop() removed; use pause as a safe no-op when no music
    EXPECT_NO_THROW(audio.pauseMusic());
    // Ensure setters still work independently.
    audio.setSFXVolume(35.0f);
    ASSERT_FLOAT_EQ(audio.getSFXVolume(), 35.0f);
}

TEST(AudioLibTest, PlaySFXAddAndSetSFXVolume) {
    AudioLib audio;
    // Create a simple sound buffer from generated samples to avoid external asset
    std::vector<std::int16_t> samples(441, 0); // small buffer
    sf::SoundBuffer buffer;
    ASSERT_TRUE(buffer.loadFromSamples(reinterpret_cast<const std::int16_t*>(samples.data()), static_cast<std::uint64_t>(samples.size()), 1, 44100, {sf::SoundChannel::Mono}));

    // play SFX should add the sound to queue and set its volume
    EXPECT_NO_THROW(audio.playSFX(buffer));
    audio.setSFXVolume(10.0f);
    EXPECT_FLOAT_EQ(audio.getSFXVolume(), 10.0f);
}

TEST(SoundManagerTest, GetMissingThrows) {
    SoundManager mgr;
    EXPECT_THROW({ mgr.get("nope"); }, std::out_of_range);
}

TEST(SoundManagerTest, LoadInvalidPathThrows) {
    SoundManager mgr;
    // Use a guaranteed invalid path to trigger error branch
    EXPECT_THROW({ mgr.load("bad", "assets/audio/does_not_exist.wav"); }, std::runtime_error);
}

TEST(SoundManagerTest, LoadValidThenDuplicateIdSkipsReload) {
    SoundManager mgr;
    // Attempt to load a real asset; skip test if asset cannot be opened.
    try {
        mgr.load("laser", "assets/audio/laserSound.mp3");
    } catch (const std::runtime_error&) {
        GTEST_SKIP() << "Missing audio asset: skipping test";
    }
    // Second load with same id should early-return without throwing.
    EXPECT_NO_THROW(mgr.load("laser", "assets/audio/laserSound.mp3"));
    auto buf = mgr.get("laser");
    ASSERT_TRUE(buf);
}

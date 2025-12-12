#include <gtest/gtest.h>

#include "Graphic/AssetManager/FontManager.hpp"
#include "Graphic/AssetManager/TextureManager.hpp"
#include "Graphic/AssetManager/SoundManager.hpp"

using namespace std;

TEST(AssetManagers, FontManagerLoadAndGet) {
    FontManager fm;
    // Load a known font from assets
    string path = "../../assets/fonts/Orbitron-VariableFont_wght.ttf";
    fm.load("title_font", path);
    EXPECT_TRUE(fm.isLoaded("title_font"));
    EXPECT_EQ(fm.size(), 1u);
    auto& f = fm.get("title_font");
    (void)f;
    EXPECT_TRUE(fm.unload("title_font"));
    EXPECT_FALSE(fm.isLoaded("title_font"));
}

TEST(AssetManagers, FontManagerLoadInvalidThrows) {
    FontManager fm;
    EXPECT_THROW(fm.load("bad_font", "assets/fonts/does_not_exist.ttf"), std::runtime_error);
}

TEST(AssetManagers, TextureManagerLoadAndGet) {
    TextureManager tm;
    string path = "../../assets/img/astroVessel.png";
    tm.load("astro", path);
    EXPECT_TRUE(tm.isLoaded("astro"));
    EXPECT_EQ(tm.size(), 1u);
    auto& t = tm.get("astro");
    (void)t;
    EXPECT_TRUE(tm.unload("astro"));
    EXPECT_FALSE(tm.isLoaded("astro"));
}

TEST(AssetManagers, TextureManagerLoadInvalidThrows) {
    TextureManager tm;
    EXPECT_THROW(tm.load("bad_tex", "assets/img/does_not_exist.png"), std::runtime_error);
}

TEST(AssetManagers, SoundManagerLoadAndGet) {
    SoundManager sm;
    string path = "../../assets/audio/clickButton.mp3";
    sm.load("click", path);
    auto sharedPtr = sm.get("click");
    EXPECT_NE(sharedPtr, nullptr);
}

TEST(AssetManagers, SoundManagerLoadInvalidThrows) {
    SoundManager sm;
    EXPECT_THROW(sm.load("bad_sound", "assets/audio/does_not_exist.mp3"), std::runtime_error);
}

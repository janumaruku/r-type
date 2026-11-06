/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Hud.hpp
*/

#ifndef HUD_HPP
#define HUD_HPP

#include <string>
#include "ClientWorld.hpp"
#include "GameEventBus.hpp"

namespace rtype::client {

constexpr float MARGIN = 16.0F;
constexpr float SCREEN_WIDTH = 1280.0F;    // the 720p playfield
constexpr float SCREEN_HEIGHT = 720.0F;
constexpr float RIGHT_COLUMN_X = 960.0F;   // TextAlign has no RIGHT: a fixed column
constexpr float LINE_SPACING = 1.4F;       // line height, as a multiple of the font size
constexpr float LABEL_OFFSET = 36.0F;      // label centre, above the ship centre
constexpr float HELP_X = MARGIN;
constexpr float HELP_Y = 160.0F;           // below the four slot lines
constexpr unsigned MAX_CHARACTER_SIZE = 255;



struct Banner {
    std::string text;
    float remaining;
};

class HudReactor {
public:
    static constexpr std::size_t MAX_BANNERS = 3;
    static constexpr float BANNER_SECONDS    = 3.0F;
    static constexpr float GAME_OVER_SECONDS = 5.0F;

    HudReactor(GameEventBus &bus, const ClientWorld &world);
    void onEvent(const protocol::Event &event);
    void update(float dt);
    [[nodiscard]] std::deque<Banner> &banners() noexcept;
    [[nodiscard]] std::optional<float> gameOverCountdown() const noexcept;

private:
    void addBanner(std::string text);

    const ClientWorld &_world;
    std::deque<Banner> _banners;
    std::optional<float> _gameOverIn;
    GameEventBus::Subscription _subscription;
};

struct HudState {
    bool showHelp = false;
    std::optional<std::chrono::milliseconds> ping;
};

[[nodiscard]] std::string playerLabel(std::uint8_t playerId);
[[nodiscard]] std::string slotLine(const protocol::PlayerInfo &slot);
[[nodiscard]] std::uint32_t teamScore(const std::array<protocol::PlayerInfo, 4>
        &roster) noexcept;

} // namespace rtype::client

#endif /* !HUD_HPP_ */

/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Volume.hpp
*/

#ifndef VOLUME_HPP
#define VOLUME_HPP

namespace engine::audio {

/*
** The volume rules shared by every IAudio implementation. Each level is
** clamped to [0, 1], and a NaN becomes 0. Muting doesn't touch the stored
** levels, so unmuting gives back exactly the previous ones.
*/
class Volume {
public:
    void setMaster(float volume) noexcept;

    void setSfx(float volume) noexcept;

    void setMusic(float volume) noexcept;

    void setMuted(bool muted) noexcept;

    [[nodiscard]] float master() const noexcept;

    [[nodiscard]] float sfx() const noexcept;

    [[nodiscard]] float music() const noexcept;

    [[nodiscard]] bool muted() const noexcept;

    /* ** master x sfx x playVolume, or 0 while muted */
    [[nodiscard]] float effectiveSfx(float playVolume) const noexcept;

    /* ** master x music, or 0 while muted */
    [[nodiscard]] float effectiveMusic() const noexcept;

private:
    float _master = 1.0F;
    float _sfx = 1.0F;
    float _music = 1.0F;
    bool _muted = false;
};

}

#endif // !VOLUME_HPP

/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** PaintCommands
*/

#ifndef R_TYPE_PAINT_COMMANDS_HPP
#define R_TYPE_PAINT_COMMANDS_HPP

#include <variant>
#include <vector>

#include "kollection/core/Dimensions.hpp"
#include "kollection/core/Styles.hpp"

namespace kollection::paint {
struct FillRect {
    core::Rect rect;
    core::Color color;
};

struct StrokeRect {
    core::Rect rect;
    core::Color color;
    double width = 0.0F;
};

struct DrawText {
    core::Offset at;
    core::TextRun run;
};

using PaintCommand = std::variant<FillRect, StrokeRect, DrawText>;
using PaintList = std::vector<PaintCommand>;

class PaintContext {
public:
    explicit PaintContext(PaintList &list);

    void fillRect(const core::Rect &r, const core::Color &c) const;

    void strokeRect(const core::Rect &r, const core::Color &c, double w) const;

    void drawText(const core::Offset &at, const core::TextRun &run) const;

private:
    PaintList &_list;
};
}

#endif //R_TYPE_PAINT_COMMANDS_HPP

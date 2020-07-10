#include <libgraphic/TrueTypeFont.h>

#include "demo/Demos.h"

static TrueTypeFamily *_family = NULL;
static TrueTypeFont *_fonts[16];

void fonts_draw(Painter *painter, Rectangle screen, double time)
{
    __unused(screen);
    __unused(time);

    if (!_family)
    {
        _family = truetype_family_create("/System/Fonts/Roboto/Roboto-Medium.ttf");

        for (size_t i = 0; i < 16; i++)
        {
            _fonts[i] = truetypefont_create(_family, 8 + 4 * i);
        }
    }

    painter_clear(painter, COLOR_REBECCAPURPLE);

    int current = 4;
    for (size_t i = 0; i < 16; i++)
    {
        TrueTypeFontMetrics metrics = truetypefont_get_metrics(_fonts[i]);

        current += metrics.ascent;

        painter_draw_truetype_string(
            painter,
            _fonts[i],
            u8"The quick brown fox jumps over the lazy dog",
            vec2i(8, current),
            COLOR_WHITE);

        current -= metrics.descent;
        current += metrics.linegap;
    }
}

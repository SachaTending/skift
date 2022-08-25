#include <karm-main/main.h>
#include <karm-media/loader.h>
#include <karm-ui/app.h>

struct FontApp : public Ui::Widget<FontApp> {
    Strong<Media::Font> _font;

    FontApp(Strong<Media::Font> font)
        : _font{font} {}

    void paint(Gfx::Context &g) override {
        g.clear(Gfx::BLACK);

        g.textStyle(Gfx::text(_font));
        g.fillStr({16, 100}, "Hello, world!");
    }
};

CliResult entryPoint(CliArgs args) {
    auto font = Media::Font::fallback();
    if (args.len() == 1) {
        font = try$(Media::loadFont(args[0]));
    }
    return Ui::runApp<FontApp>(args, font);
}

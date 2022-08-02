#include <om/engine.hxx>
#include <om/game.hxx>

#include <array>
#include <iostream>

class tic_tac_toe final : public om::game
{
public:
    explicit tic_tac_toe(om::engine&);

    void               initialize() override;
    void               process_input(om::event& e) override;
    void               update(om::milliseconds frame_delta) override;
    void               draw() const override;
    [[nodiscard]] bool is_closed() const override;

private:
    om::engine&         e;
    std::array<char, 5> anim;
    size_t              index    = 0;
    const double        fps_base = 1.0 / 12;
    double              fps      = 1.0 / 12;
};

std::unique_ptr<om::game> OM_GAME create_game(om::engine& e)
{
    return std::make_unique<tic_tac_toe>(e);
}

tic_tac_toe::tic_tac_toe(om::engine& e_)
    : e(e_)
    , anim({ { '-', '\\', '|', '/', '-' } })
{
    om::engine::params params;

    params.title           = "tic_tac_toe game example";
    params.wnd_mode.width  = 1024;
    params.wnd_mode.height = 768;

    e.initialize(params);
}

void tic_tac_toe::initialize() {}

void tic_tac_toe::process_input(om::event&) {}

void tic_tac_toe::update(om::milliseconds frame_delta)
{
    const double dt =
        static_cast<double>(frame_delta.count()) * 0.001; // seconds
    fps -= dt;
    if (fps <= 0)
    {
        constexpr int count_lines = 4;
        int back_chars  = count_lines;
        while (back_chars--)
        {
            std::cout << '\b';
        }

        int lines = count_lines;
        while (lines--)
        {
            index = (index + 1) % anim.size();
            std::cout << anim[index] << std::flush;
        }

        fps += fps_base;
    }
}

void tic_tac_toe::draw() const {}

bool tic_tac_toe::is_closed() const
{
    return false;
}

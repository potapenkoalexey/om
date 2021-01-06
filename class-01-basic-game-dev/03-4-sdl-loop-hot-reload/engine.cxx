#include "engine.hxx"

#include <algorithm>
#include <array>
#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <SDL.h>

namespace om
{

static std::array<std::string_view, 17> event_names = {
    { /// input events
      "left_pressed",
      "left_released",
      "right_pressed",
      "right_released",
      "up_pressed",
      "up_released",
      "down_pressed",
      "down_released",
      "select_pressed",
      "select_released",
      "start_pressed",
      "start_released",
      "button1_pressed",
      "button1_released",
      "button2_pressed",
      "button2_released",
      /// virtual console events
      "turn_off" }
};

std::ostream& operator<<(std::ostream& stream, const event e)
{
    std::uint32_t value   = static_cast<std::uint32_t>(e);
    std::uint32_t maximal = static_cast<std::uint32_t>(event::turn_off);
    if (value <= maximal)
    {
        stream << event_names[value];
        return stream;
    }
    else
    {
        throw std::runtime_error("too big event value");
    }
}

static std::ostream& operator<<(std::ostream& out, const SDL_version& v)
{
    out << static_cast<int>(v.major) << '.';
    out << static_cast<int>(v.minor) << '.';
    out << static_cast<int>(v.patch);
    return out;
}

struct bind
{
    bind(SDL_Keycode k, std::string_view s, event pressed, event released)
        : key(k)
        , name(s)
        , event_pressed(pressed)
        , event_released(released)
    {
    }

    SDL_Keycode      key;
    std::string_view name;
    event            event_pressed;
    event            event_released;
};

const std::array<bind, 8> keys{
    { { SDLK_w, "up", event::up_pressed, event::up_released },
      { SDLK_a, "left", event::left_pressed, event::left_released },
      { SDLK_s, "down", event::down_pressed, event::down_released },
      { SDLK_d, "right", event::right_pressed, event::right_released },
      { SDLK_LCTRL,
        "button1",
        event::button1_pressed,
        event::button1_released },
      { SDLK_SPACE,
        "button2",
        event::button2_pressed,
        event::button2_released },
      { SDLK_ESCAPE, "select", event::select_pressed, event::select_released },
      { SDLK_RETURN, "start", event::start_pressed, event::start_released } }
};

static bool check_input(const SDL_Event& e, const bind*& result)
{
    using namespace std;

    const auto it = find_if(begin(keys), end(keys), [&](const bind& b) {
        return b.key == e.key.keysym.sym;
    });

    if (it != end(keys))
    {
        result = &(*it);
        return true;
    }
    return false;
}

class engine_impl final : public engine
{
public:
    /// create main window
    /// on success return empty string
    std::string initialize(std::string_view /*config*/) final
    {
        using namespace std;

        stringstream serr;

        SDL_version compiled = { 0, 0, 0 };
        SDL_version linked   = { 0, 0, 0 };

        SDL_VERSION(&compiled)
        SDL_GetVersion(&linked);

        if (SDL_COMPILEDVERSION !=
            SDL_VERSIONNUM(linked.major, linked.minor, linked.patch))
        {
            serr << "warning: SDL2 compiled and linked version mismatch: "
                 << compiled << " " << linked << endl;
        }

        const int init_result = SDL_Init(SDL_INIT_EVERYTHING);
        if (init_result != 0)
        {
            const char* err_message = SDL_GetError();
            serr << "error: failed call SDL_Init: " << err_message << endl;
            return serr.str();
        }
        /*
                SDL_Window* const window = SDL_CreateWindow("title",
                                                            SDL_WINDOWPOS_CENTERED,
                                                            SDL_WINDOWPOS_CENTERED,
                                                            640,
                                                            480,
                                                            ::SDL_WINDOW_OPENGL);

                if (window == nullptr)
                {
                    const char* err_message = SDL_GetError();
                    serr << "error: failed call SDL_CreateWindow: " <<
           err_message
                         << endl;
                    SDL_Quit();
                    return serr.str();
                }

                // Open the first available controller.
                SDL_GameController* controller = NULL;
                for (int i = 0; i < SDL_NumJoysticks(); ++i)
                {
                    if (SDL_IsGameController(i))
                    {
                        controller = SDL_GameControllerOpen(i);
                        if (controller)
                        {
                            break;
                        }
                        else
                        {
                            fprintf(stderr,
                                    "Could not open gamecontroller %i: %s\n",
                                    i,
                                    SDL_GetError());
                        }
                    }
                }
        */
        return "";
    }
    /// pool event from input queue
    /// return true if more events in queue
    bool read_input(event& e) final
    {
        using namespace std;
        // collect all events from SDL
        SDL_Event sdl_event;
        if (SDL_PollEvent(&sdl_event))
        {
            const bind* binding = nullptr;

            if (sdl_event.type == SDL_QUIT)
            {
                e = event::turn_off;
                return true;
            }
            else if (sdl_event.type == SDL_KEYDOWN)
            {
                if (check_input(sdl_event, binding))
                {
                    e = binding->event_pressed;
                    return true;
                }
            }
            else if (sdl_event.type == SDL_KEYUP)
            {
                if (check_input(sdl_event, binding))
                {
                    e = binding->event_released;
                    return true;
                }
            }
            else if (sdl_event.type == SDL_CONTROLLERDEVICEADDED)
            {
                // TODO map controller to user
                std::cerr << "controller added" << std::endl;
                // continue with next event in queue
                return read_input(e);
            }
            else if (sdl_event.type == SDL_CONTROLLERDEVICEREMOVED)
            {
                std::cerr << "controller removed" << std::endl;
            }
            else if (sdl_event.type == SDL_CONTROLLERBUTTONDOWN ||
                     sdl_event.type == SDL_CONTROLLERBUTTONUP)
            {
                // TODO finish implementation
                if (sdl_event.cbutton.state == SDL_PRESSED)
                {
                    e = event::button1_pressed;
                }
                else
                {
                    e = event::button1_released;
                }
                return true;
            }
        }
        return false;
    }
    void uninitialize() final {}
};

static bool already_exist = false;

engine* create_engine()
{
    if (already_exist)
    {
        throw std::runtime_error("engine already exist");
    }
    engine* result = new engine_impl();
    already_exist  = true;
    return result;
}

void destroy_engine(engine* e)
{
    if (already_exist == false)
    {
        throw std::runtime_error("engine not created");
    }
    if (nullptr == e)
    {
        throw std::runtime_error("e is nullptr");
    }
    delete e;
}

engine::~engine() {}

} // end namespace om

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#endif
// clang-format off
#ifdef __cplusplus
extern "C"
#endif
int main(int /*argc*/, char* /*argv*/[])
// clang-format on
{
    std::unique_ptr<om::engine, void (*)(om::engine*)> engine(
        om::create_engine(), om::destroy_engine);

    std::string err = engine->initialize("");
    if (!err.empty())
    {
        std::cerr << err << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "start app\n" << std::endl;

    if (!std::cout)
    {
#ifdef _WIN32
        const BOOL result = AttachConsole(ATTACH_PARENT_PROCESS);
        if (!result)
        {
            if (!AllocConsole())
            {
                return EXIT_FAILURE;
            }
        }

        FILE* f = std::freopen("CON", "w", stdout);
        if (!f)
        {
            throw std::runtime_error("can't reopen stdout");
        }
        FILE* fe = std::freopen("CON", "w", stderr);
        if (!fe)
        {
            throw std::runtime_error("can't reopen stderr");
        }
        std::cout.clear();
        std::cout << "start app" << std::endl;
        std::cerr << "start app" << std::endl;

        if (!std::cout.good() || !std::cerr.good())
        {
            throw std::runtime_error("can't print with std::cout");
        }
#endif
    }

    using namespace std::string_literals;
    // mingw library name for windows
    const char* library_name =
        SDL_GetPlatform() == "Windows"s ? "libgame-03-4.dll" : "game-03-4";

    void* game_handle = SDL_LoadObject(library_name);

    if (game_handle == nullptr)
    {
        std::cerr << SDL_GetError();
        return EXIT_FAILURE;
    }

    void* create_game_func_ptr = SDL_LoadFunction(game_handle, "create_game");

    if (create_game_func_ptr == nullptr)
    {
        std::cerr << SDL_GetError();
        return EXIT_FAILURE;
    }
    // void* destroy_game_func_ptr = SDL_LoadFunction(game_handle,
    // "destroy_game");

    typedef decltype(&create_game) create_game_ptr;

    auto create_game_func =
        reinterpret_cast<create_game_ptr>(create_game_func_ptr);

    om::game* game = create_game_func(engine.get());

    if (game == nullptr)
    {
        std::cerr << "game == nullptr\n";
        return EXIT_FAILURE;
    }

    game->initialize();

    bool continue_loop = true;
    while (continue_loop)
    {
        om::event event;

        while (engine->read_input(event))
        {
            std::cout << event << std::endl;
            switch (event)
            {
                case om::event::turn_off:
                    continue_loop = false;
                    break;
                default:
                    game->on_event(event);
                    break;
            }
        }

        game->update();
        game->render();
    }

    engine->uninitialize();

    return EXIT_SUCCESS;
}

#pragma once

#include <any>
#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace om
{

struct rect
{
    rect(){};
    rect(std::int32_t in_x, std::int32_t in_y, std::int32_t in_w,
         std::int32_t in_h)
        : x{ in_x }
        , y{ in_y }
        , w{ in_w }
        , h{ in_h } {};
    std::int32_t x = 0;
    std::int32_t y = 0;
    std::int32_t w = 0;
    std::int32_t h = 0;
};

struct display_mode
{
    std::uint32_t format       = 0;
    std::int32_t  w            = 0;
    std::int32_t  h            = 0;
    std::int32_t  refresh_rate = 0;
};

struct display
{
    // TODO Implement me
    display_mode mode;
    char         name[256];
    float        ddpi;
    float        vdpi;
    float        hdpi;
    rect         bounds;
    rect         usable_bounds;

    std::string_view          get_name() const;
    rect                      get_bounds() const;
    float                     get_diagonal_DPI() const;
    float                     get_horizontal_DPI() const;
    float                     get_vertical_DPI() const;
    rect                      get_usable_bounds() const;
    std::vector<display_mode> get_display_modes() const;
    display_mode              get_desktop_display_mode() const;
    display_mode              get_current_display_mode() const;
    display_mode get_closest_display_mode(const display_mode&) const;
};

struct surface
{
};

enum class hittest_result
{
    normal,
    draggable,
    resize_topleft,
    resize_top,
    resize_topright,
    resize_right,
    resize_bottomright,
    resize_bottom,
    resize_bottomleft,
    resize_left
};

struct window;

struct point
{
    std::int32_t x;
    std::int32_t y;
};

hittest_result hit_test(window& win, const point& p, std::any);

class window
{
public:
    enum class mode
    {
        undefined,
        opengl,
        vulkan
    };

    static const std::int32_t centered;  // TODO
    static const std::int32_t undefined; // TODO

    struct position
    {
        std::int32_t x = 0;
        std::int32_t y = 0;
        position(){};
        position(std::int32_t in_x, std::int32_t in_y)
            : x(in_x)
            , y(in_y)
        {
        }
        bool operator==(const position& other) const;
    };

    struct size
    {
        std::size_t w = 0;
        std::size_t h = 0;
        size(){};
        size(std::size_t in_w, std::size_t in_h)
            : w(in_w)
            , h(in_h)
        {
        }
        bool operator==(const size& other) const;
    };

public:
    window()              = delete;
    window(const window&) = delete;
    window& operator=(const window&) = delete;
    window& operator=(window&&) = delete;

    window(window&&);

    bool                 set_display_mode(const display_mode&);
    display_mode         get_display_mode() const;
    std::uint32_t        get_pixel_format() const;
    void                 set_title(const char* title);
    std::string_view     get_title() const;
    void                 set_icon(const surface& icon);
    void*                set_data(const char* name, void* userdata);
    void*                get_data(const char* name) const;
    void                 set_position(const position&);
    position             get_position() const;
    void                 set_size(const size&);
    size                 get_size() const;
    std::optional<rect>  get_border_size() const;
    void                 set_minimal_size(const size&);
    size                 get_minimal_size() const;
    void                 set_maximum_size(const size&);
    size                 get_maximum_size() const;
    void                 set_bordered(bool);
    bool                 is_bordered() const;
    void                 set_resizable(bool);
    bool                 is_resizeable() const;
    void                 show();
    bool                 is_shown() const;
    void                 hide();
    bool                 is_hidden() const;
    void                 raise();
    void                 maximize();
    bool                 is_maximized() const;
    void                 minimize();
    bool                 is_minimized() const;
    void                 restore();
    void                 set_fullscreen();
    bool                 is_fullscreen() const;
    void                 set_fullscreen_desktop();
    bool                 is_fullscreen_desktop() const;
    void                 set_windowed();
    bool                 is_windowed() const;
    surface              get_surface() const;
    bool                 update_surface(const surface&);
    bool                 update_surface_rects(const std::vector<rect>& rects);
    void                 grab_input(bool);
    bool                 has_input_grabbed() const;
    bool                 set_brightness(const float&);
    float                get_brightness() const;
    bool                 set_opacity(const float&);
    std::optional<float> get_opacity() const;
    bool                 set_modal_for(window& parent);
    void                 set_input_focus();
    bool                 has_input_focus() const;
    bool                 has_mouse_focus() const;
    bool                 has_mouse_captured() const;
    bool                 is_always_ontop() const;
    bool                 set_gamma_ramp(
                        const std::optional<std::array<std::uint16_t, 256>*> red,
                        const std::optional<std::array<std::uint16_t, 256>*> green,
                        const std::optional<std::array<std::uint16_t, 256>*> blue);
    bool get_gamma_ramp(std::optional<std::array<std::uint16_t, 256>*> red,
                        std::optional<std::array<std::uint16_t, 256>*> green,
                        std::optional<std::array<std::uint16_t, 256>*> blue);
    // bool set_hit_test(hit_test callback, std::any);
    void close();
    virtual ~window();

private:
    window(const char* title, size window_size, position window_position,
           mode window_mode);
    class impl;
    std::unique_ptr<impl> data;

    friend class video;
};

class gl_context
{
public:
    gl_context(const gl_context&) = delete;
    gl_context& operator=(const gl_context&) = delete;
    gl_context& operator=(gl_context&&) = delete;

    gl_context(gl_context&&);
    ~gl_context();

private:
    gl_context();
    class impl;
    impl* data = nullptr;

    friend class video;
};

enum class gl_swap_interval
{
    immediate,
    synchronized,
    adaptive
};

class video
{
public:
    static std::vector<std::string_view> get_drivers();
    void                                 init(const char* driver_name);
    void                                 quit();

    window     create_window(const char* title, const window::size window_size,
                             std::optional<window::position> window_position,
                             std::optional<window::mode>     window_mode);
    gl_context gl_create_context(const window&);

    std::string_view      get_current_driver() const;
    std::vector<display>  get_displays() const;
    display               get_display_for_window(const window&) const;
    window                create_window_from(const void* native_handle);
    window                get_window_fromID(std::uint32_t id) const;
    std::optional<window> get_grabbed_window() const;
    bool                  is_screen_saver_enabled() const;
    void                  enable_screen_saver();
    void                  disable_screen_saver();

    bool  gl_load_library(std::string_view path);
    void* gl_get_proc_address(std::string_view proc);
    void  gl_unload_library();
    bool  gl_extension_supported(std::string_view extension);
    void  gl_reset_attributes();
    // void         gl_set_attribute(gl_attribute attr, std::int32_t value);
    // std::int32_t gl_get_attribute(gl_attribute attr);
    bool       gl_make_current(const window& window, const gl_context& context);
    window     gl_get_current_window();
    gl_context gl_get_current_context();
    window::size gl_get_drawable_size(const window&);
    bool         gl_set_swap_interval(gl_swap_interval);
    void         gl_swap_window(const window&);
    void         gl_delete_context(const gl_context&);

    enum class gl_context_flag
    {
        debug,
        forward_compatible_mode,
        robust_access_mode,
        reset_isolation_mode,
    };

    enum class gl_context_profile
    {
        automatic,
        core,
        compatibility,
        es,
    };

    enum class gl_context_release_flag
    {
        none,
        flush
    };

private:
    class gl_attribute
    {
    public:
        void red_size(int);
        int  red_size();
        void green_size(int);
        int  green_size();
        void blue_size(int);
        int  blue_size();
        void alpha_size(int);
        int  alpha_size();
        void buffer_size(int);
        int  buffer_size();
        void doublebuffer(bool);
        bool doublebuffer();
        void depth_size(int);
        int  depth_size();
        void stencil_size(int);
        int  stencil_size();
        void accum_red_size(int);
        int  accum_red_size();
        void accum_green_size(int);
        int  accum_green_size();
        void accum_blue_size(int);
        int  accum_blue_size();
        void accum_alpha_size(int);
        int  accum_alpha_size();
        void stereo(bool);
        bool stereo();
        void multisamplebuffers(int);
        int  multisamplebuffers();
        void multisamplesamples(int);
        int  multisamplesamples();
        void accelerated_visual(bool);
        bool accelerated_visual();
        void context_major_version(int);
        int  context_major_version();
        void context_minor_version(int);
        int  context_minor_version();
        void context_flag(gl_context_flag flag, bool state); // on/off
        // FIXME:  Can we really switch flag off?
        bool               context_flag(gl_context_flag flag); // on/off
        void               context_profile_mask(gl_context_profile);
        gl_context_profile context_profile_mask();
        void               share_with_curent_context(bool);
        bool               share_with_curent_context();
        void               gl_framebuffer_srgb_capable(bool);
        bool               gl_framebuffer_srgb_capable();
        void               context_release_behavior(gl_context_release_flag);
        gl_context_release_flag context_release_behavior();
    };

public:
    gl_attribute gl_attr;
};

struct sdlxx
{
    explicit sdlxx();
    ~sdlxx();
    sdlxx(const sdlxx&) = delete;
    sdlxx& operator=(const sdlxx&) = delete;

}; // end sdlxx
} // end namespace om

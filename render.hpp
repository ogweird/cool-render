#pragma once

#include <stdint.h>
#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <numbers>
#include <cmath>

struct color {
    std::uint8_t r, g, b, a;

    constexpr color(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255) : r(r), g(g), b(b), a(a) {}

    constexpr D3DCOLOR to_d3d() const {
        return D3DCOLOR_ARGB(this->a, this->r, this->g, this->b);
    }
};

struct vertex {
    float x, y, z, rhw;
    D3DCOLOR color;
};

namespace colors {
    inline constexpr color white = color(255, 255, 255);
    inline constexpr color black = color(0, 0, 0);
    inline constexpr color red = color(255, 0, 0);
    inline constexpr color green = color(0, 255, 0);
    inline constexpr color blue = color(0, 0, 255);
}

inline std::unordered_map<std::string, ID3DXFont*> fonts = {};

enum class font_flags : std::uint8_t {
    none = 0,
    bold = 1 << 0,
    italic = 1 << 1
};

inline font_flags operator | (font_flags a, font_flags b) {
    return static_cast<font_flags>(static_cast<std::uint8_t>(a) | static_cast<std::uint8_t>(b));
}

inline font_flags& operator |= (font_flags& a, font_flags b) {
    a = a | b;
    return a;
}

namespace render {
    inline LPDIRECT3DDEVICE9 device = nullptr;

    bool device_checks() {
        if (!render::device) {
            return false;
        }

        HRESULT coop_level = device->TestCooperativeLevel();

        if (coop_level == D3DERR_DEVICELOST) {
            return false;
        }

        if (coop_level == D3DERR_DEVICENOTRESET) {
            return false;
        }

        return true;
    }

    bool check_font_flag(font_flags flags, font_flags test) {
        return (static_cast<uint8_t>(flags) & static_cast<uint8_t>(test)) != 0;
    }

    void create_font(const std::string& name, const char* font_name, std::uint8_t height, font_flags flags) {
        if (!render::device_checks()) {
            return;
        }

        auto it = fonts.find(name);

        if (it != fonts.end()) {
            return;
        }

        ID3DXFont* font = nullptr;

        auto weight = render::check_font_flag(flags, font_flags::bold) ? FW_BOLD : FW_NORMAL;
        auto italic = render::check_font_flag(flags, font_flags::italic);

        if (SUCCEEDED(D3DXCreateFontA(render::device, height, 0, weight, 1, italic, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, font_name, &fonts[name]))) {
            fonts.insert({ name, nullptr });
        }
        else {
            return;
        }
    }

    void clean_fonts() {
        for (auto& [_, font] : fonts) {
            if (font) {
                font->Release();
                font = nullptr;
            }
        }

        fonts.clear();
    }

    void text(const std::string& font, std::int32_t x, std::int32_t y, const char* text, color col) {
        if (!render::device_checks()) {
            return;
        }

        auto it = fonts.find(font);

        if (it == fonts.end() || !it->second) {
            return;
        }

        RECT space{ x, y };

        it->second->DrawTextA(nullptr, text, -1, &space, DT_NOCLIP, col.to_d3d());
    }

    void filled_rect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, color col) {
        if (!render::device_checks()) {
            return;
        }

        std::vector<vertex> vertices = {
            { (float)x, (float)y, 0.f, 1.f, col.to_d3d() },
            { (float)(x + w), (float)y, 0.f, 1.f, col.to_d3d() },
            { (float)x, (float)(y + h), 0.f, 1.f, col.to_d3d() },
            { (float)(x + w), (float)(y + h), 0.f, 1.f, col.to_d3d() }
        };

        render::device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
        render::device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices.data(), sizeof(vertex));
    }

    void outlined_rect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, color col, std::uint8_t thickness = 1) {
        if (!render::device_checks()) {
            return;
        }

        render::filled_rect(x, y, w, thickness, col);
        render::filled_rect(x, y, thickness, h, col);
        render::filled_rect(x + w - thickness, y, thickness, h, col);
        render::filled_rect(x, y + h - thickness, w, thickness, col);
    }

    void filled_circle(std::int32_t x, std::int32_t y, std::int32_t r, color col, std::uint8_t segments = 64) {
        if (!render::device_checks()) {
            return;
        }

        std::vector<vertex> vertices;
        vertices.reserve(segments + 1);

        vertices.push_back({ (float)x, (float)y, 0.f, 1.f, col.to_d3d() });

        for (std::size_t i = 0; i <= segments; i++) {
            float angle = (2.0f * std::numbers::pi_v<float> * i) / segments;
            float x1 = x + std::cos(angle) * r;
            float y1 = y + std::sin(angle) * r;

            vertices.push_back({ x1, y1, 0.f, 1.f, col.to_d3d() });
        }

        device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
        device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, segments, vertices.data(), sizeof(vertex));
    }

    void outlined_circle(std::int32_t x, std::int32_t y, std::int32_t r, color col, std::uint8_t thickness = 1, std::uint8_t segments = 64) {
        if (!render::device_checks()) {
            return;
        }

        for (std::size_t t = 0; t < thickness; t++) {
            std::vector<vertex> vertices;
            vertices.reserve(segments + 1);

            float radius = r + t;

            for (std::size_t i = 0; i <= segments; i++) {
                float angle = (2.0f * std::numbers::pi_v<float> * i) / segments;

                float x1 = x + std::cos(angle) * radius;
                float y1 = y + std::sin(angle) * radius;

                vertices.push_back({ (float)x1, (float)y1, 0.f, 1.f, col.to_d3d() });
            }

            device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
            device->DrawPrimitiveUP(D3DPT_LINESTRIP, segments, vertices.data(), sizeof(vertex));
        }
    }

    void line(std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2, color col, std::uint8_t thickness = 1) {
        if (!render::device_checks()) {
            return;
        }

        for (std::size_t i = 0; i < thickness; i++) {
            std::vector<vertex> vertices = {
            { (float)x1, (float)(y1 + i), 0.f, 1.f, col.to_d3d() },
            { (float)x2, (float)(y2 + i), 0.f, 1.f, col.to_d3d() }
            };

            device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
            device->DrawPrimitiveUP(D3DPT_LINELIST, 1, vertices.data(), sizeof(vertex));
        }
    }
}   
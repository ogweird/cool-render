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

enum font_weight {
    normal = FW_NORMAL,
    thin = FW_THIN,
    bold = FW_BOLD
};

inline std::unordered_map<std::string, ID3DXFont*> fonts = {};

namespace render {
    inline LPDIRECT3DDEVICE9 device = nullptr;

    void create_font(const std::string& name, const char* font_name, font_weight weight, std::uint8_t height, std::uint8_t width = 0, bool italic = false, std::uint8_t mip = 1) {
        auto it = fonts.find(name);

        if (it != fonts.end()) {
            return;
        }

        fonts.insert({ name, nullptr });

        D3DXCreateFontA(render::device, height, width, weight, mip, italic, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, font_name, &fonts[name]);
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
        auto it = fonts.find(font);

        if (it == fonts.end() || !it->second) {
            return;
        }

        RECT space{ x, y };

        it->second->DrawTextA(nullptr, text, -1, &space, DT_NOCLIP, col.to_d3d());
    }

    void filled_rect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, color col) {
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
        render::filled_rect(x, y, w, thickness, col);
        render::filled_rect(x, y, thickness, h, col);
        render::filled_rect(x + w - thickness, y, thickness, h, col);
        render::filled_rect(x, y + h - thickness, w, thickness, col);
    }

    void filled_circle(std::int32_t x, std::int32_t y, std::int32_t r, color col, std::uint8_t segments = 64) {
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
}
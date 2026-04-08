#pragma once

#define _USE_MATH_DEFINES

#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <vector>
#include <map>
#include <cmath>
#include <math.h>

struct color {
    int r, g, b, a;

    color() = default;
    color(int r, int g, int b, int a = 255) : r(r), g(g), b(b), a(a) {}

    D3DCOLOR to_d3d() {
        return D3DCOLOR_ARGB(this->a, this->r, this->g, this->b);
    }
};

struct vertex {
    float x, y, z, rhw;
    D3DCOLOR color;
};

namespace colors {
    auto white = color(255, 255, 255);
    auto black = color(0, 0, 0);
    auto red = color(255, 0, 0);
    auto green = color(0, 255, 0);
    auto blue = color(0, 0, 255);
}

enum class font_weight {
    normal = FW_NORMAL,
    thin = FW_THIN,
    bold = FW_BOLD
};

std::map<const char*, ID3DXFont*> fonts = {};

namespace render {
    LPDIRECT3DDEVICE9 device = nullptr;

    void create_font(const char* name, const char* font_name, unsigned int weight, int height, int width = 0, bool italic = false, int mip = 1) {
        if (fonts[name] != nullptr) {
            return;
        }

        fonts.insert({ name, nullptr });

        D3DXCreateFontA(render::device, height, width, weight, mip, italic, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, font_name, &fonts[name]);
    }

    void text(const char* font, int x, int y, const char* text, color col) {
        RECT space{ x, y, x + 1000, y + 100 };

        fonts[font]->DrawTextA(nullptr, text, -1, &space, DT_NOCLIP, col.to_d3d());
    }

    void filled_rect(int x, int y, int w, int h, color col) {
        std::vector<vertex> vertices = {
            { (float)x, (float)y, 0.f, 1.f, col.to_d3d() },
            { (float)(x + w), (float)y, 0.f, 1.f, col.to_d3d() },
            { (float)x, (float)(y + h), 0.f, 1.f, col.to_d3d() },
            { (float)(x + w), (float)(y + h), 0.f, 1.f, col.to_d3d() }
        };

        render::device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
        render::device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices.data(), sizeof(vertex));
    }

    void outlined_rect(int x, int y, int w, int h, color col, int thickness = 1) {
        render::filled_rect(x, y, w, thickness, col);
        render::filled_rect(x, y, thickness, h, col);
        render::filled_rect(x + w - thickness, y, thickness, h, col);
        render::filled_rect(x, y + h - thickness, w, thickness, col);
    }

    void filled_circle(int x, int y, int r, color col, int segments = 64) {
        std::vector<vertex> vertices;
        vertices.reserve(segments + 2);

        vertices.push_back({ (float)x, (float)y, 0.f, 1.f, col.to_d3d() });

        for (int i = 0; i <= segments; i++) {
            float angle = (2.0f * M_PI * i) / segments;
            float x1 = x + std::cos(angle) * r;
            float y1 = y + std::sin(angle) * r;

            vertices.push_back({ x1, y1, 0.f, 1.f, col.to_d3d() });
        }

        device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
        device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, segments, vertices.data(), sizeof(vertex));
    }

    void outlined_circle(int x, int y, int r, color col, int thickness = 1, int segments = 64) {
        for (int t = 0; t < thickness; t++) {
            std::vector<vertex> vertices;
            vertices.reserve(segments + 1);

            float radius = r + t;

            for (int i = 0; i <= segments; i++) {
                float angle = (2.0f * M_PI * i) / segments;

                float x1 = x + std::cos(angle) * radius;
                float y1 = y + std::sin(angle) * radius;

                vertices.push_back({ (float)x1, (float)y1, 0.f, 1.f, col.to_d3d() });
            }

            device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
            device->DrawPrimitiveUP(D3DPT_LINESTRIP, segments, vertices.data(), sizeof(vertex));
        }
    }
}
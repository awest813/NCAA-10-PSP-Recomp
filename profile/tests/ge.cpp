// Execute real GE display lists and inspect their pixels in both backends.
#include "ge.hpp"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <array>
#include <bit>
#include <cassert>
#include <iostream>

namespace {
constexpr std::uint32_t list = 0x08800000u, vertices = 0x08810000u;
constexpr std::uint32_t sample = 0x04000000u + (32u * 64u + 32u) * 4u;
class DisplayList {
public:
    explicit DisplayList(psprecomp::GuestMemory &memory) : memory_(memory) {}
    void command(std::uint32_t op, std::uint32_t data) {
        memory_.store32(list + size_++ * 4u, (op << 24u) | (data & 0xFFFFFFu));
    }
    void number(std::uint32_t op, float value) { command(op, std::bit_cast<std::uint32_t>(value) >> 8u); }
    void draw(unsigned type = 3, unsigned count = 3) {
        command(0x10, 0x080000u);
        command(0x01, vertices & 0xFFFFFFu);
        command(0x04, (type << 16u) | count);
    }
    void run(pspweb::Ge &ge) { command(0x0C, 0u); assert(ge.run_list(list, 0u)); }
private:
    psprecomp::GuestMemory &memory_;
    std::uint32_t size_{};
};

void expect_pixel(psprecomp::GuestMemory &memory, pspweb::GeGl *gl, std::uint32_t rgb, unsigned tolerance = 0,
                  std::uint32_t address = sample) {
    if (gl != nullptr) {
        gl->flush();
        // A prior pixel read can already have synchronized this whole surface.
        gl->prepare_vram_read(address, 4u);
        assert(glGetError() == GL_NO_ERROR);
    }
    const auto actual = memory.load32(address) & 0xFFFFFFu;
    bool matches = true;
    for (unsigned shift = 0; shift < 24; shift += 8) {
        const int delta = static_cast<int>((actual >> shift) & 255) - static_cast<int>((rgb >> shift) & 255);
        matches &= std::abs(delta) <= static_cast<int>(tolerance);
    }
    if (!matches) std::cerr << "pixel: expected 0x" << std::hex << rgb << ", got 0x" << actual << std::dec << '\n';
    assert(matches);
}

void lighting_colors(bool use_gl) {
    psprecomp::GuestMemory memory;
    pspweb::GeGl gl(memory);
    pspweb::Ge ge(memory);
    if (use_gl) { assert(gl.init()); ge.set_gl(&gl); }
    auto *backend = use_gl ? &gl : nullptr;
    // Float normals and positions, no vertex color: use the material registers.
    const std::array<float, 18> data = {0, 0, 1, -.8f, -.8f, 0,
                                      0, 0, 1, .8f, -.8f, 0,
                                      0, 0, 1, 0, .8f, 0};
    for (unsigned i = 0; i < data.size(); ++i) memory.store32(vertices + i * 4u, std::bit_cast<std::uint32_t>(data[i]));
    DisplayList setup(memory);
    setup.command(0x9C, 0); setup.command(0x9D, 64); setup.command(0xD2, 3);
    setup.command(0xD4, 0); setup.command(0xD5, 63 | (63 << 10));
    setup.command(0x12, (3u << 5u) | (3u << 7u));
    setup.number(0x42, 32); setup.number(0x43, 32); setup.number(0x44, 32767);
    setup.number(0x45, 32); setup.number(0x46, 32); setup.number(0x47, 32767);
    setup.command(0x17, 1);
    setup.command(0x55, 0xFFFFFF); setup.command(0x56, 0xFFFFFF);
    setup.command(0x58, 255); setup.command(0x5D, 255);
    setup.run(ge);
    for (unsigned light = 0; light < 4; ++light) {
        const unsigned ambient = 0x8F + light * 3, diffuse = ambient + 1;
        DisplayList initial(memory);
        for (unsigned i = 0; i < 4; ++i) initial.command(0x18 + i, i == light ? 1 : 0);
        initial.number(0x63 + light * 3, 0); initial.number(0x64 + light * 3, 0);
        initial.number(0x65 + light * 3, 1);
        initial.command(ambient, 0x0000FF); initial.command(diffuse, 0);
        initial.draw(); initial.run(ge);
        expect_pixel(memory, backend, 0x0000FF);

        // Change ONLY the color, leaving every other lighting register intact.
        DisplayList green(memory);
        green.command(ambient, 0x00FF00); green.draw(); green.run(ge);
        expect_pixel(memory, backend, 0x00FF00);

        DisplayList blue(memory);
        blue.command(ambient, 0); blue.command(diffuse, 0xFF0000);
        blue.draw(); blue.run(ge);
        expect_pixel(memory, backend, 0xFF0000);

        DisplayList yellow(memory);
        yellow.command(diffuse, 0x00FFFF); yellow.draw(); yellow.run(ge);
        expect_pixel(memory, backend, 0x00FFFF);
    }
}

void lighting_components(bool use_gl) {
    psprecomp::GuestMemory memory;
    pspweb::GeGl gl(memory);
    pspweb::Ge ge(memory);
    if (use_gl) { assert(gl.init()); ge.set_gl(&gl); }
    auto *backend = use_gl ? &gl : nullptr;
    auto normals = [&](float x, float y, float z) {
        const std::array<float, 18> data = {x, y, z, -.8f, -.8f, 0,
                                          x, y, z, .8f, -.8f, 0,
                                          x, y, z, 0, .8f, 0};
        for (unsigned i = 0; i < data.size(); ++i) memory.store32(vertices + i * 4u, std::bit_cast<std::uint32_t>(data[i]));
    };
    normals(0, 0, 1);
    DisplayList setup(memory);
    setup.command(0x9C, 0); setup.command(0x9D, 64); setup.command(0xD2, 3);
    setup.command(0xD4, 0); setup.command(0xD5, 63 | (63 << 10));
    setup.command(0x12, (3u << 5u) | (3u << 7u));
    setup.number(0x42, 32); setup.number(0x43, 32); setup.number(0x44, 32767);
    setup.number(0x45, 32); setup.number(0x46, 32); setup.number(0x47, 32767);
    setup.command(0x17, 1); setup.command(0x18, 1);
    setup.command(0x58, 255); setup.command(0x5D, 255);
    setup.command(0x57, 0x804020); setup.number(0x5B, 2);
    setup.command(0x5F, 1); // directional, diffuse and specular
    setup.number(0x65, 1); setup.command(0x91, 0xFFFFFF);
    setup.draw(); setup.run(ge);
    expect_pixel(memory, backend, 0x804020);

    // A black texture removes combined highlights but must leave separate
    // highlights intact, because secondary RGB is added after texturing.
    constexpr std::uint32_t texture = vertices + 0x1000;
    memory.store32(texture, 0xFF000000);
    DisplayList black(memory);
    black.command(0xA0, texture & 0xFFFFFF); black.command(0xA8, 0x080001);
    black.command(0xB8, 0); black.command(0xC3, 3); black.command(0x1E, 1);
    black.draw(); black.run(ge);
    expect_pixel(memory, backend, 0);
    DisplayList separate(memory);
    separate.command(0x5E, 1); separate.draw(); separate.run(ge);
    expect_pixel(memory, backend, 0x804020);

    // Fog follows the texture operation AND separate specular addition.
    DisplayList fogged_highlight(memory);
    fogged_highlight.command(0x1F, 1); fogged_highlight.number(0xCD, .5f);
    fogged_highlight.number(0xCE, 1); fogged_highlight.command(0xCF, 0x00FF00);
    fogged_highlight.draw(); fogged_highlight.run(ge);
    expect_pixel(memory, backend, 0x409F10);
    DisplayList unfogged_highlight(memory);
    unfogged_highlight.command(0x1F, 0); unfogged_highlight.draw(); unfogged_highlight.run(ge);
    expect_pixel(memory, backend, 0x804020);

    DisplayList blue(memory);
    blue.command(0x57, 0xFFFFFF); blue.command(0x91, 0xFF0000);
    blue.draw(); blue.run(ge);
    expect_pixel(memory, backend, 0xFF0000);
    DisplayList diffuse_only(memory);
    diffuse_only.command(0x5F, 0); diffuse_only.draw(); diffuse_only.run(ge);
    expect_pixel(memory, backend, 0);

    // Vertex material tracking has no effect without a color attribute.
    DisplayList material(memory);
    material.command(0x1E, 0); material.command(0x5E, 0);
    material.command(0x55, 0x00FF00); material.command(0x56, 0x0000FF);
    material.command(0x53, 2); material.command(0x90, 0xFFFFFF);
    material.draw(); material.run(ge);
    expect_pixel(memory, backend, 0x0000FF);

    // N.L = 1/2. Powered diffuse with exponent 2 gives 1/4; the GE's
    // 8-bit lighting scale maps full white to 64, not floating-point 63.75.
    normals(0.8660254f, 0, .5f);
    DisplayList powered(memory);
    powered.command(0x53, 0); powered.command(0x56, 0xFFFFFF);
    powered.command(0x5F, 2); powered.number(0x5B, 2);
    powered.draw(); powered.run(ge);
    expect_pixel(memory, backend, 0x404040);
    // The exponent keeps four mantissa bits: 2.99 -> 2.875. The GE's
    // piecewise-linear power at 1/2 then gives .140625, an 8-bit factor of 36.
    DisplayList exponent(memory);
    exponent.number(0x5B, 2.99f); exponent.draw(); exponent.run(ge);
    expect_pixel(memory, backend, 0x242424);
    DisplayList constant(memory);
    constant.number(0x5B, 0); constant.draw(); constant.run(ge);
    expect_pixel(memory, backend, 0xFFFFFF);

    normals(0, 0, 1);
    DisplayList reverse(memory);
    reverse.command(0x5F, 1); reverse.command(0x90, 0); reverse.number(0x5B, 0);
    reverse.command(0x51, 1); reverse.draw(); reverse.run(ge);
    expect_pixel(memory, backend, 0); // Back-facing normal suppresses specular even at exponent zero.
    DisplayList restore(memory);
    restore.command(0x51, 0); restore.draw(); restore.run(ge);
    expect_pixel(memory, backend, 0xFF0000);

    DisplayList viewer(memory);
    viewer.command(0x91, 0xFFFFFF); viewer.number(0x5B, 2);
    viewer.command(0x3C, 2); viewer.number(0x3D, 1);
    viewer.command(0x3C, 8); viewer.number(0x3D, 0);
    viewer.draw(); viewer.run(ge);
    // N.H = 1/sqrt(2); the GE power/8-bit scale yields 117.
    expect_pixel(memory, backend, 0x757575);

    DisplayList point(memory);
    point.command(0x3C, 2); point.number(0x3D, 0);
    point.command(0x3C, 8); point.number(0x3D, 1);
    point.command(0x55, 0xFFFFFF); point.command(0x5F, 0x100);
    point.command(0x8F, 0xFFFFFF); point.number(0x7B, 2);
    point.draw(); point.run(ge);
    expect_pixel(memory, backend, 0x808080);
    DisplayList zero_denominator(memory);
    zero_denominator.number(0x7B, 0); zero_denominator.draw(); zero_denominator.run(ge);
    expect_pixel(memory, backend, 0);
    DisplayList spot_outside(memory);
    spot_outside.command(0x5F, 0x200); spot_outside.number(0x7B, 2);
    spot_outside.number(0x71, 1); spot_outside.number(0x8B, .9f);
    spot_outside.draw(); spot_outside.run(ge);
    expect_pixel(memory, backend, 0);
    DisplayList spot_inside(memory);
    spot_inside.number(0x8B, .5f); spot_inside.number(0x87, 0);
    spot_inside.draw(); spot_inside.run(ge);
    expect_pixel(memory, backend, 0x808080);

    // Secondary color has no alpha. The same global/material ambient alpha
    // product remains 64 even when a full-bright highlight is added.
    DisplayList alpha(memory);
    alpha.command(0x5F, 1); alpha.command(0x8F, 0); alpha.number(0x5B, 2);
    alpha.command(0x58, 128); alpha.command(0x5D, 128); alpha.command(0x5E, 1);
    alpha.command(0x21, 1); alpha.command(0xDF, 0xA2); alpha.command(0xE1, 0);
    alpha.draw(); alpha.run(ge);
    expect_pixel(memory, backend, 0x404040); // full highlight * source alpha 64/255

    // Distinct clip w values must not distort color interpolation: the PSP
    // interpolates primary and secondary RGB in screen space. Only UV uses w.
    const std::array<float, 18> perspective_data = {0, 0, 1, -1.5f, -1.5f, 0,
                                                  1, 0, 0, 3, -3, 2,
                                                  1, 0, 0, 0, 6, 6};
    for (unsigned i = 0; i < perspective_data.size(); ++i)
        memory.store32(vertices + i * 4u, std::bit_cast<std::uint32_t>(perspective_data[i]));
    const std::array<float, 16> projection = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2};
    DisplayList interpolate(memory);
    interpolate.command(0x21, 0);
    interpolate.command(0x50, 1); interpolate.command(0x91, 0x0000FF);
    interpolate.command(0x3E, 0);
    for (float value : projection) interpolate.number(0x3F, value);
    interpolate.draw(); interpolate.run(ge);
    // Pixel center (32.5,32.5) has barycentric weight 0.234375 for red A.
    expect_pixel(memory, backend, 60, 1); // software truncates; GL rounds to RGBA8.
    DisplayList primary_interpolate(memory);
    primary_interpolate.command(0x5E, 0); primary_interpolate.draw(); primary_interpolate.run(ge);
    expect_pixel(memory, backend, 60, 1);

    // Flat shading must retain the original last vertex's color when that
    // vertex is clipped away, rather than choosing a generated intersection.
    const std::array<float, 18> clip_data = {1, 0, 0, -.8f, -.8f, 0,
                                           1, 0, 0, .8f, -.8f, 0,
                                           0, 0, 1, 0, .8f, -1.5f};
    for (unsigned i = 0; i < clip_data.size(); ++i)
        memory.store32(vertices + i * 4u, std::bit_cast<std::uint32_t>(clip_data[i]));
    DisplayList clip(memory);
    clip.command(0x50, 0); clip.command(0x3E, 0);
    for (unsigned i = 0; i < 16; ++i) clip.number(0x3F, i % 5 == 0 ? 1.0f : 0.0f);
    clip.draw(); clip.run(ge);
    expect_pixel(memory, backend, 0x0000FF);
    DisplayList separate_clip(memory);
    separate_clip.command(0x5E, 1); separate_clip.command(0x1E, 1);
    separate_clip.draw(); separate_clip.run(ge);
    expect_pixel(memory, backend, 0x0000FF);

    // With a color attribute present, material bit 4 tracks specular RGB.
    const std::array<float, 18> colored_data = {0, 0, 1, -.8f, -.8f, 0,
                                              0, 0, 1, .8f, -.8f, 0,
                                              0, 0, 1, 0, .8f, 0};
    for (unsigned v = 0; v < 3; ++v) {
        memory.store32(vertices + v * 28, 0xFF00FF00);
        for (unsigned i = 0; i < 6; ++i)
            memory.store32(vertices + v * 28 + 4 + i * 4, std::bit_cast<std::uint32_t>(colored_data[v * 6 + i]));
    }
    DisplayList tracked_specular(memory);
    tracked_specular.command(0x12, (7u << 2u) | (3u << 5u) | (3u << 7u));
    tracked_specular.command(0x53, 4); tracked_specular.command(0x91, 0xFFFFFF);
    tracked_specular.draw(); tracked_specular.run(ge);
    expect_pixel(memory, backend, 0x00FF00);
    DisplayList untracked_specular(memory);
    untracked_specular.command(0x53, 0); untracked_specular.draw(); untracked_specular.run(ge);
    expect_pixel(memory, backend, 0xFFFFFF);
}

void texture_coordinates(bool use_gl) {
    std::cout << "Texture-coordinate tests: " << (use_gl ? "GLES3" : "software") << std::endl;
    psprecomp::GuestMemory memory;
    pspweb::GeGl gl(memory);
    pspweb::Ge ge(memory);
    if (use_gl) { assert(gl.init()); ge.set_gl(&gl); }
    auto *backend = use_gl ? &gl : nullptr;
    constexpr std::uint32_t texture = vertices + 0x1000;
    // Each nearest-filtered texel identifies its x/y with separate channels.
    for (unsigned y = 0; y < 16; ++y)
        for (unsigned x = 0; x < 16; ++x)
            memory.store32(texture + (y * 16 + x) * 4, 0xFF400000 | (y * 16 << 8) | (x * 16));
    auto geometry = [&](const std::array<float, 24> &data) {
        // UV float2, normal float3, position float3.
        for (unsigned i = 0; i < data.size(); ++i)
            memory.store32(vertices + i * 4, std::bit_cast<std::uint32_t>(data[i]));
    };
    const std::array<float, 24> normal_data = {0, 0, 0, 0, 2, -.75f, -.75f, 0,
                                             0, 0, 0, 0, 2, .75f, -.75f, 0,
                                             0, 0, 0, 0, 2, 0, .75f, 0};
    geometry(normal_data);
    auto matrix = [&](DisplayList &commands, const std::array<float, 12> &values) {
        commands.command(0x40, 0);
        for (float value : values) commands.number(0x41, value);
    };
    auto identity_projection = [&](DisplayList &commands) {
        commands.command(0x3E, 0);
        for (unsigned i = 0; i < 16; ++i) commands.number(0x3F, i % 5 == 0 ? 1.0f : 0.0f);
    };
    DisplayList setup(memory);
    setup.command(0x9C, 0); setup.command(0x9D, 64); setup.command(0xD2, 3);
    setup.command(0xD4, 0); setup.command(0xD5, 63 | (63 << 10));
    setup.command(0x12, 3u | (3u << 5u) | (3u << 7u));
    setup.number(0x42, 32); setup.number(0x43, 32); setup.number(0x44, 32767);
    setup.number(0x45, 32); setup.number(0x46, 32); setup.number(0x47, 32767);
    setup.command(0x55, 0xFFFFFF); setup.command(0x58, 255);
    setup.command(0xA0, texture & 0xFFFFFF); setup.command(0xA8, 0x080010);
    setup.command(0xB8, 0x0404); setup.command(0xC3, 3); setup.command(0x1E, 1);
    setup.command(0xC7, 0x0101); setup.command(0xC9, 3); // clamp, replace RGB
    setup.number(0x48, 7); setup.number(0x49, 5); // ignored by matrix/shade modes
    setup.number(0x4A, 3); setup.number(0x4B, 2);
    setup.command(0xC0, 0x0201); // normalized model normal
    matrix(setup, {0, 0, 0, 0, 0, 0, .25f, 0, 0, .125f, .25f, 1});
    setup.draw(); setup.run(ge);
    expect_pixel(memory, backend, 0x404060); // (6,4), not raw normal's (10,4)
    DisplayList raw_normal(memory);
    raw_normal.command(0xC0, 0x0301); raw_normal.draw(); raw_normal.run(ge);
    expect_pixel(memory, backend, 0x4040A0);
    DisplayList reversed(memory);
    reversed.command(0x51, 1); reversed.command(0xC0, 0x0201);
    reversed.draw(); reversed.run(ge);
    expect_pixel(memory, backend, 0x404000); // negative s clamps at zero

    // Different texture q AND clip w: divide interpolated s/w by q/w,
    // rather than projecting each vertex's s/q before interpolation.
    geometry({0, 0, 0, 0, 1, -.75f, -.75f, -1,
              0, 0, 0, 0, 1, 1.5f, -1.5f, 0,
              0, 0, 0, 0, 1, 0, 3, 2});
    DisplayList projective(memory);
    projective.command(0x51, 0); projective.command(0xC0, 1);
    matrix(projective, {0, 0, 0, 0, 0, 0, 0, 0, 1, .5f, .25f, 2});
    projective.command(0x3E, 0);
    for (float value : std::array<float, 16>{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2})
        projective.number(0x3F, value);
    projective.draw(); projective.run(ge);
    // At (32.5,32.5): screen weights 45/192,49/192,98/192;
    // s/q = .24479166, t/q = .12239583, hence nearest texel (3,1).
    expect_pixel(memory, backend, 0x401030);

    // PSP's documented index formats are 8/16-bit. Leave holes and use high
    // 16-bit indices so treating this as a plain draw or byte indices fails.
    for (unsigned index_type : {1u, 2u}) {
        for (bool shared : {false, true}) {
            constexpr auto indices = vertices + 0x800;
            geometry({}); // a plain draw is degenerate
            const unsigned first = index_type == 1 ? 2 : 258;
            const std::array<float, 24> triangle = {0, 0, 0, 0, 1, -.75f, -.75f, -1,
                                                   0, 0, 0, 0, 1, 1.5f, -1.5f, 0,
                                                   0, 0, 0, 0, 1, 0, 3, 2};
            for (unsigned v = 0; v < 3; ++v)
                for (unsigned c = 0; c < 8; ++c)
                    memory.store32(vertices + ((first + v * 2) * 8 + c) * 4,
                                   std::bit_cast<std::uint32_t>(triangle[v * 8 + c]));
            const unsigned count = shared ? 6 : 3;
            for (unsigned i = 0; i < count; ++i) {
                const auto index = first + (i % 3) * 2;
                if (index_type == 1) memory.store8(indices + i, index);
                else memory.store16(indices + i * 2, index);
            }
            DisplayList indexed(memory);
            indexed.command(0x12, 3u | (3u << 5u) | (3u << 7u) | (index_type << 11u));
            indexed.command(0x10, 0x080000u);
            indexed.command(0x02, indices & 0xFFFFFFu);
            matrix(indexed, {0, 0, 0, 0, 0, 0, 0, 0, 1, shared ? .25f : .75f, .25f, 2});
            indexed.draw(3, count); indexed.run(ge);
            expect_pixel(memory, backend, shared ? 0x401010 : 0x401050);
        }
    }

    // A near-plane intersection must interpolate texture q as well as s/t.
    geometry({0, 0, 0, 0, 1, -.75f, -.75f, 0,
              1, 0, 0, 0, 1, .75f, -.75f, 0,
              3, 0, 0, 0, 1, 0, .75f, -1.5f});
    DisplayList clipped(memory);
    clipped.command(0x12, 3u | (3u << 5u) | (3u << 7u));
    identity_projection(clipped); clipped.command(0xC0, 0x0101);
    matrix(clipped, {0, 0, 1, 0, 0, 0, 0, 0, 0, .5f, .25f, 1});
    clipped.draw(); clipped.run(ge);
    expect_pixel(memory, backend, 0x401020); // interpolated q = 535/192

    // Zero and negative q saturate both axes even if the texture wraps.
    for (float q : {0.0f, -1.0f}) {
        DisplayList nonpositive(memory);
        nonpositive.command(0xC7, 0);
        matrix(nonpositive, {0, 0, 0, 0, 0, 0, 0, 0, 0, -.5f, 2, q});
        nonpositive.draw(); nonpositive.run(ge);
        expect_pixel(memory, backend, 0x40F0F0);
    }
    // Rectangles carry left/right q instead of projecting their endpoints.
    geometry({0, 0, 0, 0, 1, -.75f, -.75f, 0,
              3, 0, 0, 0, 1, .75f, .75f, 0,
              0, 0, 0, 0, 1, 0, 0, 0});
    DisplayList rectangle(memory);
    rectangle.command(0xC7, 0x0101);
    matrix(rectangle, {0, 0, 1, 0, 0, 0, 0, 0, 0, .5f, .25f, 1});
    rectangle.draw(6, 2); rectangle.run(ge);
    expect_pixel(memory, backend, 0x401030);

    // A point's single texture q also divides its generated coordinates.
    geometry({0, 0, 0, 0, 1, 0, 0, 0,
              0, 0, 0, 0, 1, 0, 0, 0,
              0, 0, 0, 0, 1, 0, 0, 0});
    DisplayList point(memory);
    matrix(point, {0, 0, 0, 0, 0, 0, 0, 0, 0, .5f, .25f, 2});
    point.draw(0, 1); point.run(ge);
    expect_pixel(memory, backend, 0x402040);

    // Through vertices bypass texture generation, projection and UV scale.
    geometry({2, 4, 0, 0, 1, 8, 8, 0,
              2, 4, 0, 0, 1, 56, 56, 0,
              0, 0, 0, 0, 1, 0, 0, 0});
    DisplayList through(memory);
    through.command(0x12, 3u | (3u << 5u) | (3u << 7u) | (1u << 23u));
    through.draw(6, 2); through.run(ge);
    expect_pixel(memory, backend, 0x404020);

    // Shade mapping uses the selected light vectors even when lighting and
    // all light channels are disabled. The two selector fields are independent.
    geometry(normal_data);
    DisplayList shade(memory);
    shade.command(0x12, 3u | (3u << 5u) | (3u << 7u));
    shade.command(0xC0, 2); shade.command(0xC1, 1u << 8u);
    shade.number(0x65, 1); shade.number(0x68, -1);
    shade.draw(); shade.run(ge);
    expect_pixel(memory, backend, 0x4000F0);
    DisplayList swap(memory);
    swap.command(0xC1, 1); swap.draw(); swap.run(ge);
    expect_pixel(memory, backend, 0x40F000);
    DisplayList changed_light(memory);
    changed_light.number(0x68, 1); changed_light.draw(); changed_light.run(ge);
    expect_pixel(memory, backend, 0x40F0F0);
    DisplayList reversed_shade(memory);
    reversed_shade.command(0x51, 1); reversed_shade.draw(); reversed_shade.run(ge);
    expect_pixel(memory, backend, 0x400000);

    // A specular shade vector is L + viewer direction, normalized. Only a
    // light-type or view-matrix change must refresh the cached mapping state.
    DisplayList diffuse_shade(memory);
    diffuse_shade.command(0x51, 0);
    diffuse_shade.number(0x66, 1); diffuse_shade.number(0x68, 0);
    diffuse_shade.draw(); diffuse_shade.run(ge);
    expect_pixel(memory, backend, 0x40F080);
    DisplayList specular_shade(memory);
    specular_shade.command(0x60, 1); specular_shade.draw(); specular_shade.run(ge);
    expect_pixel(memory, backend, 0x40F0D0);
    DisplayList view_shade(memory);
    view_shade.command(0x3C, 0);
    for (float value : std::array<float, 12>{1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0})
        view_shade.number(0x3D, value);
    view_shade.draw(); view_shade.run(ge);
    expect_pixel(memory, backend, 0x40F080);

    DisplayList point_shade(memory);
    point_shade.command(0x3C, 0);
    for (float value : std::array<float, 12>{1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0})
        point_shade.number(0x3D, value);
    point_shade.command(0x60, 0x0100);
    point_shade.number(0x66, 0); point_shade.number(0x68, 1);
    point_shade.draw(); point_shade.run(ge);
    expect_pixel(memory, backend, 0x40F0D0);
    DisplayList world_shade(memory);
    world_shade.command(0x3A, 0);
    for (float value : std::array<float, 12>{1, 0, 0, 0, 1, 0, 0, 0, 4, 0, 0, .5f})
        world_shade.number(0x3B, value);
    world_shade.draw(); world_shade.run(ge);
    expect_pixel(memory, backend, 0x40F0B0);

    // Mode 3 aliases ordinary UV generation. Explicit zero scales are valid,
    // and switching away from projection must reset the reused vertices' q.
    geometry({.125f, .25f, 0, 0, 1, -.75f, -.75f, 0,
              .125f, .25f, 0, 0, 1, .75f, -.75f, 0,
              .125f, .25f, 0, 0, 1, 0, .75f, 0});
    DisplayList direct(memory);
    direct.command(0xC0, 3);
    direct.number(0x48, 1); direct.number(0x49, 1);
    direct.number(0x4A, 0); direct.number(0x4B, 0);
    direct.draw(); direct.run(ge);
    expect_pixel(memory, backend, 0x404020);
    DisplayList zero_scale(memory);
    zero_scale.number(0x48, 0); zero_scale.number(0x49, 0);
    zero_scale.draw(); zero_scale.run(ge);
    expect_pixel(memory, backend, 0x400000);

    // Fixed-point normals use signed /128 and /32768, including the minimum
    // value -1 exactly. Check both direct decode and the morph accumulation.
    for (unsigned normal_type : {1u, 2u}) {
        for (unsigned morphs : {1u, 2u}) {
            const unsigned position_offset = normal_type == 1 ? 12 : 16;
            const unsigned stride = position_offset + 12;
            const std::array<float, 9> positions = {-.75f, -.75f, 0, .75f, -.75f, 0, 0, .75f, 0};
            for (unsigned vertex = 0; vertex < 3; ++vertex) {
                for (unsigned morph = 0; morph < morphs; ++morph) {
                    const auto address = vertices + (vertex * morphs + morph) * stride;
                    memory.store32(address, 0); memory.store32(address + 4, 0);
                    if (normal_type == 1) {
                        memory.store8(address + 8, 0); memory.store8(address + 9, 0);
                        memory.store8(address + 10, 0x80);
                    } else {
                        memory.store16(address + 8, 0); memory.store16(address + 10, 0);
                        memory.store16(address + 12, 0x8000);
                    }
                    for (unsigned i = 0; i < 3; ++i)
                        memory.store32(address + position_offset + i * 4, std::bit_cast<std::uint32_t>(positions[vertex * 3 + i]));
                }
            }
            DisplayList packed(memory);
            packed.command(0x3A, 0);
            for (float value : std::array<float, 12>{1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0})
                packed.number(0x3B, value);
            packed.command(0x12, 3u | (normal_type << 5u) | (3u << 7u) | ((morphs - 1) << 18u));
            packed.number(0x2C, .5f); packed.number(0x2D, .5f);
            packed.command(0xC0, 0x0301);
            matrix(packed, {0, 0, 0, 0, 0, 0, .25f, 0, 0, .5f, .25f, 1});
            packed.draw(); packed.run(ge);
            expect_pixel(memory, backend, 0x404040);
        }
    }
}

void fog_depth(bool use_gl) {
    std::cout << "Fog tests: " << (use_gl ? "GLES3" : "software") << std::endl;
    psprecomp::GuestMemory memory;
    pspweb::GeGl gl(memory);
    pspweb::Ge ge(memory);
    if (use_gl) { assert(gl.init()); ge.set_gl(&gl); }
    auto *backend = use_gl ? &gl : nullptr;
    const std::array<float, 9> positions = {-.75f, -.75f, -.5f, .75f, -.75f, -.5f, 0, .75f, -.5f};
    for (unsigned i = 0; i < positions.size(); ++i)
        memory.store32(vertices + i * 4, std::bit_cast<std::uint32_t>(positions[i]));
    DisplayList setup(memory);
    setup.command(0x9C, 0); setup.command(0x9D, 64); setup.command(0xD2, 3);
    setup.command(0xD4, 0); setup.command(0xD5, 63 | (63 << 10));
    setup.command(0x12, 3u << 7u);
    setup.number(0x42, 32); setup.number(0x43, 32); setup.number(0x44, 32767);
    setup.number(0x45, 32); setup.number(0x46, 32); setup.number(0x47, 32767);
    setup.command(0x55, 0x0000FF); setup.command(0x58, 255);
    setup.command(0x1F, 1); setup.number(0xCD, 1); setup.number(0xCE, 1);
    setup.command(0xCF, 0xFF0000);
    setup.draw(); setup.run(ge);
    // Orthographic eye z=-.5, end=1, slope=1 => fog factor 128.
    // RGB rounds up after (source * 128 + fog_color * 127) / 256.
    expect_pixel(memory, backend, 0x7F0080);

    auto geometry = [&](const std::array<float, 9> &data) {
        for (unsigned i = 0; i < data.size(); ++i)
            memory.store32(vertices + i * 4, std::bit_cast<std::uint32_t>(data[i]));
    };
    auto matrix43 = [&](DisplayList &commands, unsigned number, const std::array<float, 12> &data) {
        commands.command(number, 0);
        for (float value : data) commands.number(number + 1, value);
    };
    auto projection = [&](DisplayList &commands, const std::array<float, 16> &data) {
        commands.command(0x3E, 0);
        for (float value : data) commands.number(0x3F, value);
    };
    const std::array<float, 12> identity43 = {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0};
    const std::array<float, 16> identity44 = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

    DisplayList fog_color(memory);
    fog_color.command(0xCF, 0x00FF00); fog_color.draw(); fog_color.run(ge);
    expect_pixel(memory, backend, 0x007F80);
    DisplayList slope(memory);
    slope.number(0xCE, .5f); slope.draw(); slope.run(ge);
    expect_pixel(memory, backend, 0x00BF40);
    DisplayList end(memory);
    end.number(0xCD, .5f); end.draw(); end.run(ge);
    expect_pixel(memory, backend, 0x00FF00);
    DisplayList saturated(memory);
    saturated.number(0xCD, 3); saturated.draw(); saturated.run(ge);
    expect_pixel(memory, backend, 0x0000FF);
    DisplayList quantized(memory);
    quantized.number(0xCD, .5f + 127.75f / 256); quantized.number(0xCE, 1);
    quantized.draw(); quantized.run(ge);
    expect_pixel(memory, backend, 0x00807F); // factor truncates to 127, not 128

    // Different clip w must not perspective-correct fog. Vertex factors are
    // 0,64,192; screen weights at the sample are 45,49,98 divided by 192.
    geometry({-.75f, -.75f, -1, 1.5f, -1.5f, 0, 0, 3, 2});
    DisplayList affine(memory);
    affine.number(0xCD, 1); affine.number(0xCE, .25f); affine.command(0xCF, 0xFF0000);
    projection(affine, {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2});
    affine.draw(); affine.run(ge);
    expect_pixel(memory, backend, 0x8D0072); // flat shading does not flatten fog
    DisplayList gouraud(memory);
    gouraud.command(0x50, 1); gouraud.command(0xCF, 0x00FF00);
    gouraud.draw(); gouraud.run(ge);
    expect_pixel(memory, backend, 0x008D72);

    geometry(positions);
    DisplayList view(memory);
    projection(view, identity44);
    view.command(0xCF, 0xFF0000); view.number(0xCE, 1);
    matrix43(view, 0x3C, {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, .25f});
    view.draw(); view.run(ge);
    expect_pixel(memory, backend, 0x3F00C0);
    DisplayList world(memory);
    matrix43(world, 0x3A, {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, -.25f});
    world.draw(); world.run(ge);
    expect_pixel(memory, backend, 0x7F0080);

    // Skinning precedes the fog transform. One bone moves z from -.5 to -.25.
    for (unsigned v = 0; v < 3; ++v) {
        memory.store32(vertices + v * 16, std::bit_cast<std::uint32_t>(1.0f));
        for (unsigned c = 0; c < 3; ++c)
            memory.store32(vertices + v * 16 + 4 + c * 4, std::bit_cast<std::uint32_t>(positions[v * 3 + c]));
    }
    DisplayList skinned(memory);
    matrix43(skinned, 0x3A, identity43); matrix43(skinned, 0x3C, identity43);
    matrix43(skinned, 0x2A, {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, .25f});
    skinned.command(0x12, (3u << 7u) | (3u << 9u));
    skinned.draw(); skinned.run(ge);
    expect_pixel(memory, backend, 0x3F00C0);

    for (unsigned v = 0; v < 3; ++v)
        for (unsigned m = 0; m < 2; ++m)
            for (unsigned c = 0; c < 3; ++c) {
                const float value = c == 2 ? (m == 0 ? -.25f : -.75f) : positions[v * 3 + c];
                memory.store32(vertices + ((v * 2 + m) * 3 + c) * 4, std::bit_cast<std::uint32_t>(value));
            }
    DisplayList morph(memory);
    morph.command(0x12, (3u << 7u) | (1u << 18u));
    morph.number(0x2C, .5f); morph.number(0x2D, .5f);
    morph.draw(); morph.run(ge);
    expect_pixel(memory, backend, 0x7F0080);

    geometry({-.75f, -.75f, 0, .75f, -.75f, 0, 0, .75f, -1.5f});
    DisplayList clipped(memory);
    clipped.command(0x12, 3u << 7u); clipped.number(0xCE, .5f);
    clipped.draw(); clipped.run(ge);
    expect_pixel(memory, backend, 0xC1003E);

    // Rectangles select the opposite endpoint's fog in each horizontal half.
    geometry({-.75f, -.75f, -.75f, .75f, .75f, -.25f, 0, 0, 0});
    DisplayList rectangle(memory);
    rectangle.number(0xCE, 1); rectangle.draw(6, 2); rectangle.run(ge);
    expect_pixel(memory, backend, 0xBF0040);
    expect_pixel(memory, backend, 0x3F00C0, 0, sample - 12 * 4);
    geometry({.75f, -.75f, -.75f, -.75f, .75f, -.25f, 0, 0, 0});
    DisplayList reversed_rectangle(memory);
    reversed_rectangle.draw(6, 2); reversed_rectangle.run(ge);
    expect_pixel(memory, backend, 0x3F00C0);
    expect_pixel(memory, backend, 0xBF0040, 0, sample - 12 * 4);

    geometry({0, 0, -.5f, 0, 0, 0, 0, 0, 0});
    DisplayList point(memory);
    point.draw(0, 1); point.run(ge);
    expect_pixel(memory, backend, 0x7F0080);

    geometry({8, 8, -.75f, 56, 56, -.25f, 0, 0, 0});
    DisplayList through(memory);
    through.command(0x12, (3u << 7u) | (1u << 23u));
    through.draw(6, 2); through.run(ge);
    expect_pixel(memory, backend, 0x0000FF);
    geometry(positions);
    DisplayList transformed(memory);
    transformed.command(0x12, 3u << 7u); transformed.draw(); transformed.run(ge);
    expect_pixel(memory, backend, 0x7F0080);
    DisplayList clear(memory);
    clear.command(0xD3, 0x101); clear.draw(); clear.run(ge);
    expect_pixel(memory, backend, 0x0000FF);
    DisplayList uncleared(memory);
    uncleared.command(0xD3, 0); uncleared.draw(); uncleared.run(ge);
    expect_pixel(memory, backend, 0x7F0080);
    DisplayList disabled(memory);
    disabled.command(0x1F, 0); disabled.draw(); disabled.run(ge);
    expect_pixel(memory, backend, 0x0000FF);

    // Fog changes RGB only: an alpha test still sees the source alpha 128,
    // including a fully fogged pixel. Rejecting it would leave the red pixel.
    DisplayList alpha(memory);
    alpha.command(0x1F, 1); alpha.number(0xCD, .5f);
    alpha.command(0x58, 128); alpha.command(0x22, 1); alpha.command(0xDB, 0xFF8002);
    alpha.draw(); alpha.run(ge);
    expect_pixel(memory, backend, 0xFF0000);

    // Positive/negative nonfinite final factors follow their sign.
    for (std::uint32_t bits : {0x7F800000u, 0xFF800000u, 0x7FC00000u, 0xFFC00000u}) {
        DisplayList nonfinite(memory);
        nonfinite.number(0xCD, std::bit_cast<float>(bits));
        nonfinite.draw(); nonfinite.run(ge);
        expect_pixel(memory, backend, bits & 0x80000000u ? 0xFF0000 : 0x0000FF);
    }
}

struct GlContext {
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLContext context = EGL_NO_CONTEXT;
    EGLSurface surface = EGL_NO_SURFACE;
    GlContext() {
#if defined(_WIN32)
        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
#else
        auto get_display = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
        display = get_display ? get_display(EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, nullptr)
                              : eglGetDisplay(EGL_DEFAULT_DISPLAY);
#endif
        EGLint major, minor, count;
        assert(display != EGL_NO_DISPLAY && eglInitialize(display, &major, &minor));
        assert(eglBindAPI(EGL_OPENGL_ES_API));
        const EGLint attributes[] = {EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT, EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_NONE};
        EGLConfig config;
        assert(eglChooseConfig(display, attributes, &config, 1, &count) && count == 1);
        const EGLint context_attributes[] = {EGL_CONTEXT_MAJOR_VERSION, 3, EGL_NONE};
        context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attributes);
        const EGLint surface_attributes[] = {EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE};
        surface = eglCreatePbufferSurface(display, config, surface_attributes);
        assert(context != EGL_NO_CONTEXT && surface != EGL_NO_SURFACE);
        assert(eglMakeCurrent(display, surface, surface, context));
    }
    ~GlContext() {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroySurface(display, surface); eglDestroyContext(display, context); eglTerminate(display);
    }
};
}
int main() {
    lighting_colors(false);
    lighting_components(false);
    texture_coordinates(false);
    fog_depth(false);
    GlContext context;
    lighting_colors(true);
    lighting_components(true);
    texture_coordinates(true);
    fog_depth(true);
    std::cout << "GE lighting, texture-coordinate and fog pixel tests passed (software and GLES3)\n";
}

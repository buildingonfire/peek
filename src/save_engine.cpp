#include "save_engine.h"
#include "utils.h"
#include "inter_font.h"
#include "stb_image_write.h"
#include "stb_image_resize2.h"
#include "stb_truetype.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace peek {

static void blend_pixel(uint8_t* dst, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (a == 0) return;
    float alpha = a / 255.0f;
    float inv = 1.0f - alpha;
    dst[0] = (uint8_t)(r * alpha + dst[0] * inv);
    dst[1] = (uint8_t)(g * alpha + dst[1] * inv);
    dst[2] = (uint8_t)(b * alpha + dst[2] * inv);
    dst[3] = std::max(dst[3], a);
}

static void draw_line_on_buffer(uint8_t* buf, int w, int h, ImVec2 p0, ImVec2 p1, ImU32 col, float thickness) {
    uint8_t r = (col >> 0) & 0xFF;
    uint8_t g = (col >> 8) & 0xFF;
    uint8_t b = (col >> 16) & 0xFF;
    uint8_t a = (col >> 24) & 0xFF;

    float dx = p1.x - p0.x;
    float dy = p1.y - p0.y;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 0.5f) return;

    int steps = (int)(len * 2);
    float half_t = thickness * 0.5f;

    for (int s = 0; s <= steps; s++) {
        float t = (float)s / (float)steps;
        float cx = p0.x + dx * t;
        float cy = p0.y + dy * t;

        int x0 = std::max(0, (int)(cx - half_t));
        int y0 = std::max(0, (int)(cy - half_t));
        int x1 = std::min(w - 1, (int)(cx + half_t));
        int y1 = std::min(h - 1, (int)(cy + half_t));

        for (int y = y0; y <= y1; y++) {
            for (int x = x0; x <= x1; x++) {
                blend_pixel(&buf[(y * w + x) * 4], r, g, b, a);
            }
        }
    }
}

static void draw_rect_on_buffer(uint8_t* buf, int w, int h, ImVec2 p0, ImVec2 p1, ImU32 col, float thickness) {
    ImVec2 tl(std::min(p0.x, p1.x), std::min(p0.y, p1.y));
    ImVec2 tr(std::max(p0.x, p1.x), std::min(p0.y, p1.y));
    ImVec2 bl(std::min(p0.x, p1.x), std::max(p0.y, p1.y));
    ImVec2 br(std::max(p0.x, p1.x), std::max(p0.y, p1.y));
    draw_line_on_buffer(buf, w, h, tl, tr, col, thickness);
    draw_line_on_buffer(buf, w, h, tr, br, col, thickness);
    draw_line_on_buffer(buf, w, h, br, bl, col, thickness);
    draw_line_on_buffer(buf, w, h, bl, tl, col, thickness);
}

static void draw_triangle_filled_on_buffer(uint8_t* buf, int w, int h,
    ImVec2 v0, ImVec2 v1, ImVec2 v2, ImU32 col)
{
    uint8_t r = (col >> 0) & 0xFF;
    uint8_t g = (col >> 8) & 0xFF;
    uint8_t b = (col >> 16) & 0xFF;
    uint8_t a = (col >> 24) & 0xFF;

    int minx = std::max(0, (int)std::min({v0.x, v1.x, v2.x}));
    int maxx = std::min(w - 1, (int)std::max({v0.x, v1.x, v2.x}));
    int miny = std::max(0, (int)std::min({v0.y, v1.y, v2.y}));
    int maxy = std::min(h - 1, (int)std::max({v0.y, v1.y, v2.y}));

    for (int y = miny; y <= maxy; y++) {
        for (int x = minx; x <= maxx; x++) {
            float px = x + 0.5f, py = y + 0.5f;
            float d0 = (v1.x - v0.x) * (py - v0.y) - (v1.y - v0.y) * (px - v0.x);
            float d1 = (v2.x - v1.x) * (py - v1.y) - (v2.y - v1.y) * (px - v1.x);
            float d2 = (v0.x - v2.x) * (py - v2.y) - (v0.y - v2.y) * (px - v2.x);
            bool has_neg = (d0 < 0) || (d1 < 0) || (d2 < 0);
            bool has_pos = (d0 > 0) || (d1 > 0) || (d2 > 0);
            if (!(has_neg && has_pos)) {
                blend_pixel(&buf[(y * w + x) * 4], r, g, b, a);
            }
        }
    }
}

static void draw_text_on_buffer(uint8_t* buf, int buf_w, int buf_h,
    const char* text, ImVec2 pos, float font_size, ImU32 col,
    const uint8_t* font_data, int font_data_size)
{
    uint8_t r = (col >> 0) & 0xFF;
    uint8_t g = (col >> 8) & 0xFF;
    uint8_t b = (col >> 16) & 0xFF;

    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, font_data, stbtt_GetFontOffsetForIndex(font_data, 0)))
        return;

    float scale = stbtt_ScaleForPixelHeight(&font, font_size);
    int ascent;
    stbtt_GetFontVMetrics(&font, &ascent, nullptr, nullptr);
    int baseline = (int)(ascent * scale);

    float xpos = pos.x;
    for (const char* p = text; *p; p++) {
        int cw, ch, xoff, yoff;
        uint8_t* bitmap = stbtt_GetCodepointBitmap(&font, scale, scale, *p, &cw, &ch, &xoff, &yoff);
        if (bitmap) {
            for (int cy = 0; cy < ch; cy++) {
                for (int cx = 0; cx < cw; cx++) {
                    int px = (int)xpos + cx + xoff;
                    int py = (int)pos.y + cy + yoff + baseline;
                    if (px >= 0 && px < buf_w && py >= 0 && py < buf_h) {
                        uint8_t alpha = bitmap[cy * cw + cx];
                        if (alpha > 0) {
                            blend_pixel(&buf[(py * buf_w + px) * 4], r, g, b, alpha);
                        }
                    }
                }
            }
            stbtt_FreeBitmap(bitmap, nullptr);
        }
        int advance;
        stbtt_GetCodepointHMetrics(&font, *p, &advance, nullptr);
        xpos += advance * scale;
        if (*(p + 1)) {
            int kern = stbtt_GetCodepointKernAdvance(&font, *p, *(p + 1));
            xpos += kern * scale;
        }
    }
}

bool SaveEngine::save(const LoadedImage& img, const AnnotationLayer& layer,
                       const SaveOptions& opts, const std::string& original_path,
                       std::string& out_path) {
    if (!img.valid || img.pixels.empty()) return false;

    // Determine crop region
    int crop_x = 0, crop_y = 0, crop_w = img.width, crop_h = img.height;
    int crop_idx = layer.find_crop();
    if (crop_idx >= 0) {
        auto& crop = layer.annotations[crop_idx];
        crop_x = std::max(0, (int)std::min(crop.start.x, crop.end.x));
        crop_y = std::max(0, (int)std::min(crop.start.y, crop.end.y));
        int crop_r = std::min(img.width, (int)std::max(crop.start.x, crop.end.x));
        int crop_b = std::min(img.height, (int)std::max(crop.start.y, crop.end.y));
        crop_w = crop_r - crop_x;
        crop_h = crop_b - crop_y;
        if (crop_w <= 0 || crop_h <= 0) {
            crop_x = 0; crop_y = 0; crop_w = img.width; crop_h = img.height;
        }
    }

    // Create working buffer (cropped region)
    std::vector<uint8_t> buffer((size_t)crop_w * crop_h * 4);
    for (int y = 0; y < crop_h; y++) {
        memcpy(&buffer[y * crop_w * 4],
               &img.pixels[((crop_y + y) * img.width + crop_x) * 4],
               crop_w * 4);
    }

    // Draw annotations onto buffer (skip crop annotation)
    for (auto& ann : layer.annotations) {
        if (ann.type == AnnotationType::Crop) continue;

        // Offset annotation coordinates by crop origin
        auto offset_pt = [&](ImVec2 p) -> ImVec2 {
            return ImVec2(p.x - crop_x, p.y - crop_y);
        };

        switch (ann.type) {
            case AnnotationType::Freehand: {
                for (size_t i = 1; i < ann.points.size(); i++) {
                    draw_line_on_buffer(buffer.data(), crop_w, crop_h,
                        offset_pt(ann.points[i-1]), offset_pt(ann.points[i]),
                        ann.color, ann.thickness);
                }
                break;
            }
            case AnnotationType::Box: {
                draw_rect_on_buffer(buffer.data(), crop_w, crop_h,
                    offset_pt(ann.start), offset_pt(ann.end), ann.color, ann.thickness);
                break;
            }
            case AnnotationType::Arrow: {
                ImVec2 a = offset_pt(ann.start);
                ImVec2 b = offset_pt(ann.end);
                draw_line_on_buffer(buffer.data(), crop_w, crop_h, a, b, ann.color, ann.thickness);
                // Arrowhead
                float dx = b.x - a.x, dy = b.y - a.y;
                float len = sqrtf(dx * dx + dy * dy);
                if (len > 1.0f) {
                    dx /= len; dy /= len;
                    float px = -dy, py = dx;
                    float sz = std::max(10.0f, ann.thickness * 4.0f);
                    ImVec2 left(b.x - dx * sz + px * sz * 0.5f, b.y - dy * sz + py * sz * 0.5f);
                    ImVec2 right(b.x - dx * sz - px * sz * 0.5f, b.y - dy * sz - py * sz * 0.5f);
                    draw_triangle_filled_on_buffer(buffer.data(), crop_w, crop_h, b, left, right, ann.color);
                }
                break;
            }
            case AnnotationType::Text: {
                draw_text_on_buffer(buffer.data(), crop_w, crop_h,
                    ann.text.c_str(), offset_pt(ann.position), ann.font_size, ann.color,
                    inter_regular_ttf_data, (int)inter_regular_ttf_size);
                break;
            }
            default: break;
        }
    }

    // Scale if needed
    int out_w = (int)(crop_w * opts.scale);
    int out_h = (int)(crop_h * opts.scale);
    std::vector<uint8_t> scaled;
    const uint8_t* final_pixels = buffer.data();

    if (opts.scale != 1.0f && out_w > 0 && out_h > 0) {
        scaled.resize((size_t)out_w * out_h * 4);
        stbir_resize_uint8_linear(
            buffer.data(), crop_w, crop_h, crop_w * 4,
            scaled.data(), out_w, out_h, out_w * 4,
            STBIR_RGBA);
        final_pixels = scaled.data();
    } else {
        out_w = crop_w;
        out_h = crop_h;
    }

    // Strip alpha if saving as JPG or PNG without transparency
    int out_channels = 4;
    std::vector<uint8_t> rgb_pixels;
    if (!opts.save_as_png || !opts.png_transparency) {
        out_channels = 3;
        rgb_pixels.resize((size_t)out_w * out_h * 3);
        for (int i = 0; i < out_w * out_h; i++) {
            rgb_pixels[i * 3 + 0] = final_pixels[i * 4 + 0];
            rgb_pixels[i * 3 + 1] = final_pixels[i * 4 + 1];
            rgb_pixels[i * 3 + 2] = final_pixels[i * 4 + 2];
        }
        final_pixels = rgb_pixels.data();
    }

    // Generate output path
    std::string ext = opts.save_as_png ? ".png" : ".jpg";
    out_path = generate_save_path(original_path, opts.append_string, ext);

    // Encode and write
    int result = 0;
    if (opts.save_as_png) {
        result = stbi_write_png(out_path.c_str(), out_w, out_h, out_channels,
                                 final_pixels, out_w * out_channels);
    } else {
        result = stbi_write_jpg(out_path.c_str(), out_w, out_h, out_channels,
                                 final_pixels, opts.jpeg_quality);
    }

    return result != 0;
}

uint64_t SaveEngine::estimate_size(const LoadedImage& img, const SaveOptions& opts) {
    int w = (int)(img.width * opts.scale);
    int h = (int)(img.height * opts.scale);
    if (w <= 0 || h <= 0) return 0;

    if (opts.save_as_png) {
        // Rough PNG estimate: ~60% of raw, depending on content
        int channels = opts.png_transparency ? 4 : 3;
        return (uint64_t)(w * h * channels * 0.5);
    } else {
        // JPG estimate based on quality
        float ratio = opts.jpeg_quality / 100.0f * 0.3f + 0.05f;
        return (uint64_t)(w * h * 3 * ratio);
    }
}

std::string SaveEngine::preview_path(const std::string& original_path, const SaveOptions& opts) {
    std::string ext = opts.save_as_png ? ".png" : ".jpg";
    return generate_save_path(original_path, opts.append_string, ext);
}

} // namespace peek

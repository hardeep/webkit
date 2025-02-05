/*
 * Copyright (C) 2011 Research In Motion Limited. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef InstrumentedPlatformCanvas_h
#define InstrumentedPlatformCanvas_h

#include <skia/ext/platform_canvas.h>

#define DEBUG_SKIA_DRAWING 0
#if DEBUG_SKIA_DRAWING
#define WRAPCANVAS_LOG_ENTRY(...) do { \
    fprintf(stderr, "%s ", __FUNCTION__); \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n"); \
} while (0)
#else
#define WRAPCANVAS_LOG_ENTRY(...) ((void)0)
#endif

namespace WebCore {

class InstrumentedPlatformCanvas : public skia::PlatformCanvas {
public:
    InstrumentedPlatformCanvas(int width, int height, bool is_opaque, SkDeviceFactory* factory)
        : skia::PlatformCanvas(width, height, is_opaque, factory)
        , m_size(width, height)
        , m_isSolidColor(true)
        , m_solidColor(0, 0, 0, 0)
    {
    }

    virtual ~InstrumentedPlatformCanvas() { }

    bool isSolidColor() const { return m_isSolidColor; }
    Color solidColor() const { return m_solidColor; }

    // overrides from SkCanvas
    virtual int save(SaveFlags flags)
    {
        WRAPCANVAS_LOG_ENTRY("");
        return skia::PlatformCanvas::save(flags);
    }

    virtual int saveLayer(const SkRect* bounds, const SkPaint* paint, SaveFlags flags)
    {
        WRAPCANVAS_LOG_ENTRY("");
        m_isSolidColor = false;
        return skia::PlatformCanvas::saveLayer(bounds, paint, flags);
    }

    virtual void restore()
    {
        WRAPCANVAS_LOG_ENTRY("");
        skia::PlatformCanvas::restore();
    }

    virtual bool translate(SkScalar dx, SkScalar dy)
    {
        WRAPCANVAS_LOG_ENTRY("");
        return skia::PlatformCanvas::translate(dx, dy);
    }

    virtual bool scale(SkScalar sx, SkScalar sy)
    {
        WRAPCANVAS_LOG_ENTRY("");
        return skia::PlatformCanvas::scale(sx, sy);
    }

    virtual bool rotate(SkScalar degrees)
    {
        WRAPCANVAS_LOG_ENTRY("");
        return skia::PlatformCanvas::rotate(degrees);
    }

    virtual bool skew(SkScalar sx, SkScalar sy)
    {
        WRAPCANVAS_LOG_ENTRY("");
        return skia::PlatformCanvas::skew(sx, sy);
    }

    virtual bool concat(const SkMatrix& matrix)
    {
        WRAPCANVAS_LOG_ENTRY("");
        return skia::PlatformCanvas::concat(matrix);
    }

    virtual void setMatrix(const SkMatrix& matrix)
    {
        WRAPCANVAS_LOG_ENTRY("");
        skia::PlatformCanvas::setMatrix(matrix);
    }

    virtual bool clipRect(const SkRect& rect, SkRegion::Op op)
    {
        WRAPCANVAS_LOG_ENTRY("");
        return skia::PlatformCanvas::clipRect(rect, op);
    }

    virtual bool clipPath(const SkPath& path, SkRegion::Op op) 
    {
        WRAPCANVAS_LOG_ENTRY("");
        m_isSolidColor = false;
        return skia::PlatformCanvas::clipPath(path, op);
    }

    virtual bool clipRegion(const SkRegion& region, SkRegion::Op op)
    {
        WRAPCANVAS_LOG_ENTRY("");
        m_isSolidColor = false;
        return skia::PlatformCanvas::clipRegion(region, op);
    }

    virtual void clear(SkColor color)
    {
        WRAPCANVAS_LOG_ENTRY("");
        m_isSolidColor = true;
        m_solidColor = Color(color);
        skia::PlatformCanvas::clear(color);
    }

    virtual void drawPaint(const SkPaint& paint)
    {
        WRAPCANVAS_LOG_ENTRY("");
        m_isSolidColor = false;
        skia::PlatformCanvas::drawPaint(paint);
    }

    virtual void drawPoints(PointMode mode, size_t count, const SkPoint pts[],
            const SkPaint& paint)
    {
        WRAPCANVAS_LOG_ENTRY("");
        m_isSolidColor = false;
        skia::PlatformCanvas::drawPoints(mode, count, pts, paint);
    }

    virtual void drawRect(const SkRect& rect, const SkPaint& paint)
    {
        IntRect rectToDraw(rect);
        WRAPCANVAS_LOG_ENTRY("rect = (x=%d,y=%d,width=%d,height=%d)", rectToDraw.x(), rectToDraw.y(), rectToDraw.width(), rectToDraw.height());
        IntRect canvasRect(IntPoint(), m_size);
        if (m_isSolidColor && getTotalMatrix().rectStaysRect() && getTotalClip().contains(canvasRect)) {
            const SkMatrix& matrix = getTotalMatrix();
            SkRect mapped;
            matrix.mapRect(&mapped, rect);
            if (mapped.contains(canvasRect)) {
                Color color = solidColor(paint);
                m_isSolidColor = color.isValid();
                m_solidColor = color;
             } else
                 m_isSolidColor = false;
        } else
            m_isSolidColor = false;
        skia::PlatformCanvas::drawRect(rect, paint);
    }

    virtual void drawPath(const SkPath& path, const SkPaint& paint)
    {
        WRAPCANVAS_LOG_ENTRY("");
        m_isSolidColor = false;
        skia::PlatformCanvas::drawPath(path, paint);
    }

    virtual void drawBitmap(const SkBitmap& bitmap, SkScalar left,
            SkScalar top, const SkPaint* paint)
    {
        WRAPCANVAS_LOG_ENTRY("");
        m_isSolidColor = false;
        skia::PlatformCanvas::drawBitmap(bitmap, left, top, paint);
    }

    virtual void drawBitmapRect(const SkBitmap& bitmap, const SkIRect* src,
            const SkRect& dst, const SkPaint* paint)
    {
        WRAPCANVAS_LOG_ENTRY("");
        m_isSolidColor = false;
        skia::PlatformCanvas::drawBitmapRect(bitmap, src, dst, paint);
    }

    virtual void drawBitmapMatrix(const SkBitmap& bitmap,
            const SkMatrix& matrix, const SkPaint* paint)
    {
        WRAPCANVAS_LOG_ENTRY("");
        m_isSolidColor = false;
        skia::PlatformCanvas::drawBitmapMatrix(bitmap, matrix, paint);
    }

    virtual void drawSprite(const SkBitmap& bitmap, int left, int top,
            const SkPaint* paint)
    {
        WRAPCANVAS_LOG_ENTRY("");
        m_isSolidColor = false;
        skia::PlatformCanvas::drawSprite(bitmap, left, top, paint);
    }

    virtual void drawText(const void* text, size_t byteLength, SkScalar x,
            SkScalar y, const SkPaint& paint)
    {
        WRAPCANVAS_LOG_ENTRY("");
        m_isSolidColor = false;
        skia::PlatformCanvas::drawText(text, byteLength, x, y, paint);
    }

    virtual void drawPosText(const void* text, size_t byteLength,
            const SkPoint pos[], const SkPaint& paint)
    {
        WRAPCANVAS_LOG_ENTRY("");
        m_isSolidColor = false;
        skia::PlatformCanvas::drawPosText(text, byteLength, pos, paint);
    }

    virtual void drawPosTextH(const void* text, size_t byteLength,
            const SkScalar xpos[], SkScalar constY, const SkPaint& paint)
    {
        WRAPCANVAS_LOG_ENTRY("");
        m_isSolidColor = false;
        skia::PlatformCanvas::drawPosTextH(text, byteLength, xpos, constY, paint);
    }

    virtual void drawTextOnPath(const void* text, size_t byteLength,
            const SkPath& path, const SkMatrix* matrix, const SkPaint& paint)
    {
        WRAPCANVAS_LOG_ENTRY("");
        m_isSolidColor = false;
        skia::PlatformCanvas::drawTextOnPath(text, byteLength, path, matrix, paint);
    }

    virtual void drawPicture(SkPicture& picture)
    {
        WRAPCANVAS_LOG_ENTRY("");
        m_isSolidColor = false;
        skia::PlatformCanvas::drawPicture(picture);
    }

    virtual void drawVertices(VertexMode mode, int vertexCount,
            const SkPoint vertices[], const SkPoint texs[],
            const SkColor colors[], SkXfermode* xfermode,
            const uint16_t indices[], int indexCount, const SkPaint& paint)
    {
        WRAPCANVAS_LOG_ENTRY("");
        m_isSolidColor = false;
        skia::PlatformCanvas::drawVertices(mode, vertexCount, vertices, texs, colors, xfermode, indices, indexCount, paint);
    }

    virtual void drawData(const void* data, size_t size)
    {
        WRAPCANVAS_LOG_ENTRY("");
        m_isSolidColor = false;
        skia::PlatformCanvas::drawData(data, size);
    }

private:
    Color solidColor(const SkPaint& paint)
    {
        if (paint.getStyle() != SkPaint::kFill_Style)
            return Color();
        if (paint.getLooper() || paint.getShader())
            return Color();

        SkXfermode::Mode mode;
        SkXfermode::AsMode(paint.getXfermode(), &mode);
        if (mode == SkXfermode::kClear_Mode)
            return Color(0, 0, 0, 0);

        if ((mode == SkXfermode::kSrcOver_Mode && paint.getAlpha() == 255) || mode == SkXfermode::kSrc_Mode)
            return Color(paint.getColor());
        return Color();
    }

    IntSize m_size;
    bool m_isSolidColor;
    Color m_solidColor;
    SkPaint m_solidPaint;
};

} // namespace WebCore

#endif // InstrumentedPlatformCanvas_h

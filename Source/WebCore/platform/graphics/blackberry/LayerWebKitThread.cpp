/*
 * Copyright (C) 2010, 2011, 2012 Research In Motion Limited. All rights reserved.
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if USE(ACCELERATED_COMPOSITING)

#include "LayerWebKitThread.h"

#include "GraphicsContext.h"
#include "InstrumentedPlatformCanvas.h"
#include "LayerCompositingThread.h"
#include "LayerMessage.h"
#include "PlatformContextSkia.h"
#include "RenderLayerBacking.h"
#include "TransformationMatrix.h"

#include <BlackBerryPlatformGraphics.h>
#include <wtf/CurrentTime.h>

namespace WebCore {

using namespace std;

PassRefPtr<LayerWebKitThread> LayerWebKitThread::create(LayerType type, GraphicsLayerBlackBerry* owner)
{
    return adoptRef(new LayerWebKitThread(type, owner));
}

LayerWebKitThread::LayerWebKitThread(LayerType type, GraphicsLayerBlackBerry* owner)
    : LayerData(type)
    , m_owner(owner)
    , m_superlayer(0)
    , m_contents(0)
    , m_scale(1.0)
    , m_isDrawable(false)
    , m_isMask(false)
{
    m_tiler = LayerTiler::create(this);
    m_layerCompositingThread = LayerCompositingThread::create(type, m_tiler);
}

LayerWebKitThread::~LayerWebKitThread()
{
    m_layerCompositingThread->clearAnimations();

    if (m_frontBufferLock)
        pthread_mutex_destroy(m_frontBufferLock);

    m_tiler->layerWebKitThreadDestroyed();

    // Our superlayer should be holding a reference to us so there should be no
    // way for us to be destroyed while we still have a superlayer.
    ASSERT(!superlayer());

    // Remove the superlayer reference from all sublayers.
    removeAllSublayers();
}

SkBitmap LayerWebKitThread::paintContents(const IntRect& contentsRect, double scale, bool* isSolidColor, Color* color)
{
    // Don't try to allocate image data bigger than this. This should be big
    // enough to accomodate a huge iScroll use case.
    // FIXME: This is a hack to work around a crash bug on maps.bing.com where
    // a (visually empty) layer becomes too big.
    static const int maximumBitmapSizeInBytes = 40 * 1024 * 1024;
    static const int bytesPerPixel = 4;

    if (isSolidColor)
        *isSolidColor = false;

    if (contentsRect.width() * contentsRect.height() * bytesPerPixel > maximumBitmapSizeInBytes)
        return SkBitmap();

    SkBitmap bitmap;

    // Keep the canvas alive until we're done extracting its pixels
    OwnPtr<InstrumentedPlatformCanvas> canvas;

    if (drawsContent()) { // Layer contents must be drawn into a canvas.
        IntRect untransformedContentsRect = contentsRect;

        canvas = adoptPtr(new InstrumentedPlatformCanvas(contentsRect.width(), contentsRect.height(), false, 0));
        PlatformContextSkia skiaContext(canvas.get());

        GraphicsContext graphicsContext(&skiaContext);
        graphicsContext.translate(-contentsRect.x(), -contentsRect.y());

        if (scale != 1.0) {
            TransformationMatrix matrix;
            matrix.scale(1.0 / scale);
            untransformedContentsRect = matrix.mapRect(contentsRect);

            // We extract from the contentsRect but draw a slightly larger region than
            // we were told to, in order to avoid pixels being rendered only partially.
            const int atLeastOneDevicePixel = static_cast<int>(ceilf(1.0 / scale));
            untransformedContentsRect.inflate(atLeastOneDevicePixel);

            graphicsContext.scale(FloatSize(scale, scale));
        }

        // RenderLayerBacking doesn't always clip, so we need to do this by ourselves.
        graphicsContext.clip(untransformedContentsRect);
        m_owner->paintGraphicsLayerContents(graphicsContext, untransformedContentsRect);

        bitmap = canvas->getDevice()->accessBitmap(false);
        if (isSolidColor) {
            *isSolidColor = canvas->isSolidColor();
            if (color)
                *color = canvas->solidColor();
        }
    }

    ASSERT(!bitmap.isNull());

    // FIXME: do we need to support more image configurations?
    ASSERT(bitmap.config() == SkBitmap::kARGB_8888_Config);
    if (bitmap.config() != SkBitmap::kARGB_8888_Config)
        return SkBitmap();

    return bitmap;
}

bool LayerWebKitThread::contentsVisible(const IntRect& contentsRect) const
{
    if (!m_owner)
        return false;

    return m_owner->contentsVisible(contentsRect);
}

void LayerWebKitThread::createFrontBufferLock()
{
    pthread_mutexattr_t mutexAttributes;
    pthread_mutexattr_init(&mutexAttributes);
    m_frontBufferLock = new pthread_mutex_t;
    pthread_mutex_init(m_frontBufferLock, &mutexAttributes);
}

void LayerWebKitThread::updateTextureContentsIfNeeded()
{
    m_tiler->updateTextureContentsIfNeeded(m_isMask ? 1.0 : contentsScale());
}

void LayerWebKitThread::setContents(Image* contents)
{
    // Check if the image has changed.
    if (m_contents == contents)
        return;
    m_contents = contents;
    setNeedsTexture(m_isDrawable && (this->contents() || drawsContent() || pluginView()));

    if (m_contents)
        setNeedsDisplay();
    else
        setNeedsCommit();
}

void LayerWebKitThread::setDrawable(bool isDrawable)
{
    if (m_isDrawable == isDrawable)
        return;

    m_isDrawable = isDrawable;

    setNeedsTexture(m_isDrawable && (drawsContent() || contents() || pluginView() || mediaPlayer() || canvas()));
    setNeedsCommit();
}

void LayerWebKitThread::setNeedsCommit()
{
    // Call notifySyncRequired(), which in this implementation plumbs through to
    // call scheduleRootLayerCommit() on the WebView, which will cause us to commit
    // changes done on the WebKit thread for display on the Compositing thread.
    if (m_owner)
        m_owner->notifySyncRequired();
}

void LayerWebKitThread::notifyAnimationStarted(double time)
{
    if (m_owner)
        m_owner->notifyAnimationStarted(time);
}

void LayerWebKitThread::commitOnWebKitThread(double scale)
{
    // Updating texture contents require the latest visibility info.
    updateTextureContents(scale);

    // Make sure all animations are started at the same time
    // to avoid showing animations out-of-sync.
    // Do this after updating texture contents, because that can be a slow
    // operation.
    startAnimations(currentTime());
}

void LayerWebKitThread::startAnimations(double time)
{
    for (size_t i = 0; i < m_runningAnimations.size(); ++i) {
        if (!m_runningAnimations[i]->startTime()) {
            m_runningAnimations[i]->setStartTime(time);
            notifyAnimationStarted(time);
        }
    }

    size_t listSize = m_sublayers.size();
    for (size_t i = 0; i < listSize; i++)
        m_sublayers[i]->startAnimations(time);
}

void LayerWebKitThread::updateTextureContents(double scale)
{
    if (m_scale != scale) {
        m_scale = scale;

        // Only web content can redraw at the new scale.
        // Canvas, images, video etc can't.
        if (drawsContent())
            setNeedsDisplay();
    }

    updateTextureContentsIfNeeded();

    if (includeVisibility()) {
        // The RenderLayerBacking cast looks unsafe given that there are two classes
        // derived from GraphicsLayerClient but this code is only reachable for
        // things that produce RenderLayerBacking derivatives; i.e., plugins and media.
        RenderLayer* renderLayer(static_cast<RenderLayerBacking*>(m_owner->client())->owningLayer());
        bool isVisible(renderLayer->hasVisibleContent() || renderLayer->hasVisibleDescendant());
        if (m_isVisible != isVisible) {
            m_isVisible = isVisible;
            setNeedsCommit();
        }
    }

    size_t listSize = m_sublayers.size();
    for (size_t i = 0; i < listSize; i++)
        m_sublayers[i]->updateTextureContents(scale);

    if (maskLayer())
        maskLayer()->updateTextureContents(scale);

    if (replicaLayer())
        replicaLayer()->updateTextureContents(scale);
}

void LayerWebKitThread::commitOnCompositingThread()
{
    FloatPoint oldPosition = m_position;
    m_position += m_absoluteOffset;
    // Copy the base variables from this object into m_layerCompositingThread
    replicate(m_layerCompositingThread.get());
    m_position = oldPosition;
    updateLayerHierarchy();
    m_tiler->commitPendingTextureUploads();

    size_t listSize = m_sublayers.size();
    for (size_t i = 0; i < listSize; i++)
        m_sublayers[i]->commitOnCompositingThread();

    if (maskLayer()) {
        maskLayer()->commitOnCompositingThread();
        layerCompositingThread()->setMaskLayer(maskLayer()->layerCompositingThread());
    } else
        layerCompositingThread()->setMaskLayer(0);

    if (replicaLayer()) {
        replicaLayer()->commitOnCompositingThread();
        layerCompositingThread()->setReplicaLayer(replicaLayer()->layerCompositingThread());
    } else
        layerCompositingThread()->setReplicaLayer(0);
}

void LayerWebKitThread::addSublayer(PassRefPtr<LayerWebKitThread> sublayer)
{
    insertSublayer(sublayer, numSublayers());
}

void LayerWebKitThread::insertSublayer(PassRefPtr<LayerWebKitThread> sublayer, size_t index)
{
    index = min(index, m_sublayers.size());
    sublayer->removeFromSuperlayer();
    sublayer->setSuperlayer(this);
    m_sublayers.insert(index, sublayer);

    setNeedsCommit();
}

void LayerWebKitThread::removeFromSuperlayer()
{
    if (m_superlayer)
        m_superlayer->removeSublayer(this);
}

void LayerWebKitThread::removeSublayer(LayerWebKitThread* sublayer)
{
    int foundIndex = indexOfSublayer(sublayer);
    if (foundIndex == -1)
        return;

    sublayer->setSuperlayer(0);
    m_sublayers.remove(foundIndex);

    setNeedsCommit();
}

void LayerWebKitThread::replaceSublayer(LayerWebKitThread* reference, PassRefPtr<LayerWebKitThread> newLayer)
{
    ASSERT_ARG(reference, reference);
    ASSERT_ARG(reference, reference->superlayer() == this);

    if (reference == newLayer)
        return;

    int referenceIndex = indexOfSublayer(reference);
    if (referenceIndex == -1) {
        ASSERT_NOT_REACHED();
        return;
    }

    reference->removeFromSuperlayer();

    if (newLayer) {
        newLayer->removeFromSuperlayer();
        insertSublayer(newLayer, referenceIndex);
    }
}

int LayerWebKitThread::indexOfSublayer(const LayerWebKitThread* reference)
{
    for (size_t i = 0; i < m_sublayers.size(); i++) {
        if (m_sublayers[i] == reference)
            return i;
    }
    return -1;
}

void LayerWebKitThread::setBounds(const IntSize& size)
{
    if (m_bounds == size)
        return;

    bool firstResize = !m_bounds.width() && !m_bounds.height() && size.width() && size.height();

    m_bounds = size;

    boundsChanged();

    if (firstResize)
        setNeedsDisplay();
    else
        setNeedsCommit();
}

void LayerWebKitThread::setFrame(const FloatRect& rect)
{
    if (rect == m_frame)
      return;

    m_frame = rect;
    setNeedsDisplay();
}

const LayerWebKitThread* LayerWebKitThread::rootLayer() const
{
    const LayerWebKitThread* layer = this;
    LayerWebKitThread* superlayer = layer->superlayer();

    while (superlayer) {
        layer = superlayer;
        superlayer = superlayer->superlayer();
    }
    return layer;
}

void LayerWebKitThread::removeAllSublayers()
{
    while (m_sublayers.size()) {
        RefPtr<LayerWebKitThread> layer = m_sublayers[0].get();
        ASSERT(layer->superlayer());
        layer->removeFromSuperlayer();
    }
    setNeedsCommit();
}

void LayerWebKitThread::setSublayers(const Vector<RefPtr<LayerWebKitThread> >& sublayers)
{
    if (sublayers == m_sublayers)
        return;

    removeAllSublayers();
    size_t listSize = sublayers.size();
    for (size_t i = 0; i < listSize; i++)
        addSublayer(sublayers[i]);
}

void LayerWebKitThread::setNeedsDisplayInRect(const FloatRect& dirtyRect)
{
    m_tiler->setNeedsDisplay(dirtyRect);
    setNeedsCommit(); // FIXME: Replace this with a more targeted message for dirty rect handling with plugin content?
}

void LayerWebKitThread::setNeedsDisplay()
{
    m_tiler->setNeedsDisplay();
    setNeedsCommit(); // FIXME: Replace this with a more targeted message for dirty rect handling with plugin content?
}

void LayerWebKitThread::updateLayerHierarchy()
{
    m_layerCompositingThread->setSuperlayer(superlayer() ? superlayer()->m_layerCompositingThread.get() : 0);

    Vector<RefPtr<LayerCompositingThread> > sublayers;
    size_t listSize = m_sublayers.size();
    for (size_t i = 0; i < listSize; i++)
        sublayers.append(m_sublayers[i]->m_layerCompositingThread.get());
    m_layerCompositingThread->setSublayers(sublayers);
}

void LayerWebKitThread::setIsMask(bool isMask)
{
    m_isMask = isMask;
    if (isMask)
        m_tiler->disableTiling(true);
}

}

#endif // USE(ACCELERATED_COMPOSITING)

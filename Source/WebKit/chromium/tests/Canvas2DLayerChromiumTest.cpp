/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include "Canvas2DLayerChromium.h"

#include "CCSchedulerTestCommon.h"
#include "FakeWebGraphicsContext3D.h"
#include "GraphicsContext3DPrivate.h"
#include "Region.h"
#include "TextureManager.h"
#include "cc/CCCanvasLayerImpl.h"
#include "cc/CCSingleThreadProxy.h"
#include "cc/CCTextureUpdater.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <wtf/RefPtr.h>

using namespace WebCore;
using namespace WebKit;
using namespace WebKitTests;
using testing::InSequence;
using testing::Return;
using testing::Test;

namespace {

class MockCanvasContext : public FakeWebGraphicsContext3D {
public:
    MOCK_METHOD0(createFramebuffer, WebGLId());
    MOCK_METHOD0(createTexture, WebGLId());

    MOCK_METHOD2(bindFramebuffer, void(WGC3Denum, WebGLId));
    MOCK_METHOD5(framebufferTexture2D, void(WGC3Denum, WGC3Denum, WGC3Denum, WebGLId, WGC3Dint));

    MOCK_METHOD2(bindTexture, void(WGC3Denum, WebGLId));
    MOCK_METHOD8(copyTexSubImage2D, void(WGC3Denum, WGC3Dint, WGC3Dint, WGC3Dint, WGC3Dint, WGC3Dint, WGC3Dsizei, WGC3Dsizei));

    MOCK_METHOD1(deleteFramebuffer, void(WebGLId));
    MOCK_METHOD1(deleteTexture, void(WebGLId));
};

class MockTextureAllocator : public TextureAllocator {
public:
    MOCK_METHOD2(createTexture, unsigned(const IntSize&, GC3Denum));
    MOCK_METHOD3(deleteTexture, void(unsigned, const IntSize&, GC3Denum));
};

} // namespace

namespace WebCore {

class Canvas2DLayerChromiumTest : public Test {
protected:
    void fullLifecycleTest(bool threaded)
    {
        GraphicsContext3D::Attributes attrs;

        RefPtr<GraphicsContext3D> mainContext = GraphicsContext3DPrivate::createGraphicsContextFromWebContext(adoptPtr(new MockCanvasContext()), GraphicsContext3D::RenderDirectlyToHostWindow);
        RefPtr<GraphicsContext3D> implContext = GraphicsContext3DPrivate::createGraphicsContextFromWebContext(adoptPtr(new MockCanvasContext()), GraphicsContext3D::RenderDirectlyToHostWindow);

        MockCanvasContext& mainMock = *static_cast<MockCanvasContext*>(GraphicsContext3DPrivate::extractWebGraphicsContext3D(mainContext.get()));
        MockCanvasContext& implMock = *static_cast<MockCanvasContext*>(GraphicsContext3DPrivate::extractWebGraphicsContext3D(implContext.get()));

        MockTextureAllocator allocatorMock;
        CCTextureUpdater updater(&allocatorMock);

        const IntSize size(300, 150);
        const size_t maxTextureSize = size.width() * size.height() * 4;
        OwnPtr<TextureManager> textureManager = TextureManager::create(maxTextureSize, maxTextureSize, maxTextureSize);

        if (threaded)
            CCProxy::setImplThread(new FakeCCThread);

        const WebGLId backTextureId = 1;
        const WebGLId frontTextureId = 2;
        const WebGLId fboId = 3;
        {
            InSequence sequence;

            // Note that the canvas backing texture is doublebuffered only when using the threaded
            // compositor.
            if (threaded) {
                // Setup Canvas2DLayerChromium (on the main thread).
                EXPECT_CALL(mainMock, createFramebuffer())
                    .WillOnce(Return(fboId));

                // Create texture and do the copy (on the impl thread).
                EXPECT_CALL(allocatorMock, createTexture(size, GraphicsContext3D::RGBA))
                    .WillOnce(Return(frontTextureId));
                EXPECT_CALL(implMock, bindTexture(GraphicsContext3D::TEXTURE_2D, frontTextureId));
                EXPECT_CALL(implMock, bindFramebuffer(GraphicsContext3D::FRAMEBUFFER, fboId));
                EXPECT_CALL(implMock, framebufferTexture2D(GraphicsContext3D::FRAMEBUFFER, GraphicsContext3D::COLOR_ATTACHMENT0, GraphicsContext3D::TEXTURE_2D, backTextureId, 0));
                EXPECT_CALL(implMock, copyTexSubImage2D(GraphicsContext3D::TEXTURE_2D, 0, 0, 0, 0, 0, 300, 150));
                EXPECT_CALL(implMock, bindFramebuffer(GraphicsContext3D::FRAMEBUFFER, 0));

                // Teardown Canvas2DLayerChromium.
                EXPECT_CALL(mainMock, deleteFramebuffer(fboId));

                // Teardown TextureManager.
                EXPECT_CALL(allocatorMock, deleteTexture(frontTextureId, size, GraphicsContext3D::RGBA));
            }
        }

        RefPtr<Canvas2DLayerChromium> canvas = Canvas2DLayerChromium::create(mainContext.get(), size);
        canvas->setIsDrawable(true);
        canvas->setTextureManager(textureManager.get());
        canvas->setBounds(IntSize(600, 300));
        canvas->setTextureId(backTextureId);

        canvas->setNeedsDisplay();
        EXPECT_TRUE(canvas->needsDisplay());
        Region occludedScreenSpace;
        canvas->paintContentsIfDirty(occludedScreenSpace);
        EXPECT_FALSE(canvas->needsDisplay());
        {
            DebugScopedSetImplThread scopedImplThread;

            OwnPtr<CCLayerImpl> layerImpl = canvas->createCCLayerImpl();
            EXPECT_EQ(0u, static_cast<CCCanvasLayerImpl*>(layerImpl.get())->textureId());

            canvas->updateCompositorResources(implContext.get(), updater);
            canvas->pushPropertiesTo(layerImpl.get());

            if (threaded)
                EXPECT_EQ(frontTextureId, static_cast<CCCanvasLayerImpl*>(layerImpl.get())->textureId());
            else
                EXPECT_EQ(backTextureId, static_cast<CCCanvasLayerImpl*>(layerImpl.get())->textureId());
        }
        canvas.clear();
        textureManager->reduceMemoryToLimit(0);
        textureManager->deleteEvictedTextures(&allocatorMock);
    }
};

TEST_F(Canvas2DLayerChromiumTest, testFullLifecycleSingleThread)
{
    fullLifecycleTest(false);
}

TEST_F(Canvas2DLayerChromiumTest, testFullLifecycleThreaded)
{
    fullLifecycleTest(true);
}

} // namespace webcore

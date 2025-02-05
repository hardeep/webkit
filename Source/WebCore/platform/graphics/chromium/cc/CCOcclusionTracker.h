/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CCOcclusionTracker_h
#define CCOcclusionTracker_h

#include "FloatQuad.h"
#include "Region.h"
#include "TransformationMatrix.h"

namespace WebCore {
class CCLayerImpl;
class CCRenderSurface;
class LayerChromium;
class RenderSurfaceChromium;

template<typename RenderSurfaceType>
class CCOcclusionTrackerDamageClientBase {
public:
    virtual FloatRect damageRect(const RenderSurfaceType*) const = 0;
};

typedef CCOcclusionTrackerDamageClientBase<RenderSurfaceChromium> CCOcclusionTrackerDamageClient;
typedef CCOcclusionTrackerDamageClientBase<CCRenderSurface> CCOcclusionTrackerDamageClientImpl;

// This class is used to track occlusion of layers while traversing them in a front-to-back order. As each layer is visited, one of the
// methods in this class is called to notify it about the current target surface.
// Then, occlusion in the content space of the current layer may be queried, via methods such as occluded() and unoccludedContentRect().
// If the current layer owns a RenderSurface, then occlusion on that RenderSurface may also be queried via surfaceOccluded() and surfaceUnoccludedContentRect().
// Finally, once finished with the layer, occlusion behind the layer should be marked by calling markOccludedBehindLayer().
template<typename LayerType, typename RenderSurfaceType>
class CCOcclusionTrackerBase {
    WTF_MAKE_NONCOPYABLE(CCOcclusionTrackerBase);
    typedef CCOcclusionTrackerDamageClientBase<RenderSurfaceType> DamageClientType;
public:
    CCOcclusionTrackerBase(IntRect scissorRectInScreenSpace);
    CCOcclusionTrackerBase(IntRect scissorRectInScreenSpace, const DamageClientType*);

    // Called when visiting a layer representing itself. If the target was not already current, then this indicates we have entered a new surface subtree.
    void enterTargetRenderSurface(const RenderSurfaceType* newTarget);

    // Called when visiting a layer representing a target surface. This indicates we have visited all the layers within the surface, and we may
    // perform any surface-wide operations.
    void finishedTargetRenderSurface(const LayerType*, const RenderSurfaceType* finishedTarget);

    // Called when visiting a layer representing a contributing surface. This  indicates that we are leaving our current surface, and
    // entering the new one. We then perform any operations required for merging results from the child subtree into its parent.
    void leaveToTargetRenderSurface(const RenderSurfaceType* newTarget);

    // Add the layer's occlusion to the tracked state.
    void markOccludedBehindLayer(const LayerType*);

    // Returns true if the given rect in content space for the layer is fully occluded in either screen space or the layer's target surface.
    bool occluded(const LayerType*, const IntRect& contentRect) const;
    // Gives an unoccluded sub-rect of |contentRect| in the content space of the layer. Used when considering occlusion for a layer that paints/draws something.
    IntRect unoccludedContentRect(const LayerType*, const IntRect& contentRect) const;

    // FIXME: Remove these in future, they are to make CLs for transitioning to this easier.
    const Region& currentOcclusionInScreenSpace() const;
    const Region& currentOcclusionInTargetSurface() const;

protected:
    struct StackObject {
        const RenderSurfaceType* surface;
        Region occlusionInScreen;
        Region occlusionInTarget;
    };

    // The stack holds occluded regions for subtrees in the RenderSurface-Layer tree, so that when we leave a subtree we may
    // apply a mask to it, but not to the parts outside the subtree.
    // - The first time we see a new subtree under a target, we add that target to the top of the stack. This can happen as a layer representing itself, or as a target surface.
    // - When we visit a target surface, we apply its mask to its subtree, which is at the top of the stack.
    // - When we visit a layer representing itself, we add its occlusion to the current subtree, which is at the top of the stack.
    // - When we visit a layer representing a contributing surface, the current target will never be the top of the stack since we just came from the contributing surface.
    // We merge the occlusion at the top of the stack with the new current subtree. This new target is pushed onto the stack if not already there.
    Vector<StackObject, 1> m_stack;

    // Allow tests to override this.
    virtual IntRect layerScissorRectInTargetSurface(const LayerType*) const;

private:
    IntRect m_scissorRectInScreenSpace;
    const DamageClientType* m_surfaceDamageClient;
};

typedef CCOcclusionTrackerBase<LayerChromium, RenderSurfaceChromium> CCOcclusionTracker;
typedef CCOcclusionTrackerBase<CCLayerImpl, CCRenderSurface> CCOcclusionTrackerImpl;

}
#endif // CCOcclusionTracker_h

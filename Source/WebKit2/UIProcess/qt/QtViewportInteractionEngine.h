/*
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2011 Benjamin Poulain <benjamin@webkit.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef QtViewportInteractionEngine_h
#define QtViewportInteractionEngine_h

#include "OwnPtr.h"
#include "qwebkitglobal.h"
#include <QTouchEvent>
#include <QtCore/QObject>
#include <QtCore/QRectF>
#include <QtCore/QVariant>
#include <QtCore/QVariantAnimation>

QT_BEGIN_NAMESPACE
class QPointF;
class QQuickItem;
class QtFlickProvider;
class QQuickWebPage;
class QQuickWebView;
class QWheelEvent;
QT_END_NAMESPACE

namespace WebKit {

class ViewportUpdateDeferrer;

class QtViewportInteractionEngine : public QObject {
    Q_OBJECT

public:
    QtViewportInteractionEngine(QQuickWebView*, QQuickWebPage*, QtFlickProvider*);
    ~QtViewportInteractionEngine();

    struct Constraints {
        Constraints()
            : initialScale(1.0)
            , minimumScale(0.25)
            , maximumScale(1.8)
            , devicePixelRatio(1.0)
            , isUserScalable(true)
            , layoutSize(QSize())
        { }

        qreal initialScale;
        qreal minimumScale;
        qreal maximumScale;
        qreal devicePixelRatio;
        bool isUserScalable;
        QSize layoutSize;
    };

    void reset();
    void applyConstraints(const Constraints&);

    void setItemRectVisible(const QRectF&);
    bool animateItemRectVisible(const QRectF&);

    void wheelEvent(QWheelEvent*);
    void pagePositionRequest(const QPoint& pos);

    bool scrollAnimationActive() const;

    bool panGestureActive() const;
    void panGestureStarted(const QTouchEvent*);
    void panGestureRequestUpdate(const QTouchEvent*);
    void panGestureCancelled();
    void panGestureEnded(const QTouchEvent*);

    bool scaleAnimationActive() const;
    void interruptScaleAnimation();

    bool pinchGestureActive() const;
    void pinchGestureStarted(const QPointF& pinchCenterInViewportCoordinates);
    void pinchGestureRequestUpdate(const QPointF& pinchCenterInViewportCoordinates, qreal totalScaleFactor);
    void pinchGestureEnded();

    void zoomToAreaGestureEnded(const QPointF& touchPoint, const QRectF& targetArea);
    void focusEditableArea(const QRectF& caretArea, const QRectF& targetArea);

    const Constraints& constraints() const { return m_constraints; }
    qreal currentCSSScale();

Q_SIGNALS:
    void contentSuspendRequested();
    void contentResumeRequested();

    void contentWasMoved(const QPointF& trajectoryVector);
    void contentWasScaled();

    void viewportTrajectoryVectorChanged(const QPointF&);
    void visibleContentRectAndScaleChanged();

private Q_SLOTS:
    // Respond to changes of content that are not driven by us, like the page resizing itself.
    void itemSizeChanged();

    void flickableMovingPositionUpdate();

    void scaleAnimationStateChanged(QAbstractAnimation::State, QAbstractAnimation::State);
    void scaleAnimationValueChanged(QVariant value) { setItemRectVisible(value.toRectF()); }

    void flickableMoveStarted(); // Called when panning starts.
    void flickableMoveEnded(); //   Called when panning (+ kinetic animation) ends.

private:
    friend class ViewportUpdateDeferrer;

    qreal cssScaleFromItem(qreal);
    qreal itemScaleFromCSS(qreal);
    qreal itemCoordFromCSS(qreal);
    QRectF itemRectFromCSS(const QRectF&);

    qreal innerBoundedCSSScale(qreal);
    qreal outerBoundedCSSScale(qreal);

    QRectF computePosRangeForItemAtScale(qreal itemScale) const;
    bool ensureContentWithinViewportBoundary(bool immediate = false);

    void scaleContent(const QPointF& centerInCSSCoordinates, qreal cssScale);

    QQuickWebView* const m_viewport;
    QQuickWebPage* const m_content;

    QtFlickProvider* const m_flickProvider;

    Constraints m_constraints;

    int m_suspendCount;
    bool m_hasSuspendedContent;
    OwnPtr<ViewportUpdateDeferrer> m_scaleUpdateDeferrer;
    OwnPtr<ViewportUpdateDeferrer> m_scrollUpdateDeferrer;

    bool m_hadUserInteraction;

    class ScaleAnimation : public QVariantAnimation {
    public:
        ScaleAnimation(QObject* parent = 0)
            : QVariantAnimation(parent)
        { }

        virtual void updateCurrentValue(const QVariant&) { }
    };

    ScaleAnimation* m_scaleAnimation;
    QPointF m_lastPinchCenterInViewportCoordinates;
    QPointF m_lastScrollPosition;
    qreal m_pinchStartScale;
};

inline bool operator==(const QtViewportInteractionEngine::Constraints& a, const QtViewportInteractionEngine::Constraints& b)
{
    return a.initialScale == b.initialScale
            && a.minimumScale == b.minimumScale
            && a.maximumScale == b.maximumScale
            && a.isUserScalable == b.isUserScalable;
}

}

#endif // QtViewportInteractionEngine_h

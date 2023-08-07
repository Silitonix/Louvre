#include <private/LScenePrivate.h>
#include <private/LViewPrivate.h>
#include <private/LSceneViewPrivate.h>
#include <private/LSurfacePrivate.h>
#include <LSurfaceView.h>
#include <LOutput.h>
#include <LCursor.h>
#include <LSeat.h>
#include <LDNDManager.h>
#include <LDNDIconRole.h>
#include <LPointer.h>
#include <LCompositor.h>

#include <unistd.h>

LScene::LScene()
{
    m_imp = new LScenePrivate();
    imp()->view = new LSceneView();
    imp()->view->setPos(0);

    LView *baseView = imp()->view;
    baseView->imp()->scene = this;
}

LScene::~LScene()
{
    delete imp()->view;
    delete m_imp;
}

void LScene::handleInitializeGL(LOutput *output)
{
    imp()->view->imp()->fb = output->framebuffer();
}

void LScene::handlePaintGL(LOutput *output)
{
    imp()->view->imp()->fb = output->framebuffer();
    imp()->view->render();
}

void LScene::handleMoveGL(LOutput *output)
{
    imp()->view->imp()->fb = output->framebuffer();
    imp()->view->damageAll(output);
}

void LScene::handleResizeGL(LOutput *output)
{
    imp()->view->damageAll(output);
}

void LScene::handleUninitializeGL(LOutput *output)
{
    L_UNUSED(output);

    auto it = imp()->view->imp()->threadsMap.find(std::this_thread::get_id());

    if (it != imp()->view->imp()->threadsMap.end())
        imp()->view->imp()->threadsMap.erase(it);
}

LView *LScene::handlePointerMoveEvent(Float32 dx, Float32 dy, LPoint *outLocalPos)
{
    return handlePointerPosChangeEvent(cursor()->pos().x() + dx, cursor()->pos().y() + dy, outLocalPos);
}

LView *LScene::handlePointerPosChangeEvent(Float32 x, Float32 y, LPoint *outLocalPos)
{
    LSurface *surface = nullptr;
    LView *view = nullptr;
    LPoint localPos;

    cursor()->setPos(LPointF(x, y));

    imp()->pointerIsBlocked = false;
    imp()->handlePointerMove(mainView(), cursor()->pos(), &view);

    if (view)
    {
        localPos = imp()->viewLocalPos(view, cursor()->pos());

        if (outLocalPos)
            *outLocalPos = localPos;

        if (view->type() == LView::Surface)
        {
            LSurfaceView *surfaceView = (LSurfaceView*)view;
            surface = surfaceView->surface();
        }
    }

    if (!handleWaylandPointerEventsEnabled())
        return view;

    // Repaint cursor outputs if hardware composition is not supported
    for (LOutput *output : cursor()->intersectedOutputs())
    {
        if (!cursor()->hasHardwareSupport(output))
            output->repaint();
    }

    if (seat()->pointer()->lastCursorRequest())
    {
        for (LOutput *output : compositor()->outputs())
        {
            if (output == cursor()->output())
                seat()->pointer()->lastCursorRequest()->surface()->sendOutputEnterEvent(output);
            else
                seat()->pointer()->lastCursorRequest()->surface()->sendOutputLeaveEvent(output);
        }
    }

    // Update the drag & drop icon (if there was one)
    if (seat()->dndManager()->icon())
    {
        seat()->dndManager()->icon()->surface()->setPos(cursor()->pos());
        seat()->dndManager()->icon()->surface()->repaintOutputs();
    }

    // Update the Toplevel size (if there was one being resized)
    if (seat()->pointer()->resizingToplevel())
    {
        seat()->pointer()->updateResizingToplevelSize();
        return view;
    }

    // Update the Toplevel pos (if there was one being moved interactively)
    if (seat()->pointer()->movingToplevel())
    {
        seat()->pointer()->updateMovingToplevelPos();

        seat()->pointer()->movingToplevel()->surface()->repaintOutputs();

        if (seat()->pointer()->movingToplevel()->maximized())
            seat()->pointer()->movingToplevel()->configure(seat()->pointer()->movingToplevel()->states() &~ LToplevelRole::Maximized);

        return view;
    }

    // DO NOT GET CONFUSED! If we are in a drag & drop session, we call setDragginSurface(NULL) in case there is a surface being dragged.
    if (seat()->dndManager()->dragging())
        seat()->pointer()->setDragginSurface(nullptr);

    // If there was a surface holding the left pointer button
    if (seat()->pointer()->draggingSurface())
    {
        if (seat()->pointer()->draggingSurface()->imp()->lastPointerEventView)
            seat()->pointer()->sendMoveEvent(imp()->viewLocalPos(seat()->pointer()->draggingSurface()->imp()->lastPointerEventView, cursor()->pos()));
        else
            seat()->pointer()->sendMoveEvent();

        return view;
    }

    if (!surface)
    {
        seat()->pointer()->setFocus(nullptr);
    }
    else
    {
        surface->imp()->lastPointerEventView = (LSurfaceView*)view;

        if (seat()->pointer()->focusSurface() == surface)
            seat()->pointer()->sendMoveEvent(localPos);
        else
            seat()->pointer()->setFocus(surface, localPos);
    }

    return view;
}

void LScene::handlePointerButtonEvent(LPointer::Button button, LPointer::ButtonState state)
{
    imp()->handlePointerButton(mainView(), button, state);

    if (!handleWaylandPointerEventsEnabled())
        return;

    if (button == LPointer::Left && state == LPointer::Released)
        seat()->dndManager()->drop();

    if (!seat()->pointer()->focusSurface())
    {
        LSurface *surface = nullptr;
        LView *view = viewAt(cursor()->pos());

        if (view && view->type() == LView::Surface)
        {
            LSurfaceView *surfaceView = (LSurfaceView*)view;
            surface = surfaceView->surface();
        }

        if (surface)
        {
            if (seat()->keyboard()->grabbingSurface() && seat()->keyboard()->grabbingSurface()->client() != surface->client())
            {
                seat()->keyboard()->setGrabbingSurface(nullptr, nullptr);
                seat()->pointer()->dismissPopups();
            }

            if (!seat()->keyboard()->focusSurface() || !surface->isSubchildOf(seat()->keyboard()->focusSurface()))
                seat()->keyboard()->setFocus(surface);

            seat()->pointer()->setFocus(surface, imp()->viewLocalPos(view, cursor()->pos()));
            seat()->pointer()->sendButtonEvent(button, state);
        }
        // If no surface under the cursor
        else
        {
            seat()->keyboard()->setGrabbingSurface(nullptr, nullptr);
            seat()->keyboard()->setFocus(nullptr);
            seat()->pointer()->dismissPopups();
        }

        return;
    }

    seat()->pointer()->sendButtonEvent(button, state);

    if (button != LPointer::Button::Left)
        return;

    // Left button pressed
    if (state == LPointer::ButtonState::Pressed)
    {
        /* We save the pointer focus surface in order to continue sending events to it even when the cursor
         * is outside of it (while the left button is being held down)*/
        seat()->pointer()->setDragginSurface(seat()->pointer()->focusSurface());

        if (seat()->keyboard()->grabbingSurface() && seat()->keyboard()->grabbingSurface()->client() != seat()->pointer()->focusSurface()->client())
        {
            seat()->keyboard()->setGrabbingSurface(nullptr, nullptr);
            seat()->pointer()->dismissPopups();
        }

        if (!seat()->pointer()->focusSurface()->popup())
            seat()->pointer()->dismissPopups();

        if (!seat()->keyboard()->focusSurface() || !seat()->pointer()->focusSurface()->isSubchildOf(seat()->keyboard()->focusSurface()))
            seat()->keyboard()->setFocus(seat()->pointer()->focusSurface());

        if (seat()->pointer()->focusSurface()->toplevel() && !seat()->pointer()->focusSurface()->toplevel()->activated())
            seat()->pointer()->focusSurface()->toplevel()->configure(seat()->pointer()->focusSurface()->toplevel()->states() | LToplevelRole::Activated);

        // Raise surface
        if (seat()->pointer()->focusSurface() == compositor()->surfaces().back())
            return;

        if (seat()->pointer()->focusSurface()->parent())
            compositor()->raiseSurface(seat()->pointer()->focusSurface()->topmostParent());
        else
            compositor()->raiseSurface(seat()->pointer()->focusSurface());
    }
    // Left button released
    else
    {
        seat()->pointer()->stopResizingToplevel();
        seat()->pointer()->stopMovingToplevel();

        // We stop sending events to the surface on which the left button was being held down
        seat()->pointer()->setDragginSurface(nullptr);

        if (seat()->pointer()->focusSurface()->imp()->lastPointerEventView)
        {
            if (!imp()->pointerIsOverView(seat()->pointer()->focusSurface()->imp()->lastPointerEventView, cursor()->pos()))
            {
                seat()->keyboard()->setGrabbingSurface(nullptr, nullptr);
                seat()->pointer()->setFocus(nullptr);
            }
        }
        else
        {
            if (!seat()->pointer()->focusSurface()->inputRegion().containsPoint(cursor()->pos() - seat()->pointer()->focusSurface()->pos()))
            {
                seat()->keyboard()->setGrabbingSurface(nullptr, nullptr);
                seat()->pointer()->setFocus(nullptr);
            }
        }
    }
}

void LScene::handlePointerAxisEvent(Float64 axisX, Float64 axisY, Int32 discreteX, Int32 discreteY, UInt32 source)
{
    imp()->handlePointerAxisEvent(mainView(), axisX, axisY, discreteX, discreteY, source);

    if (!handleWaylandPointerEventsEnabled())
        return;

    seat()->pointer()->sendAxisEvent(axisX, axisY, discreteX, discreteY, source);
}

bool LScene::handleWaylandPointerEventsEnabled() const
{
    return imp()->handleWaylandPointerEvents;
}

void LScene::enableHandleWaylandPointerEvents(bool enabled)
{
    imp()->handleWaylandPointerEvents = enabled;
}

void LScene::handleKeyModifiersEvent(UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group)
{
    imp()->handleKeyModifiersEvent(mainView(), depressed, latched, locked, group);

    if (handleWaylandKeyboardEventsEnabled())
        seat()->keyboard()->sendModifiersEvent(depressed, latched, locked, group);
}

void LScene::handleKeyEvent(UInt32 keyCode, UInt32 keyState)
{
    imp()->handleKeyEvent(mainView(), keyCode, keyState);

    if (handleWaylandKeyboardEventsEnabled())
        seat()->keyboard()->sendKeyEvent(keyCode, keyState);

    if (!auxKeyboardImplementationEnabled())
        return;

    bool L_CTRL = seat()->keyboard()->isKeyCodePressed(KEY_LEFTCTRL);
    bool L_SHIFT = seat()->keyboard()->isKeyCodePressed(KEY_LEFTSHIFT);
    bool mods = seat()->keyboard()->isKeyCodePressed(KEY_LEFTALT) && L_CTRL;

    if (keyState == LKeyboard::Released)
    {
        // Launches weston-terminal
        if (keyCode == KEY_F1 && !mods)
        {
            if (fork() == 0)
            {
                execl("/usr/bin/weston-terminal", "weston-terminal", NULL);
                exit(0);
            }
        }

        // Terminates client connection
        else if (L_CTRL && seat()->keyboard()->keySymbol(keyCode) == XKB_KEY_q)
        {
            if (seat()->keyboard()->focusSurface())
                seat()->keyboard()->focusSurface()->client()->destroy();
        }

        // Minimizes currently focused surface
        else if (L_CTRL && seat()->keyboard()->keySymbol(keyCode) == XKB_KEY_m)
        {
            if (seat()->keyboard()->focusSurface() && seat()->keyboard()->focusSurface()->toplevel())
                seat()->keyboard()->focusSurface()->toplevel()->setMinimizedRequest();
        }

        // Terminates the compositor
        else if (keyCode == KEY_ESC && L_CTRL && L_SHIFT)
            compositor()->finish();

        else if (L_CTRL && !L_SHIFT)
            seat()->dndManager()->setPreferredAction(LDNDManager::Copy);
        else if (!L_CTRL && L_SHIFT)
            seat()->dndManager()->setPreferredAction(LDNDManager::Move);
        else if (!L_CTRL && !L_SHIFT)
            seat()->dndManager()->setPreferredAction(LDNDManager::NoAction);
    }

    // Key press
    else
    {
        // CTRL sets Copy as the preferred action in drag & drop sesión
        if (L_CTRL)
            seat()->dndManager()->setPreferredAction(LDNDManager::Copy);

        // SHIFT sets the Move as the preferred action in drag & drop sesión
        else if (L_SHIFT)
            seat()->dndManager()->setPreferredAction(LDNDManager::Move);
    }
}

bool LScene::handleWaylandKeyboardEventsEnabled() const
{
    return imp()->handleWaylandKeyboardEvents;
}

void LScene::enableHandleWaylandKeyboardEvents(bool enabled)
{
    imp()->handleWaylandKeyboardEvents = enabled;
}

bool LScene::auxKeyboardImplementationEnabled() const
{
    return imp()->auxKeyboardImplementationEnabled;
}

void LScene::enableAuxKeyboardImplementation(bool enabled)
{
    imp()->auxKeyboardImplementationEnabled = enabled;
}

LSceneView *LScene::mainView() const
{
    return imp()->view;
}

LView *LScene::viewAt(const LPoint &pos)
{
    return imp()->viewAt(mainView(), pos);
}

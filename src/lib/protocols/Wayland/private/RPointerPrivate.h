#ifndef RPOINTERPRIVATE_H
#define RPOINTERPRIVATE_H

#include <protocols/Wayland/RPointer.h>

using namespace Louvre::Protocols::Wayland;

LPRIVATE_CLASS(RPointer)
    static void set_cursor(wl_client *client, wl_resource *resource, UInt32 serial, wl_resource *_surface, Int32 hotspot_x, Int32 hotspot_y);
#if LOUVRE_WL_SEAT_VERSION >= 5
    static void release(wl_client *client, wl_resource *resource);
#endif

    GSeat *gSeat;
    std::vector<Protocols::RelativePointer::RRelativePointer*> relativePointerResources;
    std::vector<Protocols::PointerGestures::RGestureSwipe*> gestureSwipeResources;
    std::vector<Protocols::PointerGestures::RGesturePinch*> gesturePinchResources;
    std::vector<Protocols::PointerGestures::RGestureHold*> gestureHoldResources;
};

#endif // RPOINTERPRIVATE_H

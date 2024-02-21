#ifndef RGESTURESWIPEPRIVATE_H
#define RGESTURESWIPEPRIVATE_H

#include <protocols/PointerGestures/RGestureSwipe.h>

using namespace Louvre::Protocols::Wayland;
using namespace Louvre::Protocols::PointerGestures;

LPRIVATE_CLASS(RGestureSwipe)
static void destroy(wl_client *client, wl_resource *resource);

RPointer *rPointer { nullptr };
};

#endif // RGESTURESWIPEPRIVATE_H

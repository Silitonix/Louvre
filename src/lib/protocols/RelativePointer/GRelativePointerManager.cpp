#include <protocols/RelativePointer/private/GRelativePointerManagerPrivate.h>
#include <protocols/RelativePointer/relative-pointer-unstable-v1.h>
#include <private/LClientPrivate.h>

using namespace Louvre::Protocols::RelativePointer;

GRelativePointerManager::GRelativePointerManager
    (
        wl_client *client,
        const wl_interface *interface,
        Int32 version,
        UInt32 id,
        const void *implementation
        )
    :LResource
    (
        client,
        interface,
        version,
        id,
        implementation
    ),
    LPRIVATE_INIT_UNIQUE(GRelativePointerManager)
{
    this->client()->imp()->relativePointerManagerGlobals.push_back(this);
}

GRelativePointerManager::~GRelativePointerManager()
{
    LVectorRemoveOneUnordered(client()->imp()->relativePointerManagerGlobals, this);
}


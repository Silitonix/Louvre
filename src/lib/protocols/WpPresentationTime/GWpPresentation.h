#ifndef PRESENTATION_H
#define PRESENTATION_H

#include <LResource.h>

class Louvre::Protocols::WpPresentationTime::GWpPresentation : public LResource
{
public:
    GWpPresentation(wl_client *client,
                    const wl_interface *interface,
                    Int32 version,
                    UInt32 id,
                    const void *implementation);

    ~GWpPresentation();

    // Since 1
    bool clockId(UInt32 clockId);

    LPRIVATE_IMP_UNIQUE(GWpPresentation)
};

#endif // PRESENTATION_H

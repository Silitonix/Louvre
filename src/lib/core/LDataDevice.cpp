#include <private/LDataDevicePrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LDataOfferPrivate.h>
#include <private/LDataSourcePrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LDNDManagerPrivate.h>

#include <protocols/Wayland/RDataOffer.h>
#include <protocols/Wayland/GSeat.h>
#include <protocols/Wayland/private/RDataDevicePrivate.h>


#include <LWayland.h>
#include <LTime.h>

using namespace Louvre;


LDataDevice::LDataDevice()
{
    m_imp = new LDataDevicePrivate();
}

LDataDevice::~LDataDevice()
{
    delete m_imp;
}

LClient *LDataDevice::client() const
{
    return imp()->client;
}

LSeat *LDataDevice::seat() const
{
    return client()->seat();
}

void LDataDevice::sendSelectionEvent()
{
    // Send data device selection first
    if (seat()->dataSelection())
    {
        for (Protocols::Wayland::GSeat *d : client()->seatGlobals())
        {
            if (d->dataDeviceResource())
            {
                Protocols::Wayland::RDataOffer *lRDataOffer = new Protocols::Wayland::RDataOffer(d->dataDeviceResource(), 0);

                lRDataOffer->dataOffer()->imp()->usedFor = LDataOffer::Selection;
                d->dataDeviceResource()->sendDataOffer(lRDataOffer);

                for (const LDataSource::LSource &s : seat()->dataSelection()->sources())
                    lRDataOffer->sendOffer(s.mimeType);

                d->dataDeviceResource()->sendSelection(lRDataOffer);

            }
        }
    }
}

void LDataDevice::LDataDevicePrivate::sendDNDEnterEvent(LSurface *surface, Float64 x, Float64 y)
{
    LSeat *seat = client->seat();

    if (seat->dndManager()->dragging() && seat->dndManager()->focus() != surface)
    {
        sendDNDLeaveEvent();

        if (seat->dndManager()->source())
        {
            for (Protocols::Wayland::GSeat *d : client->seatGlobals())
            {
                if (d->dataDeviceResource())
                {
                    Protocols::Wayland::RDataOffer *lRDataOffer = new Protocols::Wayland::RDataOffer(d->dataDeviceResource(), 0);

                    lRDataOffer->dataOffer()->imp()->usedFor = LDataOffer::DND;
                    d->dataDeviceResource()->imp()->dataOffered = lRDataOffer->dataOffer();
                    d->dataDeviceResource()->sendDataOffer(lRDataOffer);

                    for (const LDataSource::LSource &s : seat->dndManager()->source()->sources())
                        lRDataOffer->sendOffer(s.mimeType);

                    d->dataDeviceResource()->sendEnter(surface,
                                                       x/seat->compositor()->globalScale(),
                                                       y/seat->compositor()->globalScale(),
                                                       lRDataOffer);

                    lRDataOffer->sendSourceActions(seat->dndManager()->source()->dndActions());


                }
            }

            seat->dndManager()->imp()->focus = surface;
        }
        else
        {
            if (surface == seat->dndManager()->origin())
            {

                for (Protocols::Wayland::GSeat *d : client->seatGlobals())
                {
                    if (d->dataDeviceResource())
                    {
                        d->dataDeviceResource()->sendEnter(surface,
                                                           x/seat->compositor()->globalScale(),
                                                           y/seat->compositor()->globalScale(),
                                                           NULL);
                    }
                }

                seat->dndManager()->imp()->focus = surface;
            }
        }
    }
}

void LDataDevice::LDataDevicePrivate::sendDNDMotionEvent(Float64 x, Float64 y)
{
    LSeat *seat = client->seat();

    if (seat->dndManager()->dragging() && seat->dndManager()->focus())
    {
        if (seat->dndManager()->source() || (!seat->dndManager()->source() && seat->dndManager()->focus() == seat->dndManager()->origin()))
        {
            for (Protocols::Wayland::GSeat *d : client->seatGlobals())
                if (d->dataDeviceResource())
                    d->dataDeviceResource()->sendMotion(x/seat->compositor()->globalScale(), y/seat->compositor()->globalScale());
        }
    }
}

void LDataDevice::LDataDevicePrivate::sendDNDLeaveEvent()
{
    LSeat *seat = client->seat();

    if (seat->dndManager()->dragging() && seat->dndManager()->focus())
        for (Protocols::Wayland::GSeat *d : client->seatGlobals())
            if (d->dataDeviceResource())
                d->dataDeviceResource()->sendLeave();

    seat->dndManager()->imp()->matchedMimeType = false;
    seat->dndManager()->imp()->focus = nullptr;
}

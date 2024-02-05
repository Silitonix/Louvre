#ifndef SURFACE_H
#define SURFACE_H

#include <LSurface.h>
#include <map>

using namespace Louvre;
using namespace std;

class Surface : public LSurface
{
public:
    Surface(const void *params);
    void mappingChanged() override;
    void orderChanged() override;
    void minimizedChanged() override;

    // Temp variables used in paintGL()
    bool firstMap = true;
    bool occluded = false;
    bool isRenderable = false;
    LRect currentRect;
    LRegion currentOpaqueTranslated;
    LRegion currentDamageTranslated;
    LRegion currentOpaqueTranslatedSum;
    LRegion currentTranslucentTranslated;

    // Cached data for each output
    struct OutputData
    {
        UInt32 lastRenderedDamageId;
        LRect previousRect;
        bool changedOrder = true;
    };
    map<LOutput*,OutputData>outputsMap;

    // Handle to prevent looking in the map each time
    OutputData *currentOutputData;
};

#endif // SURFACE_H

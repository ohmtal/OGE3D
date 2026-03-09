add:
#define TORQUE_INTERIOR_ENABLED
to torqueConfig.h to get it work! 
------------------------------
finally found: SceneZoneSpaceManager


...................
** when inside a zone the rendering is never called **

bool InteriorInstance::onAdd() << load the interior
=> bool InteriorInstance::_loadInterior()

node.exterior << we are inside

void Interior::cloneMatInstances() << insert the render materials


mZoneRNList.push_back( RNList ); << this is the zone list ?!




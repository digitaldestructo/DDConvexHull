//
//  DDConvexHullPlugin.cpp
//  DDConvexHull
//
//  Created by Jonathan Tilden on 12/30/12.
//  Copyright (c) 2012 Jonathan Tilden. All rights reserved.
//

#include <maya/MFnPlugin.h>
#include "DDConvexHullNode.h"
#include "DDConvexHullCmd.h"

MStatus initializePlugin(MObject obj)
{
    MFnPlugin plugin(obj, "DigitalDestructo", "1.0", "Any");
    plugin.registerNode("DDConvexHull",
                        DDConvexHullNode::id,
                        DDConvexHullNode::creator,
                        DDConvexHullNode::initialize);
    plugin.registerCommand("DDConvexHull", DDConvexHullCmd::creator);
    return MS::kSuccess;
}

MStatus uninitializePlugin( MObject obj )
{
    MFnPlugin plugin( obj );
    plugin.deregisterNode(DDConvexHullNode::id);
    plugin.deregisterCommand("DDConvexHull");
    return MS::kSuccess;
}

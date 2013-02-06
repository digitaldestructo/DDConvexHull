//
//  DDConvexHullCmd.cpp
//  DDConvexHull
//
//  Created by Jonathan Tilden on 1/1/13.
//  Copyright (c) 2013 Jonathan Tilden. All rights reserved.
//

#include "DDConvexHullCmd.h"
#include "DDConvexHullUtils.h"
#include <maya/MString.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MDagModifier.h>

#include <sstream>
MString DDConvexHullCmd::dbl_to_string(double x)
{
    std::ostringstream ss;
    ss << x;
    return MString((ss.str()).c_str());
}

MString DDConvexHullCmd::int_to_string(int x)
{
    std::ostringstream ss;
    ss << x;
    return MString((ss.str()).c_str());
}

MStatus DDConvexHullCmd::doIt(const MArgList& args)
{
    if (args.length() != 1)
    {
        MGlobal::displayError("Needs at least 2 args");
        return MS::kFailure;
    }
    MString input = args.asString(0);
    MString output = args.asString(1);
    
    // Get the mObject for the input
    MSelectionList selList;
    selList.add(input);
    MDagPath inputMesh;
    selList.getDagPath(0, inputMesh);
    
    // Ensure we're looking at the shape
    inputMesh.extendToShape();

    // Create output object
    MDagModifier dm;
    MObject outMeshNode = dm.createNode(MFn::kMesh);
    MFnDependencyNode outMeshDag(outMeshNode);
    outMeshDag.setName("poopShape#");
    return DDConvexHullUtils::generateMayaHull(inputMesh.node(), outMeshNode);

}

void* DDConvexHullCmd::creator()
{
    return new DDConvexHullCmd;
}
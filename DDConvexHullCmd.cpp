//
// DDConvexHullCmd.cpp
// DDConvexHull
//
// Created by Jonathan Tilden on 1/1/13.
//
// MIT License
//
// Copyright (c) 2017 Jonathan Tilden

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in 
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
    DDConvexHullUtils::hullOpts hullOptions;
    return DDConvexHullUtils::generateMayaHull(outMeshNode,
                                               inputMesh.node(), hullOptions);

}

void* DDConvexHullCmd::creator()
{
    return new DDConvexHullCmd;
}
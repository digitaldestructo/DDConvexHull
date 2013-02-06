//
//  DDConvexHullUtils.h
//  DDConvexHull
//
//  Created by Jonathan Tilden on 1/28/13.
//  Copyright (c) 2013 Jonathan Tilden. All rights reserved.
//

#ifndef __DDConvexHull__DDConvexHullUtils__
#define __DDConvexHull__DDConvexHullUtils__

#include <maya/MObject.h>
#include <maya/MString.h>
#include <maya/MStatus.h>

namespace DDConvexHullUtils
{;
    MStatus generateMayaHull(const MObject &input,
                          MObject &output,
                          bool forceTriangles=false,
                          uint maxOutputVertices=4096,
                          bool useSkinWidth=false,
                          double skinWidth=0.01f,
                          double normalEpsilon=0.001f,
                          bool reverseTriangleOrder=false);
    MString dbl_to_string(double x);
    MString int_to_string(int x);
};

#endif /* defined(__DDConvexHull__DDConvexHullUtils__) */

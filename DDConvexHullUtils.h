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
#include <maya/MPointArray.h>
#include <maya/MString.h>
#include <maya/MStatus.h>
#include <maya/MIntArray.h>

namespace DDConvexHullUtils
{;
    struct hullOpts
    {
        // Create a constructor with default values
        hullOpts() : forceTriangles(false),
                     maxOutputVertices(4096),
                     useSkinWidth(false),
                     skinWidth(0.01f),
                     normalEpsilon(0.001f),
                     reverseTriangleOrder(false) {}
        
        bool forceTriangles;
        uint maxOutputVertices;
        bool useSkinWidth;
        double skinWidth;
        double normalEpsilon;
        bool reverseTriangleOrder;
    };
    
    MStatus generateMayaHull(MObject &output,
                             const MPointArray &vertices,
                             const hullOpts &hullOptions);
    
    MStatus generateMayaHull(MObject &output,
                             const MObject &input,
                             const hullOpts &hullOptions);
    
    MStatus componentToVertexIDs(MIntArray &outIndices,
                                 const MObject &mesh,
                                 const MObject &component);
 
    MString dbl_to_string(double x);
    MString int_to_string(int x);
};

#endif /* defined(__DDConvexHull__DDConvexHullUtils__) */

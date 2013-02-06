//
//  DDConvexHullUtils.cpp
//  DDConvexHull
//
//  Created by Jonathan Tilden on 1/28/13.
//  Copyright (c) 2013 Jonathan Tilden. All rights reserved.
//

#include "DDConvexHullUtils.h"
#include "StanHull/hull.h"
#include <maya/MFnMesh.h>
#include <maya/MFnMesh.h>
#include <maya/MIntArray.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <sstream>
MString DDConvexHullUtils::dbl_to_string(double x)
{
    std::ostringstream ss;
    ss << x;
    return MString((ss.str()).c_str());
}

MString DDConvexHullUtils::int_to_string(int x)
{
    std::ostringstream ss;
    ss << x;
    return MString((ss.str()).c_str());
}

MStatus DDConvexHullUtils::generateMayaHull(const MObject &input,
                                            MObject &output,
                                            bool forceTriangles,
                                            uint maxOutputVertices,
                                            bool useSkinWidth,
                                            double skinWidth,
                                            double normalEpsilon,
                                            bool reverseTriangleOrder)
{
    // Convert the input mobject to mfnmesh
    if (!input.hasFn(MFn::kMesh))
    {
        return MStatus::kInvalidParameter;
    }
    MFnMesh inputMesh(input);
    
    // Allocate an array for the vertices and fill it with the data extracted
    // from the input mesh
    uint numInputVerts = inputMesh.numVertices();
    if (!numInputVerts)
    {
        return MStatus::kFailure;
    }

    // Mem Cleanup req.
    double *inputVerts = new double[numInputVerts*3];
    
    MPointArray points;
    inputMesh.getPoints(points);
    for (uint i=0; i < numInputVerts; i++)
    {
        uint offset = i*3;
        inputVerts[offset]   = points[i].x;
        inputVerts[offset+1] = points[i].y;
        inputVerts[offset+2] = points[i].z;
    }
    
    
    // Setup the flags
    uint hullFlags = QF_DEFAULT;
    if (forceTriangles)
    {
        hullFlags |= QF_TRIANGLES;
    }
    if (useSkinWidth)
    {
        hullFlags |= QF_SKIN_WIDTH;
    }
    if (reverseTriangleOrder)
    {
        hullFlags |= QF_REVERSE_ORDER;
    }
    
    // Create the description
    HullDesc hullDescription;
    hullDescription.mFlags = hullFlags;
    hullDescription.mMaxVertices = maxOutputVertices;
    hullDescription.mSkinWidth = skinWidth;
    hullDescription.mNormalEpsilon = normalEpsilon;
    hullDescription.mVertexStride = sizeof(double)*3;
    hullDescription.mVcount = numInputVerts;
    hullDescription.mVertices = inputVerts;
    
    // Create the hull
    HullLibrary hullComputer;
    HullResult hullResult;
    HullError err = hullComputer.CreateConvexHull(hullDescription, hullResult);
    MStatus hullStat = MStatus::kSuccess;
    if (err == QE_OK)
    {
        // Grab the verts
        MPointArray outPoints;
        for (uint i=0; i < hullResult.mNumOutputVertices; i++)
        {
            uint offset = i*3;
            MPoint curPoint(hullResult.mOutputVertices[offset],
                            hullResult.mOutputVertices[offset+1],
                            hullResult.mOutputVertices[offset+2]);
            outPoints.append(curPoint);
        }
        
        // Check if the results are in polygons, or triangles. Depending on
        // which for the result is in, the way the face indices are setup
        // is different.
        MIntArray polyCounts;
        MIntArray vertexConnects;
        
        if (hullResult.mPolygons)
        {
            const uint *idx = hullResult.mIndices;
            for (uint i=0; i < hullResult.mNumFaces; i++)
            {
                uint pCount = *idx++;
                polyCounts.append(pCount);
                
                for (uint j=0; j < pCount; j++)
                {
                    uint val = idx[0];
                    vertexConnects.append(val);
                    idx++;
                }
            }
        }
        else
        {
            for (uint i=0; i < hullResult.mNumFaces; i++)
            {
                polyCounts[i] = 3;
                uint *idx = &hullResult.mIndices[i*3];
                
                vertexConnects.append(idx[0]);
                vertexConnects.append(idx[1]);
                vertexConnects.append(idx[2]);
            }
        }
        // Setup the outmesh
        MFnMesh outMeshFn(output);
        outMeshFn.create(hullResult.mNumOutputVertices,
                         hullResult.mNumFaces,
                         outPoints,
                         polyCounts,
                         vertexConnects,
                         output,
                         &hullStat);
    }
    else
    {
        hullStat = MStatus::kFailure;
    }
    
    // Mem Cleanup
    delete[] inputVerts;
    
    return hullStat;
}
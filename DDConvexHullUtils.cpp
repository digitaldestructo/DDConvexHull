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

MStatus DDConvexHullUtils::generateMayaHull(MObject &output,
                                const MObject &input,
                                const DDConvexHullUtils::hullOpts &hullOptions)
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
    
    MPointArray points;
    inputMesh.getPoints(points);
    
    return generateMayaHull(output, points, hullOptions);
}

MStatus DDConvexHullUtils::generateMayaHull(MObject &output,
                                const MPointArray &vertices,
                                const DDConvexHullUtils::hullOpts &hullOptions)
{
    // Allocate and push the vert list into the new array Mem Cleanup req.
    uint numInputVerts = vertices.length();
    double *inputVerts = new double[numInputVerts*3];
    for (uint i=0; i < numInputVerts; i++)
    {
        uint offset = i*3;
        inputVerts[offset]   = vertices[i].x;
        inputVerts[offset+1] = vertices[i].y;
        inputVerts[offset+2] = vertices[i].z;
    }
    
    // Setup the flags
    uint hullFlags = QF_DEFAULT;
    if (hullOptions.forceTriangles)
    {
        hullFlags |= QF_TRIANGLES;
    }
    if (hullOptions.useSkinWidth)
    {
        hullFlags |= QF_SKIN_WIDTH;
    }
    if (hullOptions.reverseTriangleOrder)
    {
        hullFlags |= QF_REVERSE_ORDER;
    }
    
    // Create the description
    HullDesc hullDescription;
    hullDescription.mFlags = hullFlags;
    hullDescription.mMaxVertices = hullOptions.maxOutputVertices;
    hullDescription.mSkinWidth = hullOptions.skinWidth;
    hullDescription.mNormalEpsilon = hullOptions.normalEpsilon;
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
            polyCounts.setLength(hullResult.mNumFaces);
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
    hullComputer.ReleaseResult(hullResult);
    delete[] inputVerts;
    
    return hullStat;
}
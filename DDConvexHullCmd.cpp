//
//  DDConvexHullCmd.cpp
//  DDConvexHull
//
//  Created by Jonathan Tilden on 1/1/13.
//  Copyright (c) 2013 Jonathan Tilden. All rights reserved.
//

#include "DDConvexHullCmd.h"
#include <StanHull/hull.h>
#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MFnMesh.h>
#include <maya/MIntArray.h>
#include <maya/MFloatPoint.h>
#include <maya/MFloatPointArray.h>
#include <maya/MPointArray.h>

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
    MFnMesh inputMeshFn(inputMesh);
    
    // Create the convex hull
    // Iterate through the mesh and build out the required bits of data
    HullDesc desc;
    //desc.mFlags  = QF_TRIANGLES | QF_SKIN_WIDTH;
    desc.mVcount = inputMeshFn.numVertices();
    //desc.mMaxVertices = desc.mVcount;
    desc.mVertices = new double[desc.mVcount*3];
    desc.mVertexStride = sizeof(double)*3;
    
    MPointArray points;
    inputMeshFn.getPoints(points, MSpace::kWorld);
    for (uint i=0; i < desc.mVcount; i++)
    {
        desc.mVertices[i*3]   = points[i].x;
        desc.mVertices[i*3+1] = points[i].y;
        desc.mVertices[i*3+2] = points[i].z;
        MGlobal::displayInfo((dbl_to_string(points[i].x) + " " +
                              dbl_to_string(points[i].y) + " " +
                              dbl_to_string(points[i].z)));
    }
    
    // Using the desc object, create the hull
    HullResult fhull;
    HullLibrary hullComputer;
    HullError ret = hullComputer.CreateConvexHull(desc, fhull);
    //FHullResult fhull(hull);
    if (ret == QE_OK)
    {
        // Grab the verts
        MPointArray outPoints;
        for (uint i=0; i < fhull.mNumOutputVertices; i++)
        {
            MFloatPoint curPoint(fhull.mOutputVertices[i*3],
                                 fhull.mOutputVertices[i*3+1],
                                 fhull.mOutputVertices[i*3+2]);
            //MGlobal::displayInfo(<#const MString &theMessage#>)
            outPoints.append(curPoint);
        }
        
        // Check if the results are in polygons, or triangles. Depending on
        // which for the result is in, the way the face indices are setup
        // is different.
        MIntArray polyCounts;
        MIntArray vertexConnects;
        
        if (fhull.mPolygons)
        {
            const uint *idx = fhull.mIndices;
            for (uint i=0; i < fhull.mNumFaces; i++)
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
            for (uint i=0; i < fhull.mNumFaces; i++)
            {
                polyCounts[i] = 3;
                uint *idx = &fhull.mIndices[i*3];
                
                vertexConnects.append(idx[0]);
                vertexConnects.append(idx[1]);
                vertexConnects.append(idx[2]);
            }
        }
        
        MGlobal::displayInfo(int_to_string(fhull.mNumOutputVertices));
        MGlobal::displayInfo(int_to_string(fhull.mNumFaces));
        MGlobal::displayInfo(int_to_string(polyCounts.length()));
        MGlobal::displayInfo(int_to_string(vertexConnects.length()));
        MGlobal::displayInfo((MString("PTS: ") + int_to_string(outPoints.length())));
        for (uint i=0; i < outPoints.length(); i++)
        {
            MString x = dbl_to_string(outPoints[i].x);
            MString y = dbl_to_string(outPoints[i].y);
            MString z = dbl_to_string(outPoints[i].z);
            MGlobal::displayInfo((x+" "+y+" "+z));
        }
        MString myverts = "";
        for (uint i=0; i < vertexConnects.length(); i++)
        {
            myverts += (int_to_string(vertexConnects[i]) + " ");
        }
        MGlobal::displayInfo(myverts);
        
        // Setup the outmesh
        MFnMesh outMeshFn;
        outMeshFn.create(fhull.mNumOutputVertices,
                         fhull.mNumFaces,
                         outPoints,
                         polyCounts,
                         vertexConnects);
    }

    
    return MS::kSuccess;
}

void* DDConvexHullCmd::creator()
{
    return new DDConvexHullCmd;
}
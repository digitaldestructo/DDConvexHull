//
//  DDConvexHullNode.cpp
//  DDConvexHull
//
//  Created by Jonathan Tilden on 12/30/12.
//  Copyright (c) 2012 Jonathan Tilden. All rights reserved.
//

#include "DDConvexHullNode.h"

#include <maya/MStatus.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnMeshData.h>
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>
#include <stdio.h>

// Attribute definitions
MObject DDConvexHullNode::aInputPolymesh;
MObject DDConvexHullNode::aOutput;

// Maya Node ID
MTypeId DDConvexHullNode::id(0x7FFFF);


// Constructor / destructor
DDConvexHullNode::DDConvexHullNode(){}
DDConvexHullNode::~DDConvexHullNode(){}


void* DDConvexHullNode::creator()
{
    return new DDConvexHullNode;
}


MStatus DDConvexHullNode::initialize()
{
    MStatus stat;
    
    // InputPolymesh
    MFnTypedAttribute inputPolymeshAttr;
    aInputPolymesh = inputPolymeshAttr.create("inputPolymesh", "ip",
                                             MFnData::kMesh, &stat);
    if (stat != MStatus::kSuccess)
    {
        return stat;
    }
    //inputPolymeshAttr.setKeyable(false);
    
    // Output
    MFnTypedAttribute outputAttr;
    aOutput = outputAttr.create("output", "out", MFnData::kMesh, &stat);
    if (stat != MStatus::kSuccess)
    {
        return stat;
    }
    outputAttr.setWritable(false);
    outputAttr.setStorable(false);
    outputAttr.setKeyable(false);
    
    // Add the attributes
    addAttribute(aInputPolymesh);
    addAttribute(aOutput);
    
    // Setup attribute relationships
    attributeAffects(aInputPolymesh, aOutput);
    return MStatus::kSuccess;
}


HullError DDConvexHullNode::createConvexHull(MObject &outMesh,
                                             const MFnMesh &inMeshFn)
{
    return QE_OK;
}


MStatus DDConvexHullNode::compute(const MPlug &plug, MDataBlock &data)
{
    MStatus stat;
    if (plug == aOutput)
    {
        // Get the data from the input and set it to the output
        MDataHandle inputData  = data.inputValue(aInputPolymesh, &stat);
        if (stat != MStatus::kSuccess)
        {
            return stat;
        }
        
        MObject inputMesh   = inputData.asMesh();
        MFnMesh inputMeshFn(inputMesh, &stat);
        if (stat != MStatus::kSuccess)
        {
            return stat;
        }
        
        MFnMeshData outputDataCreator;
        MObject outputMesh = outputDataCreator.create(&stat);
        if (stat != MStatus::kSuccess)
        {
            return stat;
        }

/*        
        // Compute the hull
        HullError res = createConvexHull(outputMesh, inputMeshFn);
        if (res != QE_OK)
        {
            return MStatus::kFailure;
        }
 */
 
        // Iterate through the mesh and build out the required bits of data
        HullDesc desc;
        //desc.mFlags  = QF_TRIANGLES | QF_SKIN_WIDTH;
        desc.mVcount = inputMeshFn.numVertices();
        //desc.mMaxVertices = desc.mVcount;
        desc.mVertices = new double[desc.mVcount*3];
        desc.mVertexStride = sizeof(double)*3;
        
        MPointArray points;
        inputMeshFn.getPoints(points);
        for (uint i=0; i < desc.mVcount; i++)
        {
            desc.mVertices[i*3]   = points[i].x;
            desc.mVertices[i*3+1] = points[i].y;
            desc.mVertices[i*3+2] = points[i].z;
        }
        
        // Using the desc object, create the hull
        HullResult hull;
        HullLibrary hullComputer;
        HullError ret = hullComputer.CreateConvexHull(desc, hull);
        if (ret == QE_OK)
        {
            // Grab the verts
            MPointArray outPoints;
            for (uint i=0; i < hull.mNumOutputVertices; i++)
            {
                MPoint curPoint(hull.mOutputVertices[i*3],
                                hull.mOutputVertices[i*3+1],
                                hull.mOutputVertices[i*3+2]);
                outPoints.append(curPoint);
            }
            
            // Check if the results are in polygons, or triangles. Depending on
            // which for the result is in, the way the face indices are setup
            // is different.
            MIntArray polyCounts;
            MIntArray vertexConnects;
            
            if (hull.mPolygons)
            {
                const uint *idx = hull.mIndices;
                for (uint i=0; i < hull.mNumFaces; i++)
                {
                    uint pCount = *idx++;
                    polyCounts.append(pCount);
                    
                    for (uint j=0; j < pCount; j++)
                    {
                        uint val = idx[0] + 1;
                        vertexConnects.append(val);
                        idx++;
                    }
                }
            }
            else
            {
                for (uint i=0; i < hull.mNumFaces; i++)
                {
                    polyCounts[i] = 3;
                    uint *idx = &hull.mIndices[i*3];
                    vertexConnects.append(idx[0]+1);
                    vertexConnects.append(idx[1]+1);
                    vertexConnects.append(idx[2]+1);
                }
            }
                        
            // Setup the outmesh
            MFnMesh outMeshFn;
            outMeshFn.create(hull.mNumOutputVertices,
                             hull.mNumFaces,
                             outPoints,
                             polyCounts,
                             vertexConnects,
                             outputMesh,
                             &stat);
        }
        
        MDataHandle outputData = data.outputValue(aOutput, &stat);
        if (stat != MStatus::kSuccess)
        {
            return stat;
        }

        
        
        
        if (stat != MStatus::kSuccess)
        {
            return stat;
        }
 
 
        // Set the data
        //inputMeshFn.copy(inputMesh, outputMesh);
        outputData.set(outputMesh);
        data.setClean(aOutput);
        
        // Cleanup and return
        delete desc.mVertices;
        hullComputer.ReleaseResult(hull);

    }
    else
    {
        return MStatus::kUnknownParameter;
    }
    return MStatus::kSuccess;
}
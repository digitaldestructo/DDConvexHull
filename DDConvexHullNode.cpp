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
#include <maya/MFnMesh.h>

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
        MDataHandle outputData = data.outputValue(aOutput, &stat);
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
        
        // Set the data
        inputMeshFn.copy(inputMesh, outputMesh);
        outputData.set(outputMesh);
        data.setClean(aOutput);

    }
    else
    {
        return MStatus::kUnknownParameter;
    }
    return MStatus::kSuccess;
}
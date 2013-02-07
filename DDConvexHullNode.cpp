//
//  DDConvexHullNode.cpp
//  DDConvexHull
//
//  Created by Jonathan Tilden on 12/30/12.
//  Copyright (c) 2012 Jonathan Tilden. All rights reserved.
//

#include "DDConvexHullNode.h"
#include "DDConvexHullUtils.h"
#include <maya/MStatus.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnNumericData.h>
#include <maya/MFnMeshData.h>
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>
#include <stdio.h>

// Attribute definitions
MObject DDConvexHullNode::inputPolymeshAttr;
MObject DDConvexHullNode::outputPolymeshAttr;
MObject DDConvexHullNode::useSkinWidthAttr;
MObject DDConvexHullNode::skinWidthAttr;
MObject DDConvexHullNode::normalEpsilonAttr;
MObject DDConvexHullNode::useTrianglesAttr;
MObject DDConvexHullNode::maxOutputVerticesAttr;
MObject DDConvexHullNode::useReverseTriOrderAttr;


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
    MFnTypedAttribute inputPolymeshAttrFn;
    inputPolymeshAttr = inputPolymeshAttrFn.create("inputPolymesh", "ip",
                                                   MFnData::kMesh, &stat);
    if (stat != MStatus::kSuccess)
    {
        return stat;
    }
        
    // Output
    MFnTypedAttribute outputPolymeshAttrFn;
    outputPolymeshAttr = outputPolymeshAttrFn.create("output", "out",
                                                     MFnData::kMesh, &stat);
    if (stat != MStatus::kSuccess)
    {
        return stat;
    }
    outputPolymeshAttrFn.setWritable(false);
    outputPolymeshAttrFn.setStorable(false);
    outputPolymeshAttrFn.setKeyable(false);
    
    // Skin Width Attrs
    MFnNumericAttribute useSkinWidthAttrFn;
    useSkinWidthAttr = useSkinWidthAttrFn.create("skinWidthEnabled","skwen",
                                                 MFnNumericData::kBoolean,
                                                 false, &stat);
    useSkinWidthAttrFn.setWritable(true);
    useSkinWidthAttrFn.setStorable(true);
    useSkinWidthAttrFn.setKeyable(true);
    useSkinWidthAttrFn.setDefault(false);
    
    MFnNumericAttribute skinWidthFn;
    skinWidthAttr = skinWidthFn.create("skinWidth", "skw",
                                       MFnNumericData::kDouble, .01f, &stat);
    skinWidthFn.setWritable(true);
    skinWidthFn.setStorable(true);
    skinWidthFn.setKeyable(true);
    skinWidthFn.setDefault(0.01f);
    
    // Normal Epsilon
    MFnNumericAttribute normalEpsilonAttrFn;
    normalEpsilonAttr = normalEpsilonAttrFn.create("normalEpisilon", "ep",
                                                   MFnNumericData::kDouble,
                                                   .001f, &stat);
    normalEpsilonAttrFn.setWritable(true);
    normalEpsilonAttrFn.setStorable(true);
    normalEpsilonAttrFn.setKeyable(true);
    normalEpsilonAttrFn.setDefault(0.001f);
    normalEpsilonAttrFn.setMin(0.000001f);
    
    // Force usage of triangles
    // NOTE: As far as I can tell, the hulls always seem to come in already
    //       triangulated.  The code looks to read as only "report" as tris
    //       instead of as polys.  Not sure what the difference is here, so
    //       for now, the attribute is hidden and defaulting to true.
    MFnNumericAttribute useTrianglesAttrFn;
    useTrianglesAttr = useSkinWidthAttrFn.create("forceTriangles","tri",
                                                 MFnNumericData::kBoolean,
                                                 true, &stat);
    useTrianglesAttrFn.setWritable(true);
    useTrianglesAttrFn.setStorable(true);
    useTrianglesAttrFn.setKeyable(true);
    useTrianglesAttrFn.setDefault(true);
    useTrianglesAttrFn.setHidden(true);
    
    // Maximum number of output verts attr
    MFnNumericAttribute maxOutputVerticesAttrFn;
    maxOutputVerticesAttr = maxOutputVerticesAttrFn.create("maxVertices", "max",
                                                           MFnNumericData::kInt,
                                                           4096, &stat);
    maxOutputVerticesAttrFn.setWritable(true);
    maxOutputVerticesAttrFn.setStorable(true);
    maxOutputVerticesAttrFn.setKeyable(true);
    maxOutputVerticesAttrFn.setDefault(4096);
    maxOutputVerticesAttrFn.setMin(4);
    
    // Reverse Triangle Order
    MFnNumericAttribute useReverseTriOrderAttrFn;
    useReverseTriOrderAttr = useReverseTriOrderAttrFn.create("reverseNormals",
                                              "rev", MFnNumericData::kBoolean,
                                              false, &stat);
    useReverseTriOrderAttrFn.setWritable(true);
    useReverseTriOrderAttrFn.setStorable(true);
    useReverseTriOrderAttrFn.setKeyable(true);
    useReverseTriOrderAttrFn.setDefault(false);
    
    
    // Add the attributes
    addAttribute(useSkinWidthAttr);
    addAttribute(skinWidthAttr);
    addAttribute(normalEpsilonAttr);
    addAttribute(useTrianglesAttr);
    addAttribute(maxOutputVerticesAttr);
    addAttribute(useReverseTriOrderAttr);
    addAttribute(inputPolymeshAttr);
    addAttribute(outputPolymeshAttr);
    
    // Setup attribute relationships
    attributeAffects(useSkinWidthAttr, outputPolymeshAttr);
    attributeAffects(skinWidthAttr, outputPolymeshAttr);
    attributeAffects(normalEpsilonAttr, outputPolymeshAttr);
    attributeAffects(useTrianglesAttr, outputPolymeshAttr);
    attributeAffects(maxOutputVerticesAttr, outputPolymeshAttr);
    attributeAffects(useReverseTriOrderAttr, outputPolymeshAttr);
    attributeAffects(inputPolymeshAttr, outputPolymeshAttr);
    return MStatus::kSuccess;
}


MStatus DDConvexHullNode::compute(const MPlug &plug, MDataBlock &data)
{
    MStatus stat;
    if (plug == outputPolymeshAttr)
    {
        // Get the data from the input and set it to the output
        MDataHandle inputData(data.inputValue(inputPolymeshAttr, &stat));
        if (stat != MStatus::kSuccess)
        {
            return stat;
        }
        MObject inputMesh = inputData.asMesh();
        
        // Skin Width
        MDataHandle useSkinWidthData(data.inputValue(useSkinWidthAttr, &stat));
        if (stat != MStatus::kSuccess)
        {
            return stat;
        }
        bool useSkinWidth = useSkinWidthData.asBool();
        
        MDataHandle skinWidthData(data.inputValue(skinWidthAttr, &stat));
        if (stat != MStatus::kSuccess)
        {
            return stat;
        }
        double skinWidth = skinWidthData.asDouble();
        
        // Epsilon
        MDataHandle normalEpsilonData(data.inputValue(normalEpsilonAttr,&stat));
        if (stat != MStatus::kSuccess)
        {
            return stat;
        }
        double normalEpsilon = normalEpsilonData.asDouble();
        
        // Force Triangles
        MDataHandle useTrianglesData(data.inputValue(useTrianglesAttr, &stat));
        if (stat != MStatus::kSuccess)
        {
            return stat;
        }
        bool useTriangles = useTrianglesData.asBool();
        
        // Output verts
        MDataHandle maxOutputVertData(data.inputValue(maxOutputVerticesAttr,
                                                      &stat));
        if (stat != MStatus::kSuccess)
        {
            return stat;
        }
        uint maxOutputVerts = maxOutputVertData.asInt();
        
        // Reverse Triangles
        MDataHandle useRevTriData(data.inputValue(useReverseTriOrderAttr,
                                                  &stat));
        if (stat != MStatus::kSuccess)
        {
            return stat;
        }
        bool useReverseTriOrder = useRevTriData.asBool();
        
        // Create output MObject
        MFnMeshData outputDataCreator;
        MObject outputMesh = outputDataCreator.create(&stat);
        if (stat != MStatus::kSuccess)
        {
            return stat;
        }

        // Generate the hull
        stat = DDConvexHullUtils::generateMayaHull(inputMesh,
                                                   outputMesh,
                                                   useTriangles,
                                                   maxOutputVerts,
                                                   useSkinWidth,
                                                   skinWidth,
                                                   normalEpsilon,
                                                   useReverseTriOrder);
        if (stat != MStatus::kSuccess)
        {
            return stat;
        }
        
        // Set the output Data
        MDataHandle outputData = data.outputValue(outputPolymeshAttr, &stat);
        if (stat != MStatus::kSuccess)
        {
            return stat;
        }
        outputData.set(outputMesh);
        data.setClean(outputPolymeshAttr);
    }
    else
    {
        return MStatus::kUnknownParameter;
    }
    return MStatus::kSuccess;
}
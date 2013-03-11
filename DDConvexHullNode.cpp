//
//  DDConvexHullNode.cpp
//  DDConvexHull
//
//  Created by Jonathan Tilden on 12/30/12.
//  Copyright (c) 2012 Jonathan Tilden. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is 
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "DDConvexHullNode.h"
#include "DDConvexHullUtils.h"
#include <maya/MArrayDataHandle.h>
#include <maya/MDataHandle.h>
#include <maya/MStatus.h>
#include <maya/MItMeshEdge.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnComponentListData.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnNumericData.h>
#include <maya/MFnMeshData.h>
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MGlobal.h>
#include <stdio.h>

// Attribute definitions
MObject DDConvexHullNode::inputAttr;
MObject DDConvexHullNode::inputComponentsAttr;
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
    inputPolymeshAttrFn.setDisconnectBehavior(MFnAttribute::kDelete);
    inputPolymeshAttrFn.setKeyable(false);
    
    // Input Components
    MFnTypedAttribute inputComponentsAttrFn;
    inputComponentsAttr = inputComponentsAttrFn.create("inputComponents",
                                                "ics", MFnData::kComponentList);
    inputComponentsAttrFn.setDisconnectBehavior(MFnAttribute::kReset);
    inputPolymeshAttrFn.setKeyable(false);
    
    // Setup the compound attr as array called input, with polymesh and
    // component inputs as children
    MFnCompoundAttribute inputAttrFn;
    inputAttr = inputAttrFn.create("input", "input");
    inputAttrFn.addChild(inputPolymeshAttr);
    inputAttrFn.addChild(inputComponentsAttr);
    inputAttrFn.setArray(true);
    inputAttrFn.setIndexMatters(false);
        
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
    normalEpsilonAttr = normalEpsilonAttrFn.create("normalEpsilon", "ep",
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
    addAttribute(inputAttr);
    addAttribute(outputPolymeshAttr);
    
    // Setup attribute relationships
    attributeAffects(useSkinWidthAttr, outputPolymeshAttr);
    attributeAffects(skinWidthAttr, outputPolymeshAttr);
    attributeAffects(normalEpsilonAttr, outputPolymeshAttr);
    attributeAffects(useTrianglesAttr, outputPolymeshAttr);
    attributeAffects(maxOutputVerticesAttr, outputPolymeshAttr);
    attributeAffects(useReverseTriOrderAttr, outputPolymeshAttr);
    attributeAffects(inputAttr, outputPolymeshAttr);
    attributeAffects(inputPolymeshAttr, outputPolymeshAttr);
    attributeAffects(inputComponentsAttr, outputPolymeshAttr);
    return MStatus::kSuccess;
}

MStatus DDConvexHullNode::processInputIndex(MPointArray &allPoints,
                                            MDataHandle &meshHndl,
                                            MDataHandle &compHndl)
{
    MObject curMesh = meshHndl.asMeshTransformed();
    MObject curComps = compHndl.data();
    
    // Need a mesh plug for comps to work
    if (curMesh.isNull())
    {
        return MStatus::kFailure;
    }
    
    // Create the function set for meshes and query for the points
    MFnMesh curMeshFn(curMesh);
    MPointArray meshPoints;
    curMeshFn.getPoints(meshPoints);
    
    // Assume we have a mesh here.  If the comps are null, then that
    // is fine, we're using the whole mesh
    if (curComps.isNull())
    {
        uint numPoints = meshPoints.length();
        for (uint i=0; i < numPoints; i++)
        {
            allPoints.append(meshPoints[i]);
        }
    }
    else
    {
        // Get the single-indexed components, convert to points, then
        // upload the values to the master points array (allPoints)
        MFnComponentListData compListFn(curComps);
        uint compListLen = compListFn.length();
        for (uint i=0; i < compListLen; i++)
        {
            MObject component = compListFn[i];
            
            // Make sure its a vert, face, edge, or UV
            if (!component.hasFn(MFn::kSingleIndexedComponent))
            {
                continue;
            }
            
            // Check if the component is complete.  If so, push all the points
            // for the mesh into the allPoints list
            MFnSingleIndexedComponent compFn(component);
            if (compFn.isComplete())
            {
                uint numPoints = meshPoints.length();
                for (uint j=0; j < numPoints; j++)
                {
                    allPoints.append(meshPoints[j]);
                }
                continue;
            }
            else
            {
                MIntArray vertIndices;
                MStatus stat = MStatus::kSuccess;
                stat = DDConvexHullUtils::componentToVertexIDs(vertIndices,
                                                               curMesh,
                                                               component);
                if (stat == MStatus::kNotImplemented)
                {
                    continue;
                }
                else if (stat == MStatus::kFailure)
                {
                    return stat;
                }
                
                // Lookup the points and append to the list
                uint vertIndicesLen = vertIndices.length();
                for (uint j=0; j < vertIndicesLen; j++)
                {
                    MPoint point;
                    curMeshFn.getPoint(vertIndices[j],point);
                    allPoints.append(point);
                }
            }
        }
    }
        
    return MStatus::kSuccess;
}

MStatus DDConvexHullNode::compute(const MPlug &plug, MDataBlock &data)
{
    MStatus stat;
    if (plug == outputPolymeshAttr)
    {
        // Create output MObject
        MFnMeshData outputDataCreator;
        MObject outputMesh = outputDataCreator.create(&stat);
        if (stat != MStatus::kSuccess)
        {
            return stat;
        }
        
        // Get the output data handle
        MDataHandle outputData = data.outputValue(outputPolymeshAttr, &stat);
        if (stat != MStatus::kSuccess)
        {
            return stat;
        }


        // Get the data from the input compound
        MPointArray allPoints;
        MArrayDataHandle inputData(data.inputArrayValue(inputAttr, &stat));
        uint elemCount = inputData.elementCount();
        if (elemCount == 0)
        {
            return MStatus::kInvalidParameter;
        }
        for (uint i=0; i < elemCount; i++)
        {
            MDataHandle curElem = inputData.inputValue();
            MDataHandle meshHndl = curElem.child(inputPolymeshAttr);
            MDataHandle compHndl = curElem.child(inputComponentsAttr);
            MStatus result = processInputIndex(allPoints, meshHndl, compHndl);
            if (result == MStatus::kFailure)
            {
                return result;
            }
            inputData.next();
        }
        
        // Ensure we have verts.  If not, display a warning, and return success
		uint pointCount = allPoints.length();
        if (pointCount < 8)
        {
            MGlobal::displayError("At least 8 unique points are required " \
                                  "to compute the hull.");
            return MStatus::kFailure;
        }
        
        // Create the hull options and get the values from the attributes
        DDConvexHullUtils::hullOpts hullOptions;
        
        // Skin Width
        MDataHandle useSkinWidthData(data.inputValue(useSkinWidthAttr, &stat));
        if (stat != MStatus::kSuccess)
        {
            return stat;
        }
        hullOptions.useSkinWidth = useSkinWidthData.asBool();
        
        MDataHandle skinWidthData(data.inputValue(skinWidthAttr, &stat));
        if (stat != MStatus::kSuccess)
        {
            return stat;
        }
        hullOptions.skinWidth = skinWidthData.asDouble();
        
        // Epsilon
        MDataHandle normalEpsilonData(data.inputValue(normalEpsilonAttr,&stat));
        if (stat != MStatus::kSuccess)
        {
            return stat;
        }
        hullOptions.normalEpsilon = normalEpsilonData.asDouble();
        
        // Force Triangles
        MDataHandle useTrianglesData(data.inputValue(useTrianglesAttr, &stat));
        if (stat != MStatus::kSuccess)
        {
            return stat;
        }
        hullOptions.forceTriangles = useTrianglesData.asBool();
        
        // Output verts
        MDataHandle maxOutputVertData(data.inputValue(maxOutputVerticesAttr,
                                                      &stat));
        if (stat != MStatus::kSuccess)
        {
            return stat;
        }
        hullOptions.maxOutputVertices = maxOutputVertData.asInt();
        
        // Reverse Triangles
        MDataHandle useRevTriData(data.inputValue(useReverseTriOrderAttr,
                                                  &stat));
        if (stat != MStatus::kSuccess)
        {
            return stat;
        }
        hullOptions.reverseTriangleOrder = useRevTriData.asBool();
        
        // Generate the hull
        stat = DDConvexHullUtils::generateMayaHull(outputMesh,
                                                   allPoints,
                                                   hullOptions);

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
//
//  DDConvexHullNode.cpp
//  DDConvexHull
//
//  Created by Jonathan Tilden on 12/30/12.
//  Copyright (c) 2012 Jonathan Tilden. All rights reserved.
//

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
    if (stat != MStatus::kSuccess)
    {
        return stat;
    }
    
    // Input Components
    MFnTypedAttribute inputComponentsAttrFn;
    inputComponentsAttr = inputComponentsAttrFn.create("inputComponents",
                                                "ics", MFnData::kComponentList);
    
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


MStatus DDConvexHullNode::compute(const MPlug &plug, MDataBlock &data)
{
    MStatus stat;
    if (plug == outputPolymeshAttr)
    {
        // Get the data from the input compound
        MPointArray allPoints;
        MArrayDataHandle inputData(data.inputArrayValue(inputAttr, &stat));
        uint elemCount = inputData.elementCount();
        for (uint i=0; i < elemCount; i++)
        {
            MDataHandle curElem = inputData.inputValue();
            MObject curMesh = (curElem.child(inputPolymeshAttr)).asMesh();
            MObject curComps = (curElem.child(inputComponentsAttr)).data();
            
            // Need a mesh plug for comps to work
            if (curMesh.isNull())
            {
                inputData.next();
                continue;
            }
            
            // Create the function set for meshes
            MFnMesh curMeshFn(curMesh);
            MPointArray meshPoints;
            curMeshFn.getPoints(meshPoints);
            
            // Assume we have a mesh here.  If the comps are null, then that
            // is fine, we're using the whole mesh
            if (curComps.isNull())
            {
                
                uint numPoints = meshPoints.length();
                for (uint j=0; j < numPoints; j++)
                {
                    allPoints.append(meshPoints[j]);
                }
            }
            else
            {
                // Get the single-indexed components, convert to points, then
                // upload the values to the master points array (allPoints)
                MFnComponentListData compListFn(curComps);
                uint compListLen = compListFn.length();
                for (uint j=0; j < compListLen; j++)
                {
                    MObject component = compListFn[j];
                    
                    // Make sure its a vert, face, edge, or UV
                    if (!component.hasFn(MFn::kSingleIndexedComponent))
                    {
                        continue;
                    }
                    
                    MFnSingleIndexedComponent compFn(component);
                    
                    // Possible early out... check to see if we have the
                    // complete data.  If so, just push all the mesh points
                    // into the list
                    if (compFn.isComplete())
                    {
                        uint numPoints = meshPoints.length();
                        for (uint j=0; j < numPoints; j++)
                        {
                            allPoints.append(meshPoints[j]);
                        }
                        continue;
                    }
                    
                    MIntArray elems;
                    compFn.getElements(elems);
                    uint elemLen = elems.length();
                    
                    // Ensure we're looking at vertices
                    uint compType = compFn.componentType();
                    if (compType == MFn::kMeshVertComponent)
                    {
                        for (uint k=0; k < elemLen; k++)
                        {
                            allPoints.append(meshPoints[elems[k]]);
                        }
                    }
                    else if (compType == MFn::kMeshEdgeComponent)
                    {
                        // Need to get the vertices from edge
                        MItMeshEdge edgeIt(curMesh, component);
                        while (!edgeIt.isDone())
                        {
                            allPoints.append(edgeIt.point(0));
                            allPoints.append(edgeIt.point(1));
                            edgeIt.next();
                        }
                    }
                    else if (compType == MFn::kMeshPolygonComponent)
                    {
                        // For some reason, I can't use MItMeshPolygon to find
                        // the points for these components (I need a DAGPath
                        // object if I want to use the component objects
                        // Instead, going to index into the MFnMesh
                        for (uint k=0; k < elemLen; k++)
                        {
                            MIntArray polyVerts;
                            curMeshFn.getPolygonVertices(elems[k], polyVerts);
                            uint polyVertsLen = polyVerts.length();
                            for (uint p=0; p < polyVertsLen; p++)
                            {
                                allPoints.append(meshPoints[p]);
                            }
                        }
                    }
                    else if (compType == MFn::kMeshFaceVertComponent)
                    {
                        // I think this is how you convert face to object
                        // relative vertices...
                        MIntArray faceCounts;
                        MIntArray faceVerts;
                        curMeshFn.getVertices(faceCounts, faceVerts);
                        for (uint k=0; k < elemLen; k++)
                        {
                            uint objectVertID = faceVerts[k];
                            allPoints.append(meshPoints[objectVertID]);
                        }
                    }
                    else
                    {
                        // Not supported
                        continue;
                    }
                    
                }
            }
            inputData.next();
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
        
        // Create output MObject
        MFnMeshData outputDataCreator;
        MObject outputMesh = outputDataCreator.create(&stat);
        if (stat != MStatus::kSuccess)
        {
            return stat;
        }

        // Generate the hull
        stat = DDConvexHullUtils::generateMayaHull(outputMesh,
                                                   allPoints,
                                                   hullOptions);

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
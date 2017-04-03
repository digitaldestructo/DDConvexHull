//
// DDConvexHullNode.h
// DDConvexHull
//
// Created by Jonathan Tilden on 12/30/12.
//
// MIT License
//
// Copyright (c) 2017 Jonathan Tilden

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in 
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef __DDConvexHull__DDConvexHullNode__
#define __DDConvexHull__DDConvexHullNode__

#include "StanHull/hull.h"
#include <maya/MPxNode.h>
#include <maya/MTypeId.h>
#include <maya/MFnMesh.h>

class DDConvexHullNode : public MPxNode
{
public:
    DDConvexHullNode();
    virtual ~DDConvexHullNode();

    // declare the compute() function
    virtual MStatus	 compute( const MPlug& plug, MDataBlock& data );
    
    // declare a function that returns an instance of this object
    static void*	 creator();
    
    // declare the initialize() function. Function
    // called when Node is initially registered.
    // Sets up the Node's attributes.
    static MStatus	 initialize();
    
    // Node id for this guy
    static MTypeId id;
    
    // Attribute object handles
    static MObject useSkinWidthAttr;
    static MObject skinWidthAttr;
    static MObject normalEpsilonAttr;
    static MObject useTrianglesAttr;
    static MObject maxOutputVerticesAttr;
    static MObject useReverseTriOrderAttr;
    static MObject inputAttr;
    static MObject inputComponentsAttr;
    static MObject inputPolymeshAttr;
    static MObject outputPolymeshAttr;

private:
    MStatus processInputIndex(MPointArray &allPoints,
                              MDataHandle &meshHndl,
                              MDataHandle &compHndl);
};


#endif /* defined(__DDConvexHull__DDConvexHullNode__) */

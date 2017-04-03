//
// DDConvexHullUtils.h
// DDConvexHull
//
// Created by Jonathan Tilden on 1/28/13.
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

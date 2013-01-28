//
//  DDConvexHullNode.h
//  DDConvexHull
//
//  Created by Jonathan Tilden on 12/30/12.
//  Copyright (c) 2012 Jonathan Tilden. All rights reserved.
//

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
    static MObject aInputPolymesh;
    static MObject aOutput;

private:
    HullError createConvexHull(MObject &outMesh, const MFnMesh &inMeshFn);
};


#endif /* defined(__DDConvexHull__DDConvexHullNode__) */

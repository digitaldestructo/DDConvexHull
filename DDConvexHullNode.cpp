//
//  DDConvexHullNode.cpp
//  DDConvexHull
//
//  Created by Jonathan Tilden on 12/30/12.
//  Copyright (c) 2012 Jonathan Tilden. All rights reserved.
//

#include "DDConvexHullNode.h"
#include <maya/MStatus.h>

DDConvexHullNode::DDConvexHullNode(){}
DDConvexHullNode::~DDConvexHullNode(){}

MTypeId DDConvexHullNode::id(0x7FFFF);

void* DDConvexHullNode::creator()
{
    return new DDConvexHullNode;
}


MStatus DDConvexHullNode::initialize()
{
    return MStatus::kSuccess;
}


MStatus DDConvexHullNode::compute(const MPlug &plug, MDataBlock &data)
{
    return MStatus::kSuccess;
}
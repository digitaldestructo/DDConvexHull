//
//  DDConvexHullCmd.h
//  DDConvexHull
//
//  Created by Jonathan Tilden on 1/1/13.
//  Copyright (c) 2013 Jonathan Tilden. All rights reserved.
//

#ifndef DDConvexHull_DDConvexHullCmd_h
#define DDConvexHull_DDConvexHullCmd_h

#include <maya/MArgList.h>
#include <maya/MPxCommand.h>

class DDConvexHullCmd : public MPxCommand
{
public:
    MStatus doIt( const MArgList& args );
    static void* creator();
private:
    MString dbl_to_string(double x);
    MString int_to_string(int x);
};



#endif

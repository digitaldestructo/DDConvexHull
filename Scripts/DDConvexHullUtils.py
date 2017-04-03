#
# DDConvexHullUtils.py
# DDConvexHull
#
# Created by Jonathan Tilden on 1/1/13.
#
# MIT License
#
# Copyright (c) 2017 Jonathan Tilden

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in 
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

__author__ = 'Jonathan Tilden'
__doc__ = """A collection of useful utilities for use with the DDConvexHull
plugin for Maya."""

import maya.cmds as cmds
import maya.mel  as mel
import re

_INDEX_RE = re.compile("\[(?P<index>[0-9])+\]")

class DDConvexHullException(RuntimeError):
    pass

def _determineMeshNode(node):
    """
    Given the specified node, return the associated mesh.  If no node can
    be determined, then a None type is returned.
    """
    # Scan for all node types that inherit from mesh or transform, just in
    # case there are weird custom nodes in the pipeline.
    types = cmds.nodeType(node, i=True)
    if ('transform' in types):
        shapes = cmds.listRelatives(node, s=True, f=True, type='mesh')
        if shapes is None or not len(shapes):
            return None

        # Workaround for a bug in 2011 where -ni flag for listRelatives didn't
        # work as advertised.
        shapes = cmds.ls(shapes, l=True, ni=True)
        if shapes is None or not len(shapes):
            return None

        # If here, take the first node
        return shapes[0]
    elif ('mesh' in types):
        return node
    else:
        return None
    return None

def _splitattr(attr):
    """
    Splits the attr string into obj/attr tuple.

    NOTE: Only the first '.' will be split and returned
          ex. _splitattr(ball.foo.bar) returns ('ball','foo.bar')
    """
    tokens = attr.split('.')
    if len(tokens) > 1:
        return (tokens[0], '.'.join(tokens[1:]))
    return (tokens[0], "")

def _attrindex(attr):
    """
    Parses the attr string and returns the first index it finds.
        ex. _attrindex('foo.input[0].ics') returns 0
    """
    result = _INDEX_RE.search(attr)
    if result is not None:
        return int(result.group('index'))

def _getFirstEmptyInputIndex(convexHullNode, limit=32):
    """
    Returns the first empty input index from the specified convex hull node.
    The limit flag simply specifies when to stop checking for an empty index
    and return -1.
    """
    multiIndices = cmds.getAttr('%s.input' % convexHullNode, multiIndices=True)
    if multiIndices is None:
        return 0
    indices = set(multiIndices)
    for idx in xrange(0, limit):
        if not idx in indices:
            return idx
    return -1

def _optimizeComponentList(mesh, componentList):
    """
    Given the specified mesh and its list of components, return a list that
    represents the most optimized representation of the components.
        ex. ['vtx[10]','vtx[11]'] optimizes to ['vtx[10:11]']
    """
    vertices = []
    faces    = []
    edges    = []
    for comp in componentList:
        if comp.startswith('f['):
            toAdd = faces
        elif comp.startswith('vtx['):
            toAdd = vertices
        elif comp.startswith('e['):
            toAdd = edges
        else:
            mel.eval('warning "Unsupported component type: %s"' % comp)
            continue
        toAdd.append('%s.%s' % (mesh, comp))
    newCompList = []
    if len(vertices):
        newComps = cmds.polyListComponentConversion(vertices, tv=True)
        if newComps is not None:
            newCompList.extend(newComps)
    if len(faces):
        newComps = cmds.polyListComponentConversion(faces, tf=True)
        if newComps is not None:
            newCompList.extend(newComps)
    if len(edges):
        newComps = cmds.polyListComponentConversion(edges, te=True)
        if newComps is not None:
            newCompList.extend(newComps)
    return [x.split('.')[-1] for x in newCompList]


def setComponents(componentList, convexHullNode, index,
                                 preserveExisting=True, optimize=True):
    """
    Given a list of components (in the form of ['f[0]','vtx[23:67]', ...]), set
    the specified, indexed componentList attribute for the convex hull.

    If preserveExisting is True, the any components that are already on the
    attribute will be preserved.  Otherwise, they will be overwritten.

    If optimize is set to True, convert the list of components into the more
    compact form that Maya typically expects.
    """
    inputAttr = '%s.input[%d]' % (convexHullNode, index)
    if preserveExisting:
        comps = cmds.getAttr('%s.ics' % inputAttr)
        if comps is not None:
            componentList.extend(comps)
    if optimize:
        attr = '%s.input'
        mesh = cmds.connectionInfo('%s.inputPolymesh' % inputAttr,
                                   sfd=True, dfs=False)
        if mesh is None or mesh == '':
            mel.eval('warning "Unable to determine connected to ' \
                     '%s.inputPolymesh' % inputAttr)
        else:
            meshName, plug = _splitattr(mesh)
            componentList = _optimizeComponentList(meshName, componentList)

    # Flatten the list to a space-delimited string
    numComponents = len(componentList)
    print componentList
    cmds.setAttr('%s.ics' % inputAttr,
                 numComponents, type='componentList', *componentList)


def getConnectedMeshes(convexHullNode, reverseMapping=False):
    """
    Returns a dictionary mapping the array index to the mesh connected to the
    input[x].inputPolymesh attribute.
        ex. {0 : 'myMeshShape', 2 : 'myBallShape'}
    If reverse mapping is True, the connected mesh maps to the indices.
        ex. {'myMeshShape' : [0], {'myBallShape' : [2,3]}
    """
    inputAttr = '%s.input' % convexHullNode
    connections = cmds.listConnections(inputAttr, sh=True, p=True,
                                                  type='mesh', c=True)
    if connections is None or not len(connections):
        return {}

    # The convex node is always on the left, connection on right.
    mapping = {}
    for idx in xrange(0, len(connections), 2):
        if connections[idx].find('.inputPolymesh') == -1:
            continue
        index = _attrindex(connections[idx])
        mesh  = _splitattr(connections[idx+1])
        mesh  = cmds.ls(mesh, l=True)[0]
        if reverseMapping:
            if not mapping.has_key(mesh):
                mapping[mesh] = []
            mapping[mesh].append(index)
        else:
            mapping[index] = mesh
    return mapping

def connectMesh(convexHullNode, meshNode, componentList=[]):
    """
    Connects the specified mesh and components to the convex hull node.

    NOTE: New connections are always made
    """
    # Find the first empty index
    idx = _getFirstEmptyInputIndex(convexHullNode)
    cmds.connectAttr(('%s.worldMesh' % meshNode),
                     ('%s.input[%d].inputPolymesh' % (convexHullNode,idx)),
                     force=True)

    # If there are components, set them
    numComponents = len(componentList)
    if numComponents:
        setComponents(componentList, convexHullNode, idx)

def addObjects(convexHullNode, objects=[]):
    """
    Adds the specified objects to the convex hull node.  If no objects are
    specified, then the user's selection is used.
    """
    if not len(objects):
        objects = cmds.ls(sl=True, l=True)

    # Check again.  If no objects specified, generate an error
    if not len(objects):
        raise DDConvexHullException, 'No objects were specified to add to ' \
                                     'the convex hull'

    # Check the node exists
    if not cmds.objExists(convexHullNode):
        raise DDConvexHullException, 'The specified convex hull %s does not ' \
                                     'exist.' % convexHullNode

    # Generate a mapping of connected meshes.  Get a reverse mapping so its
    # easy to test if the mesh specified should be added or updated.
    connectedMeshes = getConnectedMeshes(convexHullNode, reverseMapping=True)

    # Iterate through the objects and build a map of node to components
    compMap = {}
    for obj in objects:
        node, comp = _splitattr(obj)
        mesh = _determineMeshNode(node)
        if not compMap.has_key(mesh):
            compMap[mesh] = {'update':mesh in connectedMeshes, 'components':[]}
        if comp != '':
            compMap[mesh]['components'].append(comp)
    for mesh in compMap:
        if compMap[mesh]['update']:
            setComponents(compMap[mesh]['components'],
                          convexHullNode, connectedMeshes[mesh][0])
        else:
            connectMesh(convexHullNode, mesh, compMap[mesh]['components'])

def createHull(hullname='', meshname='', objects=[], createTransform=True,
               parent=None):
    """
    Creates a new hull and output mesh with the specified connected objects.
    If no objects are specified, the user's selection is used.  If
    createTransform is True, then the meshname will be the name of the transform
    and the mesh will have 'shape' appended to the end.

    Returns a tuple in the form of (hullNode, meshNode, xformNode)
    """
    selection = cmds.ls(sl=True, l=True)

    hullargs = {}
    if hullname != '':
        hullargs['n'] = hullname
    hullNode = cmds.createNode('DDConvexHull', **hullargs)

    # Create a transform if specified.  If so, set the mesh name to the
    # name of the transform
    xformNode = None
    if createTransform:
        xformArgs  = {}
        if meshname == '':
            meshname = 'polySurface#'
        xformArgs['n'] = meshname
        if parent is not None:
            xformArgs['p'] = parent
        xformNode = cmds.createNode('transform', **xformArgs)

    meshargs = {}
    if meshname != '':
        meshargs['n'] = meshname
    if createTransform and xformNode is not None:
        meshargs['n'] = '%sShape' % xformNode
        meshargs['p'] = xformNode
    elif parent is not None:
        meshargs['p'] = parent
    meshNode = cmds.createNode('mesh', **meshargs)

    # Connect the nodes
    cmds.connectAttr('%s.output' % hullNode, '%s.inMesh' % meshNode)

    # If there are no objects specified, and no selection, handle it here
    # so it doesn't create an exception when passed to addObjects
    if not len(objects) and (selection is None or not len(selection)):
        mel.eval('warning "No objects specified to add to hull"')
    else:
        objects = selection
        addObjects(hullNode, objects=objects)
    return (hullNode, meshNode, xformNode)


DDConvexHull
============

A Convex Hull plug-in for Maya based on StanHull.  The hard work credit goes to Stan Melax and John Ratcliff for providing the open source code that actually does the convex hull.

The code should compile without issue on Windows, as it was ported from Windows to compile on OSX.


Example maya cmds code

```
import maya.cmds as cmds
def setComponents( sel, hullNode: str):
    """
    Set the component list on the DDConvexHull object after if we have one

    Args:
        sel (list): list of faces or verts of an object
        hullNode (string): name of the node to set the components on
    """
    coms = [str(i.split(".")[1]) for i in sel]
    cmds.setAttr(
        f"{hullNode}.input[0].inputComponents", len(coms), *coms, type="componentList"
    )


def create_hull():
    enabled = cmds.pluginInfo("DDConvexHull", q=True, loaded=True)
    if not enabled:
        cmds.loadPlugin("DDConvexHull")
    orig_sel = None
    sel = cmds.ls(sl=True)
    if not sel:
        return
    transform = cmds.ls(sl=True, type="transform")[0]

    if ".vtx[" in sel[0] or ".f[" in sel[0]:
        # We have a vert or face selection, we will need to update the components list later
        orig_sel = sel
        sel = [sel[0].split(".")[0]]

    if not cmds.objectType(sel[0], i="transform"):
        oldSel = sel
        sel = cmds.listRelatives(sel[0], p=True)[0]
    else:
        oldSel = sel
        sel = sel[0]
    # check if the hull name exists already
    hull_name = f"Hull_{sel}"

    inputMesh = self.get_shape(sel)
    # create the actual DDConvexHull
    hullNode = cmds.ls(cmds.createNode("DDConvexHull"))[0]
    cmds.setAttr(f"{hullNode}.maxVertices", vert_count)

    # create the shape node
    outputNode = cmds.createNode("mesh", n=f"Hull_{sel}Shape")

    # connect the ddcvx to the mesh in/output
    cmds.connectAttr(f"{inputMesh}.outMesh", f"{hullNode}.input[0].inputPolymesh")
    cmds.connectAttr(f"{hullNode}.output", f"{outputNode}.inMesh")
    cmds.parent(cmds.listRelatives(outputNode, p=True), sel, relative=True)
    if orig_sel:
        setComponents(orig_sel, hullNode)
        
# Used by doing a selection of either an object, faces or verts
create_hull()
```

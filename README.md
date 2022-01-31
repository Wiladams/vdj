# vdj
 Video DJ Experimentation

 Design experiment
 -------------
 How far can I go with a 'pull model' in a UI library.  All the video effects are based on essentially doing a scan line at a time, and asking the effects stack what the color
 of a pixel should be at any given location.

 In addition, everything at that level is based on normalized coordinates.  The range is [0..1].  So, this allows effects to be independent of size of pixels, but limits things as we don't know how to draw better for scaling.

 The other challenge is depth or complexity of the scene.  The more blocks there are, the more child blocks there are to search through when trying to figure out which block should be asked for its color value.

 Window System
 -------------
 There is a rudimentary 'window' system that is also done in normalized space.  A 'SampledWindow' can have child windows, and they stack up in reverse order.  So, addChild() will maintain a FILO queue, which is akin to the painter's algorithm when it comes time to ask for a color.  That last SampledWindow to be added is the nearest to the screen, so will have first crack at delivering a color value when asked.


Challenges to be solved
-----------------------
A great many effects can be done simply using this method.  They break down into a few categories
    * Simple block sizing - You can create a large number of sub-sections of an incoming video stream, and arrange those blocks in any way in the output screen.  Push, pull, wipe, expand, fly-out, etc, can all be achieved by simple geometry manipulations.
    * Pixel Effects - These are the effects that operate on a single pixel at a time.  Color tinting is the easiest, but altering transparency, cross fading, etc, can all be done with this method.
    * More complex effects that require geometric transformations are also achievable, but not studied yet

Speed
-----
These techniques are rather brute force.  traversing all pixels per frame, traversing down child lists, etc.  It can be quite wasteful.  There are two easy optimizations that can be employed.  
    * Multi-threading - Since most of the effects do not depend on the value of an underyling pixel within the same frame, it's ok to do things in parallel.  So, using multiple threads can speed things up significantly.
    * Better structure for holding child boundaries.  Right there they're just in a simple list.  This causes large searches for intersection.  Holding the children in an octree structure should make the search space much smaller and faster.

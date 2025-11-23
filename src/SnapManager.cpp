#include "SnapManager.h"
#include "TObjectCollection.h"
#include <AIS_ListOfInteractive.hxx>
#include <AIS_ListIteratorOfListOfInteractive.hxx>
#include <AIS_Shape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Face.hxx>
#include <TopExp.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <Precision.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <IntAna_IntConicQuad.hxx>
#include <gp_Lin.hxx>
#include <gp_Pln.hxx>
#include <QDebug>
#include <cmath>

SnapManager::SnapManager()
    : m_enabledSnaps(Endpoint | Midpoint | Vertex | Center)
    , m_snapTolerancePixels(25.0)
{
}

void SnapManager::enableSnap(SnapType type, bool enabled)
{
    if (enabled) {
        m_enabledSnaps |= type;
    } else {
        m_enabledSnaps &= ~type;
    }
}

SnapManager::SnapPoint SnapManager::findSnapPoint(int screenX, int screenY,
                                                  const Handle(AIS_InteractiveContext)& context,
                                                  const Handle(V3d_View)& view)
{
    try {
        if (view.IsNull()) {
            SnapPoint emptySnap;
            return emptySnap;
        }
        
        // Convert mouse screen position to 3D point on work plane
        // Use the same ray-plane intersection as CADController
        Standard_Real Xv, Yv, Zv, Vx, Vy, Vz;
        view->ConvertWithProj(screenX, screenY, Xv, Yv, Zv, Vx, Vy, Vz);
        
        // Create ray from eye through mouse position
        gp_Pnt rayStart(Xv, Yv, Zv);
        gp_Dir rayDir(Vx, Vy, Vz);
        
        // Work plane (Z=0)
        gp_Pln workPlane(gp::Origin(), gp::DZ());
        gp_Dir planeNormal = workPlane.Axis().Direction();
        gp_Pnt planeOrigin = workPlane.Location();
        
        // Ray-plane intersection formula: t = ((planeOrigin - rayStart) · planeNormal) / (rayDir · planeNormal)
        gp_Vec toPlane(rayStart, planeOrigin);
        double denominator = rayDir.Dot(planeNormal);
        
        if (std::abs(denominator) < 1e-10) {
            SnapPoint emptySnap;
            return emptySnap;
        }
        
        double t = toPlane.Dot(planeNormal) / denominator;
        gp_Pnt cursor3D = rayStart.XYZ() + t * rayDir.XYZ();
        
        // Calculate world-space snap tolerance
        // Convert snap tolerance from pixels to world units
        // Get a point slightly offset in screen space and convert to world
        Standard_Real Xv2, Yv2, Zv2, Vx2, Vy2, Vz2;
        view->ConvertWithProj(screenX + m_snapTolerancePixels, screenY, Xv2, Yv2, Zv2, Vx2, Vy2, Vz2);
        
        gp_Pnt rayStart2(Xv2, Yv2, Zv2);
        gp_Dir rayDir2(Vx2, Vy2, Vz2);
        gp_Vec toPlane2(rayStart2, planeOrigin);
        double denominator2 = rayDir2.Dot(planeNormal);
        
        double worldTolerance = m_snapTolerancePixels * 10.0; // Default fallback
        if (std::abs(denominator2) > 1e-10) {
            double t2 = toPlane2.Dot(planeNormal) / denominator2;
            gp_Pnt cursor3D_offset = rayStart2.XYZ() + t2 * rayDir2.XYZ();
            worldTolerance = cursor3D.Distance(cursor3D_offset);
        }
        
        // Get all visible shapes
        QList<TopoDS_Shape> shapes = getVisibleShapes(context);
        
        qDebug() << "SnapManager: Found" << shapes.size() << "visible shapes";
        
        // Find snap points from geometry
        QList<SnapPoint> candidates;
        
        for (const TopoDS_Shape& shape : shapes) {
            if (shape.IsNull()) continue;
            
            try {
                // Check if this is a beam-like object (elongated in one dimension)
                Bnd_Box bbox;
                BRepBndLib::Add(shape, bbox);
                if (!bbox.IsVoid()) {
                    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
                    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
                    
                    double dx = xmax - xmin;
                    double dy = ymax - ymin;
                    double dz = zmax - zmin;
                    
                    double maxDim = std::max({dx, dy, dz});
                    double minDim = std::min({dx, dy, dz});
                    
                    // Determine if this is a beam/column (elongated in ONE dimension)
                    // or a slab (flat in one dimension but extended in TWO dimensions)
                    bool isBeamLike = false;
                    if (maxDim > minDim * 5.0) {
                        // Check if it's elongated in one direction (beam/column)
                        // A beam has ONE large dimension and TWO small dimensions
                        // A slab has TWO large dimensions and ONE small dimension
                        std::vector<double> dims = {dx, dy, dz};
                        std::sort(dims.begin(), dims.end());
                        // dims[0] = smallest, dims[1] = middle, dims[2] = largest
                        
                        // If middle dimension is also small (similar to smallest), it's a beam
                        // If middle dimension is large (similar to largest), it's a slab
                        if (dims[1] / dims[0] > 3.0) {
                            // Middle is much larger than smallest -> Slab (two large dims)
                            isBeamLike = false;
                        } else {
                            // Middle is similar to smallest -> Beam (one large dim)
                            isBeamLike = true;
                        }
                    }
                    
                    // If beam/column, use centerline and flange endpoints
                    if (isBeamLike) {
                        gp_Pnt centerStart, centerEnd;
                        
                        // Determine beam orientation and calculate points
                        if (dx == maxDim) {
                            // Horizontal beam along X
                            centerStart = gp_Pnt(xmin, (ymin+ymax)/2, (zmin+zmax)/2);
                            centerEnd = gp_Pnt(xmax, (ymin+ymax)/2, (zmin+zmax)/2);
                            
                            // Add centerline endpoints
                            candidates.append(SnapPoint(centerStart, Endpoint, "Beam Center Start", cursor3D.Distance(centerStart)));
                            candidates.append(SnapPoint(centerEnd, Endpoint, "Beam Center End", cursor3D.Distance(centerEnd)));
                            
                            // Add BOTH top and bottom flange endpoints
                            // The closest one will be selected based on cursor position
                            gp_Pnt topStart(xmin, (ymin+ymax)/2, zmax);
                            gp_Pnt topEnd(xmax, (ymin+ymax)/2, zmax);
                            gp_Pnt bottomStart(xmin, (ymin+ymax)/2, zmin);
                            gp_Pnt bottomEnd(xmax, (ymin+ymax)/2, zmin);
                            
                            candidates.append(SnapPoint(topStart, Endpoint, "Beam Top Start", cursor3D.Distance(topStart)));
                            candidates.append(SnapPoint(topEnd, Endpoint, "Beam Top End", cursor3D.Distance(topEnd)));
                            candidates.append(SnapPoint(bottomStart, Endpoint, "Beam Bottom Start", cursor3D.Distance(bottomStart)));
                            candidates.append(SnapPoint(bottomEnd, Endpoint, "Beam Bottom End", cursor3D.Distance(bottomEnd)));
                            
                        } else if (dy == maxDim) {
                            // Horizontal beam along Y
                            centerStart = gp_Pnt((xmin+xmax)/2, ymin, (zmin+zmax)/2);
                            centerEnd = gp_Pnt((xmin+xmax)/2, ymax, (zmin+zmax)/2);
                            
                            candidates.append(SnapPoint(centerStart, Endpoint, "Beam Center Start", cursor3D.Distance(centerStart)));
                            candidates.append(SnapPoint(centerEnd, Endpoint, "Beam Center End", cursor3D.Distance(centerEnd)));
                            
                            gp_Pnt topStart((xmin+xmax)/2, ymin, zmax);
                            gp_Pnt topEnd((xmin+xmax)/2, ymax, zmax);
                            gp_Pnt bottomStart((xmin+xmax)/2, ymin, zmin);
                            gp_Pnt bottomEnd((xmin+xmax)/2, ymax, zmin);
                            
                            candidates.append(SnapPoint(topStart, Endpoint, "Beam Top Start", cursor3D.Distance(topStart)));
                            candidates.append(SnapPoint(topEnd, Endpoint, "Beam Top End", cursor3D.Distance(topEnd)));
                            candidates.append(SnapPoint(bottomStart, Endpoint, "Beam Bottom Start", cursor3D.Distance(bottomStart)));
                            candidates.append(SnapPoint(bottomEnd, Endpoint, "Beam Bottom End", cursor3D.Distance(bottomEnd)));
                            
                        } else {
                            // Vertical column along Z
                            centerStart = gp_Pnt((xmin+xmax)/2, (ymin+ymax)/2, zmin);
                            centerEnd = gp_Pnt((xmin+xmax)/2, (ymin+ymax)/2, zmax);
                            
                            candidates.append(SnapPoint(centerStart, Endpoint, "Column Bottom", cursor3D.Distance(centerStart)));
                            candidates.append(SnapPoint(centerEnd, Endpoint, "Column Top", cursor3D.Distance(centerEnd)));
                        }
                        
                        if (isSnapEnabled(Midpoint)) {
                            gp_Pnt mid((centerStart.X()+centerEnd.X())/2, 
                                      (centerStart.Y()+centerEnd.Y())/2, 
                                      (centerStart.Z()+centerEnd.Z())/2);
                            candidates.append(SnapPoint(mid, Midpoint, "Beam Midpoint", cursor3D.Distance(mid)));
                        }
                        
                        continue;
                    }
                }
                
                // For non-beam shapes, find all snap points
                if (isSnapEnabled(Endpoint)) {
                    findEndpoints(shape, cursor3D, candidates);
                }
                if (isSnapEnabled(Midpoint)) {
                    findMidpoints(shape, cursor3D, candidates);
                }
                if (isSnapEnabled(Vertex)) {
                    findVertices(shape, cursor3D, candidates);
                }
                if (isSnapEnabled(Center)) {
                    findCenter(shape, cursor3D, candidates);
                }
                if (isSnapEnabled(Nearest)) {
                    findNearest(shape, cursor3D, candidates);
                }
                
                // Filter by world tolerance
                for (int i = candidates.size() - 1; i >= 0; --i) {
                    if (candidates[i].distance > worldTolerance) {
                        candidates.removeAt(i);
                    }
                }
            } catch (...) {
                continue;
            }
        }
        
        // Find closest in 3D world space with priority
        // Priority: Endpoint > Midpoint > Vertex > Center > Nearest
        SnapPoint bestSnap;
        bestSnap.distance = worldTolerance;
        
        qDebug() << "SnapManager: Found" << candidates.size() << "candidates, tolerance:" << worldTolerance;
        
        // First pass: High-priority snaps
        for (const SnapPoint& candidate : candidates) {
            if (candidate.type == Endpoint || candidate.type == Midpoint || candidate.type == Vertex) {
                if (candidate.distance < bestSnap.distance) {
                    bestSnap = candidate;
                }
            }
        }
        
        // Second pass: Low-priority snaps (only if no high-priority found)
        if (bestSnap.type == None) {
            for (const SnapPoint& candidate : candidates) {
                if (candidate.type == Center || candidate.type == Nearest) {
                    if (candidate.distance < bestSnap.distance) {
                        bestSnap = candidate;
                    }
                }
            }
        }
        
        // Add debug info
        if (bestSnap.type != None) {
            bestSnap.description = QString("%1 [%.0fmm/%2]")
                .arg(bestSnap.description)
                .arg(bestSnap.distance)
                .arg(candidates.size());
        }
        
        qDebug() << "SnapManager: Returning snap type:" << bestSnap.type;
        
        return bestSnap;
    } catch (...) {
        qDebug() << "Exception in findSnapPoint (catch all)";
        SnapPoint emptySnap;
        return emptySnap;
    }
}

SnapManager::SnapPoint SnapManager::findSnapPointFromObjects(int screenX, int screenY,
                                                              TObjectCollection* collection,
                                                              const Handle(V3d_View)& view)
{
    try {
        if (view.IsNull() || !collection) {
            SnapPoint emptySnap;
            return emptySnap;
        }
        
        // Query all objects for their snap points
        NCollection_Sequence<Handle(TGraphicObject)> objects = collection->GetAllObjects();
        
        // Find the closest snap point in SCREEN space, not world space
        SnapPoint bestSnap;
        bestSnap.type = None;
        bestSnap.distance = 1e10;
        double minScreenDist = 1e10;
        
        for (int i = 1; i <= objects.Size(); i++) {
            Handle(TGraphicObject) obj = objects.Value(i);
            if (obj.IsNull()) continue;
            
            try {
                // Get all snap points from the object
                QList<TGraphicObject::SnapPoint> snapPoints = obj->GetSnapPoints();
                
                // Check each snap point's distance in screen space
                for (const TGraphicObject::SnapPoint& snap : snapPoints) {
                    // Convert 3D snap point to screen coordinates
                    Standard_Integer snapScreenX, snapScreenY;
                    view->Convert(snap.point.X(), snap.point.Y(), snap.point.Z(), 
                                 snapScreenX, snapScreenY);
                    
                    // Calculate screen-space distance
                    double dx = snapScreenX - screenX;
                    double dy = snapScreenY - screenY;
                    double screenDist = std::sqrt(dx*dx + dy*dy);
                    
                    // Check if within tolerance and closer than current best
                    if (screenDist < m_snapTolerancePixels && screenDist < minScreenDist) {
                        minScreenDist = screenDist;
                        bestSnap.point = snap.point;
                        bestSnap.distance = screenDist;
                        bestSnap.description = snap.description;
                        
                        // Map object snap type to SnapManager type
                        if (snap.type & 0x01) {
                            bestSnap.type = Endpoint;
                        } else if (snap.type & 0x02) {
                            bestSnap.type = Midpoint;
                        } else if (snap.type & 0x04) {
                            bestSnap.type = Center;
                        } else {
                            bestSnap.type = Vertex;
                        }
                    }
                }
            } catch (...) {
                continue;
            }
        }
        
        return bestSnap;
    } catch (...) {
        qDebug() << "Exception in findSnapPointFromObjects";
        SnapPoint emptySnap;
        return emptySnap;
    }
}

QList<SnapManager::SnapPoint> SnapManager::findAllSnapPoints(const gp_Pnt& cursorPoint,
                                                              const Handle(AIS_InteractiveContext)& context,
                                                              const Handle(V3d_View)& view)
{
    QList<SnapPoint> result;
    
    try {
        if (view.IsNull()) {
            return result;
        }
        
        QList<SnapPoint> candidates;
        
        // Convert cursor point to screen coordinates
        Standard_Integer cursorX, cursorY;
        view->Convert(cursorPoint.X(), cursorPoint.Y(), cursorPoint.Z(), cursorX, cursorY);
        
        // Get all visible shapes
        QList<TopoDS_Shape> shapes = getVisibleShapes(context);
        
        // Find snap points for each shape
        for (const TopoDS_Shape& shape : shapes) {
            if (shape.IsNull()) continue;
            
            try {
                if (isSnapEnabled(Endpoint)) {
                    findEndpoints(shape, cursorPoint, candidates);
                }
                if (isSnapEnabled(Midpoint)) {
                    findMidpoints(shape, cursorPoint, candidates);
                }
                if (isSnapEnabled(Vertex)) {
                    findVertices(shape, cursorPoint, candidates);
                }
                if (isSnapEnabled(Center)) {
                    findCenter(shape, cursorPoint, candidates);
                }
                if (isSnapEnabled(Nearest)) {
                    findNearest(shape, cursorPoint, candidates);
                }
            } catch (...) {
                // Skip this shape if any error occurs
                continue;
            }
        }
        
        // Return ALL candidates within screen-space tolerance
        for (const SnapPoint& candidate : candidates) {
            // Convert candidate point to screen coordinates
            Standard_Integer candX, candY;
            view->Convert(candidate.point.X(), candidate.point.Y(), candidate.point.Z(), candX, candY);
            
            // Calculate screen-space distance in pixels
            double dx = candX - cursorX;
            double dy = candY - cursorY;
            double screenDist = std::sqrt(dx*dx + dy*dy);
            
            if (screenDist < m_snapTolerancePixels) {
                result.append(candidate);
            }
        }
        
        return result;
    } catch (...) {
        return result;
    }
}

void SnapManager::findEndpoints(const TopoDS_Shape& shape, const gp_Pnt& cursor, QList<SnapPoint>& candidates)
{
    TopExp_Explorer edgeExp(shape, TopAbs_EDGE);
    
    while (edgeExp.More()) {
        TopoDS_Edge edge = TopoDS::Edge(edgeExp.Current());
        
        TopoDS_Vertex V1, V2;
        TopExp::Vertices(edge, V1, V2);
        
        if (!V1.IsNull()) {
            gp_Pnt p1 = BRep_Tool::Pnt(V1);
            double dist = cursor.Distance(p1);
            candidates.append(SnapPoint(p1, Endpoint, "Endpoint", dist));
        }
        
        if (!V2.IsNull() && !V1.IsSame(V2)) {
            gp_Pnt p2 = BRep_Tool::Pnt(V2);
            double dist = cursor.Distance(p2);
            candidates.append(SnapPoint(p2, Endpoint, "Endpoint", dist));
        }
        
        edgeExp.Next();
    }
}

void SnapManager::findMidpoints(const TopoDS_Shape& shape, const gp_Pnt& cursor, QList<SnapPoint>& candidates)
{
    TopExp_Explorer edgeExp(shape, TopAbs_EDGE);
    
    while (edgeExp.More()) {
        TopoDS_Edge edge = TopoDS::Edge(edgeExp.Current());
        
        Standard_Real first, last;
        Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);
        
        if (!curve.IsNull()) {
            Standard_Real mid = (first + last) / 2.0;
            gp_Pnt midPoint = curve->Value(mid);
            double dist = cursor.Distance(midPoint);
            candidates.append(SnapPoint(midPoint, Midpoint, "Midpoint", dist));
        }
        
        edgeExp.Next();
    }
}

void SnapManager::findVertices(const TopoDS_Shape& shape, const gp_Pnt& cursor, QList<SnapPoint>& candidates)
{
    TopExp_Explorer vertexExp(shape, TopAbs_VERTEX);
    
    while (vertexExp.More()) {
        TopoDS_Vertex vertex = TopoDS::Vertex(vertexExp.Current());
        gp_Pnt p = BRep_Tool::Pnt(vertex);
        double dist = cursor.Distance(p);
        candidates.append(SnapPoint(p, Vertex, "Vertex", dist));
        
        vertexExp.Next();
    }
}

void SnapManager::findCenter(const TopoDS_Shape& shape, const gp_Pnt& cursor, QList<SnapPoint>& candidates)
{
    if (!shape.IsNull()) {
        gp_Pnt center = calculateCenter(shape);
        double dist = cursor.Distance(center);
        candidates.append(SnapPoint(center, Center, "Center", dist));
    }
}

void SnapManager::findNearest(const TopoDS_Shape& shape, const gp_Pnt& cursor, QList<SnapPoint>& candidates)
{
    TopExp_Explorer edgeExp(shape, TopAbs_EDGE);
    
    while (edgeExp.More()) {
        TopoDS_Edge edge = TopoDS::Edge(edgeExp.Current());
        
        BRepAdaptor_Curve curve(edge);
        GeomAPI_ProjectPointOnCurve projector(cursor, curve.Curve().Curve());
        
        if (projector.NbPoints() > 0) {
            gp_Pnt nearestPoint = projector.NearestPoint();
            double dist = cursor.Distance(nearestPoint);
            candidates.append(SnapPoint(nearestPoint, Nearest, "Nearest", dist));
        }
        
        edgeExp.Next();
    }
}

gp_Pnt SnapManager::calculateCenter(const TopoDS_Shape& shape)
{
    Bnd_Box boundingBox;
    BRepBndLib::Add(shape, boundingBox);
    
    if (!boundingBox.IsVoid()) {
        Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
        boundingBox.Get(xMin, yMin, zMin, xMax, yMax, zMax);
        
        return gp_Pnt(
            (xMin + xMax) / 2.0,
            (yMin + yMax) / 2.0,
            (zMin + zMax) / 2.0
        );
    }
    
    return gp_Pnt(0, 0, 0);
}

QList<TopoDS_Shape> SnapManager::getVisibleShapes(const Handle(AIS_InteractiveContext)& context)
{
    QList<TopoDS_Shape> shapes;
    
    AIS_ListOfInteractive displayedObjects;
    context->DisplayedObjects(displayedObjects);
    
    AIS_ListIteratorOfListOfInteractive it(displayedObjects);
    for (; it.More(); it.Next()) {
        Handle(AIS_InteractiveObject) obj = it.Value();
        Handle(AIS_Shape) aisShape = Handle(AIS_Shape)::DownCast(obj);
        
        if (!aisShape.IsNull()) {
            TopoDS_Shape shape = aisShape->Shape();
            if (!shape.IsNull()) {
                shapes.append(shape);
            }
        }
    }
    
    return shapes;
}

bool SnapManager::isEdgeOnHorizontalFace(const TopoDS_Edge& edge, const TopoDS_Shape& shape)
{
    try {
        // First check if the entire shape is a large flat box (typical slab)
        Bnd_Box shapeBox;
        BRepBndLib::Add(shape, shapeBox);
        if (!shapeBox.IsVoid()) {
            Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
            shapeBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
            double width = xmax - xmin;
            double depth = ymax - ymin;
            double height = zmax - zmin;
            
            // If shape is large in X-Y but thin in Z, it's likely a slab
            double xyArea = width * depth;
            if (xyArea > 1000000.0 && height < std::min(width, depth) * 0.2) {
                return true;  // Skip all edges of slab-like shapes
            }
        }
        
        // Build edge-to-face map
        TopTools_IndexedDataMapOfShapeListOfShape edgeFaceMap;
        TopExp::MapShapesAndAncestors(shape, TopAbs_EDGE, TopAbs_FACE, edgeFaceMap);
        
        // Check if this edge has any parent faces
        if (!edgeFaceMap.Contains(edge)) {
            return false;  // Edge not on any face, keep it
        }
        
        const TopTools_ListOfShape& faces = edgeFaceMap.FindFromKey(edge);
        TopTools_ListIteratorOfListOfShape faceIt(faces);
        
        for (; faceIt.More(); faceIt.Next()) {
            TopoDS_Face face = TopoDS::Face(faceIt.Value());
            
            // Check if face is planar and horizontal
            BRepAdaptor_Surface surface(face);
            if (surface.GetType() == GeomAbs_Plane) {
                gp_Pln plane = surface.Plane();
                gp_Dir normal = plane.Axis().Direction();
                
                // Check if normal is nearly vertical (face is horizontal)
                double dotZ = std::abs(normal.Z());
                if (dotZ > 0.9) {  // Face is mostly horizontal
                    // Check face area - large faces are likely slabs
                    Bnd_Box box;
                    BRepBndLib::Add(face, box);
                    if (!box.IsVoid()) {
                        Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
                        box.Get(xmin, ymin, zmin, xmax, ymax, zmax);
                        double width = xmax - xmin;
                        double height = ymax - ymin;
                        double area = width * height;
                        
                        // If face area > 1 m² (1,000,000 mm²), treat as slab
                        if (area > 1000000.0) {
                            return true;  // Skip this edge
                        }
                    }
                }
            }
        }
        
        return false;  // Keep this edge
    } catch (...) {
        return false;  // On error, keep the edge
    }
}

bool SnapManager::isPointVisible(const gp_Pnt& point, 
                                 const Handle(AIS_InteractiveContext)& context,
                                 const Handle(V3d_View)& view)
{
    try {
        if (view.IsNull() || context.IsNull()) {
            return true;  // Assume visible if we can't check
        }
        
        // Get view eye position and direction
        Standard_Real eyeX, eyeY, eyeZ;
        Standard_Real projX, projY, projZ;
        view->Eye(eyeX, eyeY, eyeZ);
        view->Proj(projX, projY, projZ);
        
        gp_Pnt eyePoint(eyeX, eyeY, eyeZ);
        gp_Dir viewDir(projX, projY, projZ);
        
        // For orthographic view, eye is at infinity along view direction
        // So we cast ray from point backwards along view direction
        gp_Pnt rayStart = point.Translated(gp_Vec(viewDir.Reversed()) * 0.1);  // Start slightly behind point
        gp_Dir rayDir = viewDir;
        
        // Check if any shape occludes this point from the view
        // We do a simple depth check: convert point to screen space and check Z-depth
        Standard_Integer screenX, screenY;
        view->Convert(point.X(), point.Y(), point.Z(), screenX, screenY);
        
        // Convert screen coords back to get depth
        Standard_Real worldX, worldY, worldZ;
        view->Convert(screenX, screenY, worldX, worldY, worldZ);
        
        // Get view plane at the point
        gp_Pnt projectedPoint(worldX, worldY, worldZ);
        double pointDepth = eyePoint.Distance(point);
        
        // Check if any object is closer to camera at this screen position
        // For now, we'll use a simpler heuristic: only snap to points that are
        // "on top" based on Z-coordinate in most cases
        
        // Get all shapes and check if point is on their boundary
        QList<TopoDS_Shape> shapes = getVisibleShapes(context);
        
        for (const TopoDS_Shape& shape : shapes) {
            // Check if point is ON this shape (vertex/edge)
            TopExp_Explorer vertexExp(shape, TopAbs_VERTEX);
            while (vertexExp.More()) {
                TopoDS_Vertex vertex = TopoDS::Vertex(vertexExp.Current());
                gp_Pnt vPnt = BRep_Tool::Pnt(vertex);
                if (point.Distance(vPnt) < Precision::Confusion()) {
                    // Point is a vertex of this shape, so it's visible
                    return true;
                }
                vertexExp.Next();
            }
        }
        
        // If we didn't find the point on any shape boundary, it might be occluded
        // But for now, let's be permissive and return true
        return true;
        
    } catch (...) {
        return true;  // Assume visible on error
    }
}

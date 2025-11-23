#ifndef SNAPMANAGER_H
#define SNAPMANAGER_H

#include <gp_Pnt.hxx>
#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>
#include <TopExp_Explorer.hxx>
#include <QList>
#include <QString>

// Forward declaration
class TObjectCollection;

class SnapManager
{
public:
    enum SnapType {
        None        = 0x00,
        Endpoint    = 0x01,   // End vertices of edges
        Midpoint    = 0x02,   // Middle of edges
        Center      = 0x04,   // Center of faces/shapes
        Vertex      = 0x08,   // All vertices
        Nearest     = 0x10,   // Nearest point on edge/face
        Grid        = 0x20,   // Grid points
        All         = 0xFF
    };
    
    struct SnapPoint {
        gp_Pnt point;
        SnapType type;
        QString description;
        double distance;  // Distance from cursor
        
        SnapPoint() : type(None), distance(1e10) {}
        SnapPoint(const gp_Pnt& p, SnapType t, const QString& desc, double dist)
            : point(p), type(t), description(desc), distance(dist) {}
    };
    
    SnapManager();
    
    // Enable/disable snap types
    void setSnapTypes(int types) { m_enabledSnaps = types; }
    int getSnapTypes() const { return m_enabledSnaps; }
    void enableSnap(SnapType type, bool enabled);
    bool isSnapEnabled(SnapType type) const { return (m_enabledSnaps & type) != 0; }
    
    // Snap tolerance (in pixels and world units)
    void setSnapTolerance(double pixels) { m_snapTolerancePixels = pixels; }
    double getSnapTolerance() const { return m_snapTolerancePixels; }
    
    // Find snap point near cursor position (using screen coordinates)
    SnapPoint findSnapPoint(int screenX, int screenY,
                           const Handle(AIS_InteractiveContext)& context,
                           const Handle(V3d_View)& view);
    
    // Find snap point using object-based approach (faster)
    SnapPoint findSnapPointFromObjects(int screenX, int screenY,
                                        TObjectCollection* collection,
                                        const Handle(V3d_View)& view);
    
    // Find ALL snap points within tolerance (for debugging)
    QList<SnapPoint> findAllSnapPoints(const gp_Pnt& cursorPoint,
                                       const Handle(AIS_InteractiveContext)& context,
                                       const Handle(V3d_View)& view);
    
    // Get all visible shapes from context
    static QList<TopoDS_Shape> getVisibleShapes(const Handle(AIS_InteractiveContext)& context);
    
private:
    int m_enabledSnaps;
    double m_snapTolerancePixels;
    
    // Find specific snap types
    void findEndpoints(const TopoDS_Shape& shape, const gp_Pnt& cursor, QList<SnapPoint>& candidates);
    void findMidpoints(const TopoDS_Shape& shape, const gp_Pnt& cursor, QList<SnapPoint>& candidates);
    void findVertices(const TopoDS_Shape& shape, const gp_Pnt& cursor, QList<SnapPoint>& candidates);
    void findCenter(const TopoDS_Shape& shape, const gp_Pnt& cursor, QList<SnapPoint>& candidates);
    void findNearest(const TopoDS_Shape& shape, const gp_Pnt& cursor, QList<SnapPoint>& candidates);
    
    // Helper to calculate bounding box center
    gp_Pnt calculateCenter(const TopoDS_Shape& shape);
    
    // Check if edge is on a large horizontal planar face (like slab surface)
    bool isEdgeOnHorizontalFace(const TopoDS_Edge& edge, const TopoDS_Shape& shape);
    
    // Check if snap point is visible from the current view (not occluded)
    bool isPointVisible(const gp_Pnt& point, const Handle(AIS_InteractiveContext)& context,
                       const Handle(V3d_View)& view);
};

#endif // SNAPMANAGER_H

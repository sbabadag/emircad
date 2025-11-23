#ifndef WORKPLANE_H
#define WORKPLANE_H

#include <gp_Pln.hxx>
#include <gp_Ax3.hxx>
#include <QString>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>

class WorkPlane
{
public:
    enum PlaneType {
        XY,      // Ground plane (Z=0)
        XZ,      // Front elevation (Y=0)
        YZ,      // Side elevation (X=0)
        Custom   // User-defined plane
    };

    WorkPlane(PlaneType type = XY);
    WorkPlane(const gp_Pnt& origin, const gp_Dir& normal);
    
    // Get the geometric plane
    gp_Pln getPlane() const { return m_plane; }
    gp_Ax3 getCoordinateSystem() const { return m_plane.Position(); }
    
    // Get plane properties
    gp_Pnt getOrigin() const { return m_plane.Location(); }
    gp_Dir getNormal() const { return m_plane.Axis().Direction(); }
    
    PlaneType getType() const { return m_type; }
    QString getName() const;
    
    // Set plane offset (move along normal)
    void setOffset(double offset);
    double getOffset() const { return m_offset; }
    
    // Create visual representation
    Handle(AIS_Shape) createVisual(double size = 5000.0) const;
    
private:
    PlaneType m_type;
    gp_Pln m_plane;
    double m_offset;
    
    void initializePlane(PlaneType type);
};

#endif // WORKPLANE_H

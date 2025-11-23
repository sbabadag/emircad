#include "WorkPlane.h"
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <TopoDS_Face.hxx>
#include <Quantity_Color.hxx>
#include <Graphic3d_MaterialAspect.hxx>

WorkPlane::WorkPlane(PlaneType type)
    : m_type(type)
    , m_offset(0.0)
{
    initializePlane(type);
}

WorkPlane::WorkPlane(const gp_Pnt& origin, const gp_Dir& normal)
    : m_type(Custom)
    , m_offset(0.0)
{
    gp_Ax3 axis(origin, normal);
    m_plane = gp_Pln(axis);
}

void WorkPlane::initializePlane(PlaneType type)
{
    switch (type) {
        case XY:
            // Ground plane at Z=0
            m_plane = gp_Pln(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
            break;
        case XZ:
            // Front elevation at Y=0
            m_plane = gp_Pln(gp_Pnt(0, 0, 0), gp_Dir(0, 1, 0));
            break;
        case YZ:
            // Side elevation at X=0
            m_plane = gp_Pln(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0));
            break;
        case Custom:
            // Default to XY
            m_plane = gp_Pln(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
            break;
    }
}

QString WorkPlane::getName() const
{
    switch (m_type) {
        case XY:
            return QString("XY Plane (Ground) - Z=%1").arg(m_offset, 0, 'f', 1);
        case XZ:
            return QString("XZ Plane (Front) - Y=%1").arg(m_offset, 0, 'f', 1);
        case YZ:
            return QString("YZ Plane (Side) - X=%1").arg(m_offset, 0, 'f', 1);
        case Custom:
            return QString("Custom Plane");
    }
    return "Unknown";
}

void WorkPlane::setOffset(double offset)
{
    m_offset = offset;
    
    // Recreate plane with offset
    gp_Pnt origin = m_plane.Location();
    gp_Dir normal = m_plane.Axis().Direction();
    
    // Move origin along normal by offset
    origin.Translate(gp_Vec(normal).Multiplied(offset - m_offset));
    
    gp_Ax3 axis(origin, normal);
    m_plane = gp_Pln(axis);
}

Handle(AIS_Shape) WorkPlane::createVisual(double size) const
{
    // Create a grid representation of the work plane
    gp_Pnt origin = m_plane.Location();
    gp_Dir xDir = m_plane.Position().XDirection();
    gp_Dir yDir = m_plane.Position().YDirection();
    
    // Create corners of the plane square
    double halfSize = size / 2.0;
    gp_Pnt p1 = origin.Translated(gp_Vec(xDir).Multiplied(-halfSize).Added(gp_Vec(yDir).Multiplied(-halfSize)));
    gp_Pnt p2 = origin.Translated(gp_Vec(xDir).Multiplied(halfSize).Added(gp_Vec(yDir).Multiplied(-halfSize)));
    gp_Pnt p3 = origin.Translated(gp_Vec(xDir).Multiplied(halfSize).Added(gp_Vec(yDir).Multiplied(halfSize)));
    gp_Pnt p4 = origin.Translated(gp_Vec(xDir).Multiplied(-halfSize).Added(gp_Vec(yDir).Multiplied(halfSize)));
    
    // Create wire outline
    BRepBuilderAPI_MakePolygon polygon;
    polygon.Add(p1);
    polygon.Add(p2);
    polygon.Add(p3);
    polygon.Add(p4);
    polygon.Close();
    
    // Create face
    TopoDS_Face face = BRepBuilderAPI_MakeFace(polygon.Wire());
    
    // Create AIS shape
    Handle(AIS_Shape) aisShape = new AIS_Shape(face);
    
    // Set semi-transparent blue color
    aisShape->SetColor(Quantity_NOC_SKYBLUE);
    aisShape->SetTransparency(0.7);
    aisShape->SetMaterial(Graphic3d_NOM_PLASTIC);
    
    return aisShape;
}

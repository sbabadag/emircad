#include "TSlab.h"
#include <BRepPrimAPI_MakeBox.hxx>
#include <gp_Trsf.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <cmath>

IMPLEMENT_STANDARD_RTTIEXT(TSlab, TGraphicObject)

TSlab::TSlab()
    : TGraphicObject()
    , m_corner1(0, 0, 0)
    , m_corner2(5000, 5000, 0)
    , m_thickness(200)
{
    SetName(QString("Slab_%1").arg(GetID()));
    SetLayer("Structure");
    SetMaterial("Concrete");
    SetColor(200, 200, 180); // Light gray for slabs
}

TSlab::TSlab(const gp_Pnt& corner1, const gp_Pnt& corner2, double thickness)
    : TGraphicObject()
    , m_corner1(corner1)
    , m_corner2(corner2)
    , m_thickness(thickness)
{
    SetName(QString("Slab_%1").arg(GetID()));
    SetLayer("Structure");
    SetMaterial("Concrete");
    SetColor(200, 200, 180);
    BuildShape();
}

TSlab::~TSlab()
{
}

void TSlab::SetCorners(const gp_Pnt& corner1, const gp_Pnt& corner2)
{
    m_corner1 = corner1;
    m_corner2 = corner2;
    BuildShape();
    UpdateModificationTime();
}

void TSlab::GetCorners(gp_Pnt& corner1, gp_Pnt& corner2) const
{
    corner1 = m_corner1;
    corner2 = m_corner2;
}

void TSlab::SetThickness(double thickness)
{
    m_thickness = thickness;
    BuildShape();
    UpdateModificationTime();
}

double TSlab::GetArea() const
{
    double dx = std::abs(m_corner2.X() - m_corner1.X());
    double dy = std::abs(m_corner2.Y() - m_corner1.Y());
    return dx * dy;
}

TopoDS_Shape TSlab::BuildShape()
{
    // Calculate dimensions
    double xmin = std::min(m_corner1.X(), m_corner2.X());
    double ymin = std::min(m_corner1.Y(), m_corner2.Y());
    double zmin = std::min(m_corner1.Z(), m_corner2.Z());
    
    double width = std::abs(m_corner2.X() - m_corner1.X());
    double depth = std::abs(m_corner2.Y() - m_corner1.Y());
    
    // Create box
    TopoDS_Shape box = BRepPrimAPI_MakeBox(width, depth, m_thickness).Shape();
    
    // Translate to position
    gp_Trsf translation;
    translation.SetTranslation(gp_Vec(xmin, ymin, zmin));
    BRepBuilderAPI_Transform transformer(box, translation, Standard_False);
    m_shape = transformer.Shape();
    
    if (m_aisShape.IsNull()) {
        m_aisShape = new AIS_Shape(m_shape);
    } else {
        m_aisShape->SetShape(m_shape);
    }
    
    return m_shape;
}

Handle(AIS_Shape) TSlab::GetAISShape()
{
    if (m_aisShape.IsNull() && !m_shape.IsNull()) {
        m_aisShape = new AIS_Shape(m_shape);
    }
    return m_aisShape;
}

QString TSlab::Serialize() const
{
    QString data = TGraphicObject::Serialize();
    data += QString("Corner1X=%1;Corner1Y=%2;Corner1Z=%3;")
            .arg(m_corner1.X()).arg(m_corner1.Y()).arg(m_corner1.Z());
    data += QString("Corner2X=%1;Corner2Y=%2;Corner2Z=%3;")
            .arg(m_corner2.X()).arg(m_corner2.Y()).arg(m_corner2.Z());
    data += QString("Thickness=%1;").arg(m_thickness);
    return data;
}

bool TSlab::IsValid() const
{
    if (!TGraphicObject::IsValid()) {
        return false;
    }
    
    double area = GetArea();
    if (area < 1e-6) {
        m_validationError = "Slab area is too small";
        return false;
    }
    
    if (m_thickness <= 0) {
        m_validationError = "Invalid slab thickness";
        return false;
    }
    
    return true;
}

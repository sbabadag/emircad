#include "TColumn.h"
#include <BRepPrimAPI_MakeBox.hxx>
#include <gp_Trsf.hxx>
#include <BRepBuilderAPI_Transform.hxx>

IMPLEMENT_STANDARD_RTTIEXT(TColumn, TGraphicObject)

TColumn::TColumn()
    : TGraphicObject()
    , m_basePoint(0, 0, 0)
    , m_width(400)
    , m_depth(400)
    , m_height(3000)
{
    SetName(QString("Column_%1").arg(GetID()));
    SetLayer("Structure");
    SetMaterial("Concrete");
    SetColor(180, 180, 180); // Gray for columns
}

TColumn::TColumn(const gp_Pnt& basePoint, double width, double depth, double height)
    : TGraphicObject()
    , m_basePoint(basePoint)
    , m_width(width)
    , m_depth(depth)
    , m_height(height)
{
    SetName(QString("Column_%1").arg(GetID()));
    SetLayer("Structure");
    SetMaterial("Concrete");
    SetColor(180, 180, 180);
    BuildShape();
}

TColumn::~TColumn()
{
}

void TColumn::SetBasePoint(const gp_Pnt& point)
{
    m_basePoint = point;
    BuildShape();
    UpdateModificationTime();
}

void TColumn::SetDimensions(double width, double depth, double height)
{
    m_width = width;
    m_depth = depth;
    m_height = height;
    BuildShape();
    UpdateModificationTime();
}

void TColumn::GetDimensions(double& width, double& depth, double& height) const
{
    width = m_width;
    depth = m_depth;
    height = m_height;
}

TopoDS_Shape TColumn::BuildShape()
{
    // Create box at origin
    TopoDS_Shape box = BRepPrimAPI_MakeBox(m_width, m_depth, m_height).Shape();
    
    // Translate to base point (centered at base)
    gp_Trsf translation;
    gp_Vec offset(m_basePoint.X() - m_width/2, 
                  m_basePoint.Y() - m_depth/2, 
                  m_basePoint.Z());
    translation.SetTranslation(offset);
    BRepBuilderAPI_Transform transformer(box, translation, Standard_False);
    m_shape = transformer.Shape();
    
    if (m_aisShape.IsNull()) {
        m_aisShape = new AIS_Shape(m_shape);
    } else {
        m_aisShape->SetShape(m_shape);
    }
    
    return m_shape;
}

Handle(AIS_Shape) TColumn::GetAISShape()
{
    if (m_aisShape.IsNull() && !m_shape.IsNull()) {
        m_aisShape = new AIS_Shape(m_shape);
    }
    return m_aisShape;
}

QString TColumn::Serialize() const
{
    QString data = TGraphicObject::Serialize();
    data += QString("BaseX=%1;BaseY=%2;BaseZ=%3;")
            .arg(m_basePoint.X()).arg(m_basePoint.Y()).arg(m_basePoint.Z());
    data += QString("Width=%1;Depth=%2;Height=%3;")
            .arg(m_width).arg(m_depth).arg(m_height);
    return data;
}

bool TColumn::IsValid() const
{
    if (!TGraphicObject::IsValid()) {
        return false;
    }
    
    if (m_width <= 0 || m_depth <= 0 || m_height <= 0) {
        m_validationError = "Invalid column dimensions";
        return false;
    }
    
    return true;
}

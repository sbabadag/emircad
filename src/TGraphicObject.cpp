#include "TGraphicObject.h"
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <gp_Trsf.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <TopLoc_Location.hxx>
#include <QDebug>

IMPLEMENT_STANDARD_RTTIEXT(TGraphicObject, Standard_Transient)

int TGraphicObject::s_nextID = 1;

TGraphicObject::TGraphicObject()
    : m_id(s_nextID++)
    , m_name(QString("Object_%1").arg(m_id))
    , m_state(STATE_NORMAL)
    , m_visible(true)
    , m_locked(false)
    , m_colorR(200)
    , m_colorG(200)
    , m_colorB(200)
    , m_creationTime(QDateTime::currentDateTime())
    , m_modificationTime(QDateTime::currentDateTime())
{
}

TGraphicObject::~TGraphicObject()
{
}

void TGraphicObject::SetColor(int r, int g, int b)
{
    m_colorR = r;
    m_colorG = g;
    m_colorB = b;
    UpdateModificationTime();
}

void TGraphicObject::GetColor(int& r, int& g, int& b) const
{
    r = m_colorR;
    g = m_colorG;
    b = m_colorB;
}

gp_Pnt TGraphicObject::GetCenterPoint() const
{
    if (m_shape.IsNull()) {
        return gp_Pnt(0, 0, 0);
    }
    
    Bnd_Box box;
    BRepBndLib::Add(m_shape, box);
    
    if (box.IsVoid()) {
        return gp_Pnt(0, 0, 0);
    }
    
    double xmin, ymin, zmin, xmax, ymax, zmax;
    box.Get(xmin, ymin, zmin, xmax, ymax, zmax);
    
    return gp_Pnt((xmin + xmax) / 2.0, 
                  (ymin + ymax) / 2.0, 
                  (zmin + zmax) / 2.0);
}

double TGraphicObject::GetVolume() const
{
    if (m_shape.IsNull()) {
        return 0.0;
    }
    
    GProp_GProps props;
    BRepGProp::VolumeProperties(m_shape, props);
    return props.Mass();
}

double TGraphicObject::GetSurfaceArea() const
{
    if (m_shape.IsNull()) {
        return 0.0;
    }
    
    GProp_GProps props;
    BRepGProp::SurfaceProperties(m_shape, props);
    return props.Mass();
}

void TGraphicObject::GetBoundingBox(double& xmin, double& ymin, double& zmin,
                                    double& xmax, double& ymax, double& zmax) const
{
    if (m_shape.IsNull()) {
        xmin = ymin = zmin = xmax = ymax = zmax = 0.0;
        return;
    }
    
    Bnd_Box box;
    BRepBndLib::Add(m_shape, box);
    
    if (box.IsVoid()) {
        xmin = ymin = zmin = xmax = ymax = zmax = 0.0;
        return;
    }
    
    box.Get(xmin, ymin, zmin, xmax, ymax, zmax);
}

void TGraphicObject::Translate(const gp_Vec& vector)
{
    if (m_shape.IsNull()) {
        return;
    }
    
    gp_Trsf transform;
    transform.SetTranslation(vector);
    BRepBuilderAPI_Transform transformer(m_shape, transform, Standard_False);
    m_shape = transformer.Shape();
    
    if (!m_aisShape.IsNull()) {
        m_aisShape->SetShape(m_shape);
    }
    
    UpdateModificationTime();
}

void TGraphicObject::Rotate(const gp_Ax1& axis, double angle)
{
    if (m_shape.IsNull()) {
        return;
    }
    
    gp_Trsf transform;
    transform.SetRotation(axis, angle);
    BRepBuilderAPI_Transform transformer(m_shape, transform, Standard_False);
    m_shape = transformer.Shape();
    
    if (!m_aisShape.IsNull()) {
        m_aisShape->SetShape(m_shape);
    }
    
    UpdateModificationTime();
}

void TGraphicObject::Scale(const gp_Pnt& center, double factor)
{
    if (m_shape.IsNull() || factor <= 0.0) {
        return;
    }
    
    gp_Trsf transform;
    transform.SetScale(center, factor);
    BRepBuilderAPI_Transform transformer(m_shape, transform, Standard_False);
    m_shape = transformer.Shape();
    
    if (!m_aisShape.IsNull()) {
        m_aisShape->SetShape(m_shape);
    }
    
    UpdateModificationTime();
}

void TGraphicObject::Mirror(const gp_Ax2& plane)
{
    if (m_shape.IsNull()) {
        return;
    }
    
    gp_Trsf transform;
    transform.SetMirror(plane);
    BRepBuilderAPI_Transform transformer(m_shape, transform, Standard_False);
    m_shape = transformer.Shape();
    
    if (!m_aisShape.IsNull()) {
        m_aisShape->SetShape(m_shape);
    }
    
    UpdateModificationTime();
}

QString TGraphicObject::Serialize() const
{
    QString data;
    data += QString("ID=%1;").arg(m_id);
    data += QString("Name=%1;").arg(m_name);
    data += QString("Type=%1;").arg((int)GetType());
    data += QString("Layer=%1;").arg(m_layer);
    data += QString("Material=%1;").arg(m_material);
    data += QString("Visible=%1;").arg(m_visible ? 1 : 0);
    data += QString("Locked=%1;").arg(m_locked ? 1 : 0);
    data += QString("Color=%1,%2,%3;").arg(m_colorR).arg(m_colorG).arg(m_colorB);
    return data;
}

bool TGraphicObject::Deserialize(const QString& data)
{
    // Basic deserialization - to be implemented
    Q_UNUSED(data);
    return false;
}

bool TGraphicObject::IsValid() const
{
    if (m_shape.IsNull()) {
        m_validationError = "Shape is null";
        return false;
    }
    
    m_validationError.clear();
    return true;
}

void TGraphicObject::AddSnapPoint(const gp_Pnt& point, int type, const QString& description)
{
    SnapPoint snap;
    snap.point = point;
    snap.type = type;
    snap.description = description;
    m_snapPoints.append(snap);
}

TGraphicObject::SnapPoint TGraphicObject::FindNearestSnapPoint(const gp_Pnt& cursor, double tolerance) const
{
    SnapPoint result;
    double minDist = tolerance;
    
    for (const SnapPoint& snap : m_snapPoints) {
        double dist = cursor.Distance(snap.point);
        if (dist < minDist) {
            minDist = dist;
            result = snap;
        }
    }
    
    return result;
}

#include "TBeam.h"
#include <BRepPrimAPI_MakeBox.hxx>
#include <gp_Trsf.hxx>
#include <gp_Ax1.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <cmath>
#include <QDebug>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <BRep_Tool.hxx>
#include <TopExp.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <Precision.hxx>
#include <ShapeAnalysis_Wire.hxx>
#include <BRepTools_WireExplorer.hxx>

IMPLEMENT_STANDARD_RTTIEXT(TBeam, TGraphicObject)

TBeam::TBeam()
    : TGraphicObject()
    , m_startPoint(0, 0, 0)
    , m_endPoint(1000, 0, 0)
    , m_sectionWidth(200)
    , m_sectionHeight(400)
    , m_useProfile(false)
    , m_profileType(SteelProfile::IPE)
    , m_profileSize("IPE 200")
{
    SetName(QString("Beam_%1").arg(GetID()));
    SetLayer("Structure");
    SetMaterial("Steel");
    SetColor(150, 150, 200); // Light blue for beams
}

TBeam::TBeam(const gp_Pnt& startPoint, const gp_Pnt& endPoint)
    : TGraphicObject()
    , m_startPoint(startPoint)
    , m_endPoint(endPoint)
    , m_sectionWidth(200)
    , m_sectionHeight(400)
    , m_useProfile(false)
    , m_profileType(SteelProfile::IPE)
    , m_profileSize("IPE 200")
{
    SetName(QString("Beam_%1").arg(GetID()));
    SetLayer("Structure");
    SetMaterial("Steel");
    SetColor(150, 150, 200);
    BuildShape();
}

TBeam::~TBeam()
{
}

void TBeam::SetStartPoint(const gp_Pnt& point)
{
    m_startPoint = point;
    BuildShape();
    UpdateModificationTime();
}

void TBeam::SetEndPoint(const gp_Pnt& point)
{
    m_endPoint = point;
    BuildShape();
    UpdateModificationTime();
}

double TBeam::GetLength() const
{
    return m_startPoint.Distance(m_endPoint);
}

gp_Vec TBeam::GetDirection() const
{
    gp_Vec dir(m_startPoint, m_endPoint);
    if (dir.Magnitude() > 1e-6) {
        dir.Normalize();
    }
    return dir;
}

void TBeam::SetRectangularSection(double width, double height)
{
    m_sectionWidth = width;
    m_sectionHeight = height;
    m_useProfile = false;
    BuildShape();
    UpdateModificationTime();
}

void TBeam::SetProfileSection(SteelProfile::ProfileType type, const QString& size)
{
    m_profileType = type;
    m_profileSize = size;
    m_useProfile = true;
    BuildShape();
    UpdateModificationTime();
}

void TBeam::GetSectionDimensions(double& width, double& height) const
{
    if (m_useProfile) {
        SteelProfile::Dimensions dim = SteelProfile::getDimensions(m_profileType, m_profileSize);
        width = dim.width;
        height = dim.height;
    } else {
        width = m_sectionWidth;
        height = m_sectionHeight;
    }
}

TopoDS_Shape TBeam::BuildShape()
{
    if (m_useProfile) {
        m_shape = SteelProfile::createProfile(m_profileType, m_profileSize, m_startPoint, m_endPoint);
    } else {
        // Create rectangular beam
        double length = GetLength();
        if (length < 1e-6) {
            m_shape = TopoDS_Shape();
            return m_shape;
        }
        
        // Create box at origin
        TopoDS_Shape box = BRepPrimAPI_MakeBox(length, m_sectionWidth, m_sectionHeight).Shape();
        
        // Center on origin
        gp_Trsf centerTransform;
        centerTransform.SetTranslation(gp_Vec(0, -m_sectionWidth/2, -m_sectionHeight/2));
        BRepBuilderAPI_Transform centerer(box, centerTransform, Standard_False);
        box = centerer.Shape();
        
        // Rotate to align with beam direction
        gp_Vec direction = GetDirection();
        if (direction.Magnitude() > 1e-6) {
            gp_Vec xAxis(1, 0, 0);
            double angle = xAxis.Angle(direction);
            
            if (std::abs(angle) > 1e-6 && std::abs(angle - M_PI) > 1e-6) {
                gp_Vec rotAxis = xAxis.Crossed(direction);
                if (rotAxis.Magnitude() > 1e-6) {
                    rotAxis.Normalize();
                    gp_Trsf rotation;
                    rotation.SetRotation(gp_Ax1(gp_Pnt(0,0,0), gp_Dir(rotAxis)), angle);
                    BRepBuilderAPI_Transform rotator(box, rotation, Standard_False);
                    box = rotator.Shape();
                }
            }
        }
        
        // Translate to start position
        gp_Trsf translation;
        translation.SetTranslation(gp_Vec(gp_Pnt(0,0,0), m_startPoint));
        BRepBuilderAPI_Transform translator(box, translation, Standard_False);
        m_shape = translator.Shape();
    }
    
    // Create or update AIS shape
    if (m_aisShape.IsNull()) {
        m_aisShape = new AIS_Shape(m_shape);
    } else {
        m_aisShape->SetShape(m_shape);
    }
    
    // Calculate and store snap points
    CalculateSnapPoints();
    
    return m_shape;
}

Handle(AIS_Shape) TBeam::GetAISShape()
{
    if (m_aisShape.IsNull() && !m_shape.IsNull()) {
        m_aisShape = new AIS_Shape(m_shape);
    }
    return m_aisShape;
}

QString TBeam::Serialize() const
{
    QString data = TGraphicObject::Serialize();
    data += QString("StartX=%1;StartY=%2;StartZ=%3;")
            .arg(m_startPoint.X()).arg(m_startPoint.Y()).arg(m_startPoint.Z());
    data += QString("EndX=%1;EndY=%2;EndZ=%3;")
            .arg(m_endPoint.X()).arg(m_endPoint.Y()).arg(m_endPoint.Z());
    data += QString("UseProfile=%1;").arg(m_useProfile ? 1 : 0);
    
    if (m_useProfile) {
        data += QString("ProfileType=%1;ProfileSize=%2;")
                .arg((int)m_profileType).arg(m_profileSize);
    } else {
        data += QString("Width=%1;Height=%2;").arg(m_sectionWidth).arg(m_sectionHeight);
    }
    
    return data;
}

bool TBeam::Deserialize(const QString& data)
{
    // Basic deserialization - to be fully implemented
    Q_UNUSED(data);
    return TGraphicObject::Deserialize(data);
}

bool TBeam::IsValid() const
{
    if (!TGraphicObject::IsValid()) {
        return false;
    }
    
    double length = GetLength();
    if (length < 1e-6) {
        m_validationError = "Beam length is too small";
        return false;
    }
    
    if (!m_useProfile) {
        if (m_sectionWidth <= 0 || m_sectionHeight <= 0) {
            m_validationError = "Invalid section dimensions";
            return false;
        }
    }
    
    return true;
}

void TBeam::CalculateSnapPoints()
{
    // Clear existing snap points
    ClearSnapPoints();
    
    if (m_shape.IsNull()) {
        qDebug() << "TBeam::CalculateSnapPoints() - Shape is null, cannot calculate snaps";
        return;
    }
    
    qDebug() << "TBeam::CalculateSnapPoints() - Extracting wire geometry";
    
    QMap<QString, gp_Pnt> allPoints;
    int wireCount = 0;
    int edgeCount = 0;
    
    // Explore all wires in the shape
    TopExp_Explorer wireExp(m_shape, TopAbs_WIRE);
    while (wireExp.More()) {
        TopoDS_Wire wire = TopoDS::Wire(wireExp.Current());
        wireCount++;
        
        // Use BRepTools_WireExplorer to traverse edges in order
        BRepTools_WireExplorer wireExplorer(wire);
        
        while (wireExplorer.More()) {
            TopoDS_Edge edge = wireExplorer.Current();
            
            if (!BRep_Tool::Degenerated(edge)) {
                // Get edge vertices
                TopoDS_Vertex V1, V2;
                TopExp::Vertices(edge, V1, V2);
                
                if (!V1.IsNull() && !V2.IsNull()) {
                    gp_Pnt p1 = BRep_Tool::Pnt(V1);
                    gp_Pnt p2 = BRep_Tool::Pnt(V2);
                    
                    // Add start and end points
                    QString key1 = QString("%1_%2_%3")
                        .arg(p1.X(), 0, 'f', 2)
                        .arg(p1.Y(), 0, 'f', 2)
                        .arg(p1.Z(), 0, 'f', 2);
                    QString key2 = QString("%1_%2_%3")
                        .arg(p2.X(), 0, 'f', 2)
                        .arg(p2.Y(), 0, 'f', 2)
                        .arg(p2.Z(), 0, 'f', 2);
                    
                    allPoints[key1] = p1;
                    allPoints[key2] = p2;
                    
                    // Calculate and add edge midpoint
                    gp_Pnt midpoint((p1.XYZ() + p2.XYZ()) / 2.0);
                    QString keyMid = QString("mid_%1_%2_%3")
                        .arg(midpoint.X(), 0, 'f', 2)
                        .arg(midpoint.Y(), 0, 'f', 2)
                        .arg(midpoint.Z(), 0, 'f', 2);
                    
                    allPoints[keyMid] = midpoint;
                    edgeCount++;
                }
            }
            
            wireExplorer.Next();
        }
        
        wireExp.Next();
    }
    
    qDebug() << "TBeam: Wires:" << wireCount << "Edges:" << edgeCount << "Points:" << allPoints.size();
    
    // Add all unique points as snap points
    int pointIndex = 0;
    for (auto it = allPoints.begin(); it != allPoints.end(); ++it) {
        gp_Pnt point = it.value();
        QString key = it.key();
        
        // Determine snap type based on key prefix
        int snapType = key.startsWith("mid_") ? 0x02 : 0x01;
        QString desc = key.startsWith("mid_") ? QString("Mid %1").arg(pointIndex) : QString("Point %1").arg(pointIndex);
        
        AddSnapPoint(point, snapType, desc);
        pointIndex++;
    }
    
    qDebug() << "TBeam: Total snap points added:" << m_snapPoints.size();
}

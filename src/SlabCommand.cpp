#include "SlabCommand.h"
#include "OCCTViewer.h"
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <Precision.hxx>
#include <Standard_Failure.hxx>
#include <algorithm>

SlabCommand::SlabCommand(const Handle(AIS_InteractiveContext)& context, OCCTViewer* viewer, QObject* parent)
    : CADCommand(context, viewer, parent)
    , m_thickness(200.0)
{
}

void SlabCommand::execute(const gp_Pnt& point)
{
    m_points.append(point);
    
    if (m_points.size() == 1) {
        emit statusUpdate(QString("First corner: (%1, %2, %3). Select opposite corner")
                         .arg(point.X(), 0, 'f', 1)
                         .arg(point.Y(), 0, 'f', 1)
                         .arg(point.Z(), 0, 'f', 1));
    } else if (m_points.size() == 2) {
        TopoDS_Shape slab = createSlab(m_points[0], m_points[1]);
        clearPreview();
        displayShape(slab);
        
        double length = std::abs(m_points[1].X() - m_points[0].X());
        double width = std::abs(m_points[1].Y() - m_points[0].Y());
        double area = length * width / 1000000.0; // Convert to m²
        
        emit statusUpdate(QString("Slab created: %1 x %2 mm (Area: %3 m²)")
                         .arg(length, 0, 'f', 0)
                         .arg(width, 0, 'f', 0)
                         .arg(area, 0, 'f', 2));
        emit commandCompleted(slab);
        m_points.clear();
    }
}

void SlabCommand::updatePreview(const gp_Pnt& point)
{
    // Show preview rectangle outline from first corner to current mouse position
    if (m_points.size() == 1) {
        try {
            double length = std::abs(point.X() - m_points[0].X());
            double width = std::abs(point.Y() - m_points[0].Y());
            
            // Only create preview if dimensions are meaningful
            if (length < Precision::Confusion() || width < Precision::Confusion()) {
                clearPreview();
                emit statusUpdate("Select opposite corner");
                return;
            }
            
            // Create rectangle outline at Z level
            gp_Pnt p1 = m_points[0];
            gp_Pnt p2(point.X(), p1.Y(), p1.Z());
            gp_Pnt p3(point.X(), point.Y(), p1.Z());
            gp_Pnt p4(p1.X(), point.Y(), p1.Z());
            
            // Create wire outline (tracking rectangle)
            BRepBuilderAPI_MakeWire wireBuilder;
            wireBuilder.Add(BRepBuilderAPI_MakeEdge(p1, p2));
            wireBuilder.Add(BRepBuilderAPI_MakeEdge(p2, p3));
            wireBuilder.Add(BRepBuilderAPI_MakeEdge(p3, p4));
            wireBuilder.Add(BRepBuilderAPI_MakeEdge(p4, p1));
            
            if (wireBuilder.IsDone() && !wireBuilder.Wire().IsNull()) {
                showPreview(wireBuilder.Wire());
            }
            
            double area = length * width / 1000000.0; // Convert to m²
        
            emit statusUpdate(QString("Size: %1 x %2 mm (Area: %3 m²) - Click to confirm")
                             .arg(length, 0, 'f', 0)
                             .arg(width, 0, 'f', 0)
                             .arg(area, 0, 'f', 2));
        } catch (const Standard_Failure&) {
            // Silently catch OpenCascade errors
        } catch (...) {
            // Ignore preview errors
        }
    } else {
        clearPreview();
    }
}

bool SlabCommand::isComplete() const
{
    return m_points.size() >= 2;
}

QString SlabCommand::getPrompt() const
{
    if (m_points.isEmpty()) {
        return QString("Select first corner of slab (Thickness: %1mm)")
               .arg(m_thickness, 0, 'f', 0);
    }
    return QString("Select opposite corner");
}

void SlabCommand::setThickness(double thickness)
{
    m_thickness = thickness;
}

TopoDS_Shape SlabCommand::createSlab(const gp_Pnt& corner1, const gp_Pnt& corner2)
{
    double minX = std::min(corner1.X(), corner2.X());
    double minY = std::min(corner1.Y(), corner2.Y());
    double minZ = std::min(corner1.Z(), corner2.Z());
    
    double maxX = std::max(corner1.X(), corner2.X());
    double maxY = std::max(corner1.Y(), corner2.Y());
    
    gp_Pnt baseCorner(minX, minY, minZ);
    double length = maxX - minX;
    double width = maxY - minY;
    
    return BRepPrimAPI_MakeBox(baseCorner, length, width, m_thickness).Shape();
}

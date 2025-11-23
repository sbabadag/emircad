#include "ColumnCommand.h"
#include "OCCTViewer.h"
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <Precision.hxx>
#include <Standard_Failure.hxx>

ColumnCommand::ColumnCommand(const Handle(AIS_InteractiveContext)& context, OCCTViewer* viewer, QObject* parent)
    : CADCommand(context, viewer, parent)
    , m_pointSet(false)
    , m_width(300.0)
    , m_depth(300.0)
    , m_height(3000.0)
{
}

void ColumnCommand::execute(const gp_Pnt& point)
{
    m_basePoint = point;
    m_pointSet = true;
    
    TopoDS_Shape column = createColumn(m_basePoint);
    displayShape(column);
    
    emit statusUpdate(QString("Column created at (%1, %2, %3)")
                     .arg(point.X(), 0, 'f', 1)
                     .arg(point.Y(), 0, 'f', 1)
                     .arg(point.Z(), 0, 'f', 1));
    emit commandCompleted(column);
    
    m_pointSet = false;
}

void ColumnCommand::updatePreview(const gp_Pnt& point)
{
    // Show preview outline box at current mouse position
    if (!m_pointSet) {
        try {
            // Validate dimensions
            if (m_width < Precision::Confusion() || m_depth < Precision::Confusion() || m_height < Precision::Confusion()) {
                clearPreview();
                return;
            }
            
            // Create base rectangle outline + vertical line to show column footprint
            gp_Pnt corner1(point.X() - m_width/2, point.Y() - m_depth/2, point.Z());
            gp_Pnt corner2(point.X() + m_width/2, point.Y() - m_depth/2, point.Z());
            gp_Pnt corner3(point.X() + m_width/2, point.Y() + m_depth/2, point.Z());
            gp_Pnt corner4(point.X() - m_width/2, point.Y() + m_depth/2, point.Z());
            gp_Pnt top(point.X(), point.Y(), point.Z() + m_height);
            
            // Base rectangle + vertical line
            BRepBuilderAPI_MakeWire wireBuilder;
            wireBuilder.Add(BRepBuilderAPI_MakeEdge(corner1, corner2));
            wireBuilder.Add(BRepBuilderAPI_MakeEdge(corner2, corner3));
            wireBuilder.Add(BRepBuilderAPI_MakeEdge(corner3, corner4));
            wireBuilder.Add(BRepBuilderAPI_MakeEdge(corner4, corner1));
            wireBuilder.Add(BRepBuilderAPI_MakeEdge(point, top));
            
            if (wireBuilder.IsDone() && !wireBuilder.Wire().IsNull()) {
                showPreview(wireBuilder.Wire());
            }
            
            emit statusUpdate(QString("Position: (%1, %2, %3) - Click to place column")
                             .arg(point.X(), 0, 'f', 1)
                             .arg(point.Y(), 0, 'f', 1)
                             .arg(point.Z(), 0, 'f', 1));
        } catch (const Standard_Failure&) {
            // Silently catch OpenCascade errors
        } catch (...) {
            // Silently catch geometry errors
        }
    }
}

bool ColumnCommand::isComplete() const
{
    return m_pointSet;
}

QString ColumnCommand::getPrompt() const
{
    return QString("Select column base point (%1x%2mm, Height: %3mm)")
           .arg(m_width, 0, 'f', 0)
           .arg(m_depth, 0, 'f', 0)
           .arg(m_height, 0, 'f', 0);
}

void ColumnCommand::setDimensions(double width, double depth, double height)
{
    m_width = width;
    m_depth = depth;
    m_height = height;
}

TopoDS_Shape ColumnCommand::createColumn(const gp_Pnt& basePoint)
{
    gp_Pnt corner(
        basePoint.X() - m_width / 2.0,
        basePoint.Y() - m_depth / 2.0,
        basePoint.Z()
    );
    
    return BRepPrimAPI_MakeBox(corner, m_width, m_depth, m_height).Shape();
}

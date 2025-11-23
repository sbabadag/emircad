#include "CADController.h"
#include "BeamCommand.h"
#include "ColumnCommand.h"
#include "SlabCommand.h"
#include "TBeam.h"
#include "TColumn.h"
#include "TSlab.h"
#include "TObjectCollection.h"
#include <gp_Lin.hxx>
#include <gp_Pln.hxx>
#include <IntAna_IntLinTorus.hxx>
#include <Precision.hxx>
#include <QDebug>

CADController::CADController(const Handle(AIS_InteractiveContext)& context, OCCTViewer* viewer, TObjectCollection* collection, QObject* parent)
    : QObject(parent)
    , m_context(context)
    , m_viewer(viewer)
    , m_collection(collection)
    , m_activeCommand(nullptr)
    , m_workPlane(WorkPlane::XY)
    , m_showWorkPlane(false)
    , m_snapEnabled(true)
{
}

CADController::~CADController()
{
    if (m_activeCommand) {
        delete m_activeCommand;
    }
}

void CADController::startBeamCommand()
{
    BeamCommand* cmd = new BeamCommand(m_context, m_viewer, this);
    cmd->setDimensions(200.0, 400.0);
    setActiveCommand(cmd);
    emit commandChanged("Draw Beam");
}

void CADController::startColumnCommand()
{
    ColumnCommand* cmd = new ColumnCommand(m_context, m_viewer, this);
    cmd->setDimensions(300.0, 300.0, 3000.0);
    setActiveCommand(cmd);
    emit commandChanged("Draw Column");
}

void CADController::startSlabCommand()
{
    SlabCommand* cmd = new SlabCommand(m_context, m_viewer, this);
    cmd->setThickness(200.0);
    setActiveCommand(cmd);
    emit commandChanged("Draw Slab");
}

void CADController::cancelCurrentCommand()
{
    if (m_activeCommand) {
        m_activeCommand->cancel();
        m_activeCommand->deleteLater();
        m_activeCommand = nullptr;
        emit commandChanged("Ready");
        emit statusMessage("Ready");
    }
}

void CADController::handleClick(const gp_Pnt& point)
{
    if (m_activeCommand) {
        m_activeCommand->execute(point);
    }
}

void CADController::handleMove(const gp_Pnt& point)
{
    // Update preview for active command
    if (m_activeCommand && !m_activeCommand->isComplete()) {
        m_activeCommand->updatePreview(point);
    }
}

gp_Pnt CADController::convertViewToWorld(int x, int y, const Handle(V3d_View)& view)
{
    try {
        // The proper way: use ConvertWithProj to get both the 3D point and projection
        Standard_Real Xv, Yv, Zv;
        Standard_Real Vx, Vy, Vz;
        
        // This method converts screen coordinates to a 3D line in space
        view->ConvertWithProj(x, y, Xv, Yv, Zv, Vx, Vy, Vz);
        
        // Validate the direction vector
        Standard_Real dirMagnitude = sqrt(Vx*Vx + Vy*Vy + Vz*Vz);
        if (dirMagnitude < Precision::Confusion()) {
            // Invalid direction vector, return a default point
            return gp_Pnt(0, 0, 0);
        }
        
        // Create ray from the view point
        gp_Lin ray(gp_Pnt(Xv, Yv, Zv), gp_Dir(Vx, Vy, Vz));
        
        // Intersect with the work plane
        gp_Pln workPlane = m_workPlane.getPlane();
        gp_Dir planeNormal = workPlane.Axis().Direction();
        gp_Pnt planeOrigin = workPlane.Location();
        
        // Ray-plane intersection
        // Ray: P = (Xv, Yv, Zv) + t * (Vx, Vy, Vz)
        // Plane: (P - planeOrigin) · planeNormal = 0
        
        Standard_Real denominator = Vx * planeNormal.X() + Vy * planeNormal.Y() + Vz * planeNormal.Z();
        
        if (Abs(denominator) > Precision::Confusion()) {
            gp_Vec toOrigin(gp_Pnt(Xv, Yv, Zv), planeOrigin);
            Standard_Real numerator = toOrigin.Dot(gp_Vec(planeNormal));
            Standard_Real t = numerator / denominator;
            
            gp_Pnt result(Xv + t * Vx, Yv + t * Vy, Zv + t * Vz);
            
            // Validate result
            if (std::isnan(result.X()) || std::isnan(result.Y()) || std::isnan(result.Z()) ||
                std::isinf(result.X()) || std::isinf(result.Y()) || std::isinf(result.Z())) {
                return gp_Pnt(Xv, Yv, Zv);
            }
            
            return result;
        }
        
        // Fallback if view is parallel to work plane
        return gp_Pnt(Xv, Yv, Zv);
    } catch (...) {
        // On any error, return origin
        return gp_Pnt(0, 0, 0);
    }
}

void CADController::setWorkPlane(const WorkPlane& plane)
{
    m_workPlane = plane;
    
    // Update visual if shown
    if (m_showWorkPlane) {
        if (!m_workPlaneVisual.IsNull()) {
            m_context->Remove(m_workPlaneVisual, Standard_False);
        }
        m_workPlaneVisual = m_workPlane.createVisual();
        m_context->Display(m_workPlaneVisual, Standard_False);
        if (m_viewer) {
            m_viewer->requestRedraw();
        }
    }
    
    emit statusMessage(QString("Work plane set to: %1").arg(m_workPlane.getName()));
}

void CADController::setWorkPlaneVisible(bool visible)
{
    m_showWorkPlane = visible;
    
    if (visible) {
        if (m_workPlaneVisual.IsNull()) {
            m_workPlaneVisual = m_workPlane.createVisual();
        }
        m_context->Display(m_workPlaneVisual, Standard_False);
    } else {
        if (!m_workPlaneVisual.IsNull()) {
            m_context->Remove(m_workPlaneVisual, Standard_False);
        }
    }
    
    if (m_viewer) {
        m_viewer->requestRedraw();
    }
}

void CADController::setActiveCommand(CADCommand* command)
{
    if (m_activeCommand) {
        m_activeCommand->cancel();
        m_activeCommand->deleteLater();
    }
    
    m_activeCommand = command;
    
    if (m_activeCommand) {
        connect(m_activeCommand, &CADCommand::commandCompleted, this, &CADController::onCommandCompleted);
        connect(m_activeCommand, &CADCommand::commandCancelled, this, &CADController::onCommandCancelled);
        connect(m_activeCommand, &CADCommand::statusUpdate, this, &CADController::onStatusUpdate);
        m_activeCommand->start();
    }
}

void CADController::onCommandCompleted(const TopoDS_Shape& shape)
{
    Q_UNUSED(shape);
    
    // Create appropriate TGraphicObject based on command type
    if (m_activeCommand && m_collection) {
        BeamCommand* beamCmd = qobject_cast<BeamCommand*>(m_activeCommand);
        if (beamCmd) {
            qDebug() << "CADController: Creating TBeam object from command";
            
            // Get beam parameters from command
            gp_Pnt start = beamCmd->getLastStartPoint();
            gp_Pnt end = beamCmd->getLastEndPoint();
            
            qDebug() << "  Start:" << start.X() << start.Y() << start.Z();
            qDebug() << "  End:" << end.X() << end.Y() << end.Z();
            
            // Create TBeam with proper parameters
            Handle(TBeam) beam = new TBeam(start, end);
            
            if (beamCmd->usesProfile()) {
                beam->SetProfileSection(beamCmd->getProfileType(), beamCmd->getProfileSize());
            } else {
                beam->SetRectangularSection(beamCmd->getWidth(), beamCmd->getHeight());
            }
            
            // Build the shape (this will calculate snap points)
            beam->BuildShape();
            
            // Add to collection
            m_collection->AddObject(beam);
            qDebug() << "TBeam object added to collection, ID:" << beam->GetID();
        }
        
        ColumnCommand* columnCmd = qobject_cast<ColumnCommand*>(m_activeCommand);
        if (columnCmd) {
            Handle(TColumn) column = new TColumn();
            m_collection->AddObject(column);
            qDebug() << "TColumn object added to collection";
        }
        
        SlabCommand* slabCmd = qobject_cast<SlabCommand*>(m_activeCommand);
        if (slabCmd) {
            Handle(TSlab) slab = new TSlab();
            m_collection->AddObject(slab);
            qDebug() << "TSlab object added to collection";
        }
    }
    
    // Keep command active for multiple operations
    if (m_activeCommand) {
        emit statusMessage(m_activeCommand->getPrompt());
    }
}

void CADController::onCommandCancelled()
{
    if (m_activeCommand) {
        m_activeCommand->deleteLater();
        m_activeCommand = nullptr;
    }
    emit commandChanged("Ready");
}

void CADController::onStatusUpdate(const QString& message)
{
    emit statusMessage(message);
}

void CADController::showSnapMarker(const gp_Pnt& point, int snapType, const QString& label)
{
    if (m_viewer) {
        m_viewer->setSnapMarker(point, snapType, label);
    }
}

void CADController::hideSnapMarker()
{
    if (m_viewer) {
        m_viewer->clearSnapMarker();
    }
}

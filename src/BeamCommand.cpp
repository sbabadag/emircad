#include "BeamCommand.h"
#include "OCCTViewer.h"
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <gp_Vec.hxx>
#include <gp_Trsf.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <gp_Ax1.hxx>
#include <gp_Dir.hxx>
#include <Standard_Failure.hxx>
#include <Precision.hxx>
#include <QMessageBox>
#include <cmath>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

BeamCommand::BeamCommand(const Handle(AIS_InteractiveContext)& context, OCCTViewer* viewer, QObject* parent)
    : CADCommand(context, viewer, parent)
    , m_width(200.0)
    , m_height(400.0)
    , m_useProfile(false)
    , m_profileType(SteelProfile::IPE)
    , m_profileSize("IPE 200")
{
}

void BeamCommand::execute(const gp_Pnt& point)
{
    m_points.append(point);
    
    if (m_points.size() == 1) {
        qDebug() << "Beam start point:" << point.X() << point.Y() << point.Z();
        emit statusUpdate(QString("Start point: (%1, %2, %3). Select end point")
                         .arg(point.X(), 0, 'f', 1)
                         .arg(point.Y(), 0, 'f', 1)
                         .arg(point.Z(), 0, 'f', 1));
    } else if (m_points.size() == 2) {
        // Log to file for debugging
        QFile logFile("beam_debug.log");
        if (logFile.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&logFile);
            out << "\n=== " << QDateTime::currentDateTime().toString() << " ===\n";
            out << "Start point: (" << m_points[0].X() << ", " << m_points[0].Y() << ", " << m_points[0].Z() << ")\n";
            out << "End point: (" << point.X() << ", " << point.Y() << ", " << point.Z() << ")\n";
            
            double distance = m_points[0].Distance(m_points[1]);
            out << "Distance: " << distance << " mm\n";
            
            if (m_useProfile) {
                out << "Profile: " << m_profileSize.toStdString().c_str() << "\n";
            } else {
                out << "Rectangular: " << m_width << " x " << m_height << " mm\n";
            }
            logFile.close();
        }
        
        qDebug() << "Beam end point:" << point.X() << point.Y() << point.Z();
        qDebug() << "Creating beam from" << m_points[0].X() << m_points[0].Y() << m_points[0].Z() 
                 << "to" << m_points[1].X() << m_points[1].Y() << m_points[1].Z();
        clearPreview();
        
        // Check if points are not too close
        double distance = m_points[0].Distance(m_points[1]);
        qDebug() << "Distance between points:" << distance << "mm";
        
        if (distance < 1.0) {
            emit statusUpdate("Points too close - minimum distance is 1mm");
            m_points.clear();
            return;
        }
        
        try {
            TopoDS_Shape beam = createBeam(m_points[0], m_points[1]);
            
            if (!beam.IsNull()) {
                qDebug() << "Beam shape created successfully";
                displayShape(beam);
                emit statusUpdate(QString("Beam created: length %1 mm").arg(distance, 0, 'f', 1));
                emit commandCompleted(beam);
            } else {
                qDebug() << "ERROR: Beam shape is NULL";
                QString msg = "Failed to create beam - geometry error\nCheck beam_debug.log for details";
                QMessageBox::warning(nullptr, "Beam Creation Error", msg);
                emit statusUpdate("Failed to create beam - geometry error");
            }
        } catch (const Standard_Failure& e) {
            qDebug() << "OpenCascade exception:" << e.GetMessageString();
            QString msg = QString("OpenCascade Error: %1\nCheck beam_debug.log for details").arg(e.GetMessageString());
            QMessageBox::critical(nullptr, "Beam Creation Error", msg);
            emit statusUpdate(QString("Error creating beam: %1").arg(e.GetMessageString()));
        } catch (const std::exception& e) {
            qDebug() << "Standard exception:" << e.what();
            QString msg = QString("Error: %1\nCheck beam_debug.log for details").arg(e.what());
            QMessageBox::critical(nullptr, "Beam Creation Error", msg);
            emit statusUpdate(QString("Error creating beam: %1").arg(e.what()));
        } catch (...) {
            qDebug() << "Unknown exception in beam creation";
            QMessageBox::critical(nullptr, "Beam Creation Error", "Unknown error\nCheck beam_debug.log for details");
            emit statusUpdate("Unknown error creating beam");
        }
        
        m_points.clear();
    }
}

void BeamCommand::updatePreview(const gp_Pnt& point)
{
    // Only show preview line if we have exactly one start point
    if (m_points.size() == 1) {
        // Check if the distance is large enough to create a valid line
        double distance = m_points[0].Distance(point);
        
        // Only create preview if points are different enough
        if (distance > Precision::Confusion()) {
            // Use viewer overlay for smooth tracking line
            if (m_viewer) {
                m_viewer->setTrackingLine(m_points[0], point);
            }
            
            // Update status less frequently to reduce overhead
            static int updateCounter = 0;
            if (++updateCounter % 5 == 0) {
                emit statusUpdate(QString("Length: %1 mm").arg(distance, 0, 'f', 1));
            }
        }
    }
}

bool BeamCommand::isComplete() const
{
    return m_points.size() >= 2;
}

QString BeamCommand::getPrompt() const
{
    if (m_points.isEmpty()) {
        if (m_useProfile) {
            return QString("Select beam start point (Profile: %1)").arg(m_profileSize);
        } else {
            return QString("Select beam start point (Width: %1mm, Height: %2mm)")
                   .arg(m_width, 0, 'f', 0)
                   .arg(m_height, 0, 'f', 0);
        }
    }
    return QString("Select beam end point");
}

void BeamCommand::setDimensions(double width, double height)
{
    m_width = width;
    m_height = height;
    m_useProfile = false;
}

void BeamCommand::setProfile(SteelProfile::ProfileType type, const QString& size)
{
    m_profileType = type;
    m_profileSize = size;
    m_useProfile = true;
}

TopoDS_Shape BeamCommand::createBeam(const gp_Pnt& start, const gp_Pnt& end)
{
    // Use steel profile if selected
    if (m_useProfile) {
        return SteelProfile::createProfile(m_profileType, m_profileSize, start, end);
    }
    
    // Otherwise create simple rectangular beam
    // Calculate beam direction and length
    gp_Vec direction(start, end);
    double length = direction.Magnitude();
    
    // Create box at origin
    TopoDS_Shape box = BRepPrimAPI_MakeBox(length, m_width, m_height).Shape();
    
    // Transform to align with beam direction
    gp_Trsf transform;
    transform.SetTranslation(gp_Vec(0, -m_width/2, -m_height/2));
    BRepBuilderAPI_Transform translator1(box, transform, Standard_False);
    box = translator1.Shape();
    
    // Rotate to match direction
    if (length > 1e-6) {
        gp_Vec xAxis(1, 0, 0);
        direction.Normalize();
        
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
    
    // Move to start position
    gp_Trsf finalTransform;
    finalTransform.SetTranslation(gp_Vec(gp_Pnt(0,0,0), start));
    BRepBuilderAPI_Transform translator2(box, finalTransform, Standard_False);
    
    return translator2.Shape();
}

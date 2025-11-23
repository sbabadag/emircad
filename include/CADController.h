#ifndef CADCONTROLLER_H
#define CADCONTROLLER_H

#include <QObject>
#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>
#include "CADCommand.h"
#include "WorkPlane.h"
#include "SnapManager.h"
#include "OCCTViewer.h"

class TObjectCollection;

class CADController : public QObject
{
    Q_OBJECT

public:
    explicit CADController(const Handle(AIS_InteractiveContext)& context, OCCTViewer* viewer, TObjectCollection* collection, QObject* parent = nullptr);
    ~CADController();
    
    // Command management
    void startBeamCommand();
    void startColumnCommand();
    void startSlabCommand();
    void cancelCurrentCommand();
    
    // Event handling
    void handleClick(const gp_Pnt& point);
    void handleMove(const gp_Pnt& point);
    
    // View conversion
    gp_Pnt convertViewToWorld(int x, int y, const Handle(V3d_View)& view);
    
    // Work plane management
    void setWorkPlane(const WorkPlane& plane);
    WorkPlane getWorkPlane() const { return m_workPlane; }
    void setWorkPlaneVisible(bool visible);
    bool isWorkPlaneVisible() const { return m_showWorkPlane; }
    
    // Snap management
    SnapManager* getSnapManager() { return &m_snapManager; }
    void setSnapEnabled(bool enabled) { m_snapEnabled = enabled; }
    bool isSnapEnabled() const { return m_snapEnabled; }
    void showSnapMarker(const gp_Pnt& point, int snapType, const QString& label);
    void hideSnapMarker();
    
    // Access current command
    CADCommand* getCurrentCommand() const { return m_activeCommand; }
    
signals:
    void statusMessage(const QString& message);
    void commandChanged(const QString& commandName);
    
private slots:
    void onCommandCompleted(const TopoDS_Shape& shape);
    void onCommandCancelled();
    void onStatusUpdate(const QString& message);
    
private:
    void setActiveCommand(CADCommand* command);
    
    Handle(AIS_InteractiveContext) m_context;
    OCCTViewer* m_viewer;
    TObjectCollection* m_collection;
    CADCommand* m_activeCommand;
    
    // Work plane
    WorkPlane m_workPlane;
    Handle(AIS_Shape) m_workPlaneVisual;
    bool m_showWorkPlane;
    
    // Snap system
    SnapManager m_snapManager;
    bool m_snapEnabled;
};

#endif // CADCONTROLLER_H

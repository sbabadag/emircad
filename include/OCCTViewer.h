#ifndef OCCTVIEWER_H
#define OCCTVIEWER_H

#include <QWidget>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QShowEvent>
#include <QFocusEvent>
#include <QEvent>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <AIS_Point.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <Aspect_Handle.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <gp_Pnt.hxx>
#include <Geom_CartesianPoint.hxx>

#ifdef _WIN32
#include <WNT_Window.hxx>
#else
#include <Xw_Window.hxx>
#endif

struct OverlayLine {
    gp_Pnt start;
    gp_Pnt end;
    double r, g, b;
    double width;
};

struct OverlayMarker {
    gp_Pnt position;
    double r, g, b;
    double size;
    QString label;
};

class OCCTViewer : public QWidget
{
    Q_OBJECT

public:
    explicit OCCTViewer(QWidget *parent = nullptr);
    ~OCCTViewer();

    // Get the interactive context
    Handle(AIS_InteractiveContext) getContext() const { return m_context; }
    
    // Get the view
    Handle(V3d_View) getView() const { return m_view; }

    // View operations
    void fitAll();
    void setTopView();
    void setFrontView();
    void setRightView();
    void setIsometricView();
    void clearAll();
    
    // Request OCC redraw (call when adding/removing shapes)
    void requestRedraw();
    
    // OpenGL overlay drawing
    void setTrackingLine(const gp_Pnt& start, const gp_Pnt& end);
    void clearTrackingLine();
    void setSnapMarker(const gp_Pnt& position, int snapType, const QString& label = "");
    void setMultipleSnapMarkers(const QList<gp_Pnt>& positions); // Show all snap candidates
    void clearSnapMarker();
    void updateOverlay();
    
signals:
    void viewClicked(int x, int y, Qt::MouseButton button);
    void viewMouseMove(int x, int y, Qt::KeyboardModifiers modifiers);

protected:
    // Qt event handlers
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void changeEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    
    // Override to prevent Qt from using its paint engine
    QPaintEngine* paintEngine() const override { return nullptr; }

private:
    void initializeOCC();
    void setupViewer();
    gp_Pnt worldToScreen(const gp_Pnt& worldPoint);

    // OpenCascade objects
    Handle(V3d_Viewer) m_viewer;
    Handle(V3d_View) m_view;
    Handle(AIS_InteractiveContext) m_context;
    Handle(Aspect_DisplayConnection) m_displayConnection;
    Handle(OpenGl_GraphicDriver) m_graphicDriver;

    // Mouse interaction
    QPoint m_lastPos;
    bool m_isRotating;
    bool m_isPanning;
    bool m_isZooming;
    bool m_altWasPressed;  // Track Alt key state for highlighting
    
    // Overlay AIS objects (for tracking line and snap markers)
    Handle(AIS_Shape) m_trackingLineShape;
    Handle(AIS_Point) m_snapMarkerShape;  // Changed to AIS_Point for better visibility
    QList<Handle(AIS_Shape)> m_multipleSnapMarkers; // For showing all snap candidates
    
    // OpenGL overlay data
    bool m_hasTrackingLine;
    OverlayLine m_trackingLine;
    bool m_hasSnapMarker;
    OverlayMarker m_snapMarker;
    
    // Flag to track if OCC view needs redrawing
    bool m_occNeedsRedraw;
};

#endif // OCCTVIEWER_H

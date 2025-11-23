#include "OCCTViewer.h"
#include <AIS_Shape.hxx>
#include <Aspect_Handle.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Aspect_Window.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <Prs3d_Drawer.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Prs3d_PointAspect.hxx>
#include <Graphic3d_TransformPers.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRep_Builder.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Compound.hxx>
#include <Geom_CartesianPoint.hxx>
#include <GC_MakeCircle.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <AIS_Point.hxx>
#include <QDebug>

#ifdef _WIN32
#include <WNT_Window.hxx>
#include <windows.h>
#undef GetObject  // Undefine Windows macro that conflicts with TObjectCollection::FindObject
#endif

OCCTViewer::OCCTViewer(QWidget *parent)
    : QWidget(parent)
    , m_isRotating(false)
    , m_isPanning(false)
    , m_isZooming(false)
    , m_altWasPressed(false)
    , m_hasTrackingLine(false)
    , m_hasSnapMarker(false)
    , m_occNeedsRedraw(true)
{
    // Enable mouse tracking
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    
    // Set widget attributes to prevent flickering
    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_NativeWindow, true);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_NativeWindow);

    // Initialize OpenCascade
    initializeOCC();
}

OCCTViewer::~OCCTViewer()
{
}

void OCCTViewer::initializeOCC()
{
    // Create display connection
    m_displayConnection = new Aspect_DisplayConnection();

    // Create graphic driver
    m_graphicDriver = new OpenGl_GraphicDriver(m_displayConnection);

    // Create viewer
    m_viewer = new V3d_Viewer(m_graphicDriver);
    m_viewer->SetDefaultLights();
    m_viewer->SetLightOn();

    // Create view
    m_view = m_viewer->CreateView();

    // Create window for view
#ifdef _WIN32
    Handle(WNT_Window) wind = new WNT_Window((Aspect_Handle)winId());
#else
    Handle(Xw_Window) wind = new Xw_Window(m_displayConnection, (Window)winId());
#endif
    m_view->SetWindow(wind);
    
    if (!wind->IsMapped()) {
        wind->Map();
    }

    // Configure view settings
    m_view->SetBackgroundColor(Quantity_NOC_BLACK);
    m_view->MustBeResized();
    m_view->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_WHITE, 0.1);
    
    // Set up gradient background
    m_view->SetBgGradientColors(
        Quantity_Color(0.1, 0.1, 0.1, Quantity_TOC_RGB),
        Quantity_Color(0.3, 0.3, 0.3, Quantity_TOC_RGB),
        Aspect_GFM_VER
    );

    // Create interactive context
    m_context = new AIS_InteractiveContext(m_viewer);
    m_context->SetDisplayMode(AIS_Shaded, Standard_True);

    // Configure selection and highlighting
    Handle(Prs3d_Drawer) drawer = m_context->DefaultDrawer();
    Handle(Prs3d_Drawer) selDrawer = m_context->SelectionStyle();
    Handle(Prs3d_Drawer) hilightDrawer = m_context->HighlightStyle();

    // Set highlight color to blue
    hilightDrawer->SetColor(Quantity_NOC_DODGERBLUE1);
    hilightDrawer->SetDisplayMode(1);
    hilightDrawer->SetTransparency(0.0f);

    // Set selection color to green
    selDrawer->SetColor(Quantity_NOC_GREEN);
    selDrawer->SetDisplayMode(1);
    selDrawer->SetTransparency(0.0f);

    // Set default view
    setIsometricView();
}

void OCCTViewer::setupViewer()
{
    m_view->MustBeResized();
    m_view->Redraw();
}

void OCCTViewer::paintEvent(QPaintEvent *event)
{
    // Force OpenCascade window to redraw
    if (!m_view.IsNull()) {
        m_view->Invalidate();
        m_view->Redraw();
    }
}

void OCCTViewer::resizeEvent(QResizeEvent *event)
{
    if (!m_view.IsNull()) {
        m_view->MustBeResized();
        m_view->Redraw();
    }
}

void OCCTViewer::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_view.IsNull()) {
        m_view->MustBeResized();
        m_view->Invalidate();
        m_view->Redraw();
    }
}

void OCCTViewer::focusInEvent(QFocusEvent *event)
{
    QWidget::focusInEvent(event);
    if (!m_view.IsNull()) {
        m_view->Invalidate();
        m_view->Redraw();
    }
}

void OCCTViewer::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    // Redraw when window state changes (minimize/restore, activate/deactivate)
    if (event->type() == QEvent::WindowStateChange || 
        event->type() == QEvent::ActivationChange) {
        if (!m_view.IsNull()) {
            m_view->MustBeResized();
            m_view->Invalidate();
            m_view->Redraw();
        }
    }
}

void OCCTViewer::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->pos();

    if (event->button() == Qt::LeftButton) {
        // Check if Ctrl is pressed for selection/command mode
        if (event->modifiers() & Qt::ControlModifier) {
            emit viewClicked(event->pos().x(), event->pos().y(), event->button());
        } else {
            m_isRotating = true;
            m_view->StartRotation(event->pos().x(), event->pos().y());
        }
    }
    else if (event->button() == Qt::MiddleButton) {
        // Middle button + Shift = Rotate, otherwise Pan
        if (event->modifiers() & Qt::ShiftModifier) {
            m_isRotating = true;
            m_view->StartRotation(event->pos().x(), event->pos().y());
        } else {
            m_isPanning = true;
        }
    }
    else if (event->button() == Qt::RightButton) {
        // Context menu or selection
        m_context->MoveTo(event->pos().x(), event->pos().y(), m_view, Standard_True);
        m_context->Select(Standard_True);
    }
}

void OCCTViewer::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isRotating = false;
    }
    else if (event->button() == Qt::MiddleButton) {
        m_isRotating = false;
        m_isPanning = false;
    }
}

void OCCTViewer::mouseMoveEvent(QMouseEvent *event)
{
    QPoint currentPos = event->pos();

    if (m_isRotating) {
        // Rotate view
        m_view->Rotation(currentPos.x(), currentPos.y());
        m_view->Redraw();
    }
    else if (m_isPanning) {
        // Pan view
        int dx = currentPos.x() - m_lastPos.x();
        int dy = currentPos.y() - m_lastPos.y();
        m_view->Pan(dx, -dy);
        m_view->Redraw();
    }
    else {
        // Check if Alt key is pressed for highlighting
        bool altPressed = (event->modifiers() & Qt::AltModifier);
        
        if (altPressed) {
            // Use MoveTo to detect objects under cursor and highlight them
            m_context->MoveTo(event->pos().x(), event->pos().y(), m_view, Standard_True);
            m_altWasPressed = true;
        } else if (m_altWasPressed) {
            // Only clear highlighting when Alt is released (transition from pressed to not pressed)
            m_context->ClearDetected(Standard_True);
            m_altWasPressed = false;
        }
        
        // Emit mouse move for preview line drawing with keyboard modifiers
        emit viewMouseMove(currentPos.x(), currentPos.y(), event->modifiers());
    }

    m_lastPos = currentPos;
}

void OCCTViewer::wheelEvent(QWheelEvent *event)
{
    // Zoom in/out
    int delta = event->angleDelta().y();
    
    if (delta > 0) {
        m_view->SetZoom(1.1);
    }
    else {
        m_view->SetZoom(0.9);
    }
    
    m_view->Redraw();
}

void OCCTViewer::fitAll()
{
    if (!m_view.IsNull()) {
        m_view->FitAll();
        m_view->ZFitAll();
        m_view->Redraw();
    }
}

void OCCTViewer::setTopView()
{
    if (!m_view.IsNull()) {
        m_view->SetProj(V3d_Zpos);
        fitAll();
    }
}

void OCCTViewer::setFrontView()
{
    if (!m_view.IsNull()) {
        m_view->SetProj(V3d_Yneg);
        fitAll();
    }
}

void OCCTViewer::setRightView()
{
    if (!m_view.IsNull()) {
        m_view->SetProj(V3d_Xpos);
        fitAll();
    }
}

void OCCTViewer::setIsometricView()
{
    if (!m_view.IsNull()) {
        m_view->SetProj(V3d_XposYnegZpos);
        fitAll();
    }
}

void OCCTViewer::clearAll()
{
    if (!m_context.IsNull()) {
        m_context->RemoveAll(Standard_False);
        m_view->Redraw();
    }
}

// OpenGL overlay functions
void OCCTViewer::setTrackingLine(const gp_Pnt& start, const gp_Pnt& end)
{
    // Remove old tracking line if exists
    if (!m_trackingLineShape.IsNull()) {
        m_context->Remove(m_trackingLineShape, Standard_False);
    }
    
    // Create new line
    TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(start, end);
    m_trackingLineShape = new AIS_Shape(edge);
    
    // Configure display attributes
    Handle(Prs3d_Drawer) drawer = m_trackingLineShape->Attributes();
    drawer->SetLineAspect(new Prs3d_LineAspect(Quantity_NOC_YELLOW, Aspect_TOL_SOLID, 3.0));
    drawer->SetWireAspect(new Prs3d_LineAspect(Quantity_NOC_YELLOW, Aspect_TOL_SOLID, 3.0));
    
    // Set color and display mode
    m_context->SetColor(m_trackingLineShape, Quantity_NOC_YELLOW, Standard_False);
    m_context->SetDisplayMode(m_trackingLineShape, 0, Standard_False); // Wireframe mode
    m_context->SetWidth(m_trackingLineShape, 3.0, Standard_False);
    
    // Display with immediate update
    m_context->Display(m_trackingLineShape, Standard_True);
    m_hasTrackingLine = true;
}

void OCCTViewer::clearTrackingLine()
{
    if (!m_trackingLineShape.IsNull()) {
        m_context->Remove(m_trackingLineShape, Standard_True);
        m_trackingLineShape.Nullify();
        m_hasTrackingLine = false;
    }
}

void OCCTViewer::setSnapMarker(const gp_Pnt& position, int snapType, const QString& label)
{
    // Safety check
    if (m_view.IsNull() || m_context.IsNull()) {
        return;
    }
    
    try {
        // Remove old snap marker if exists
        if (!m_snapMarkerShape.IsNull()) {
            m_context->Remove(m_snapMarkerShape, Standard_False);
            m_snapMarkerShape.Nullify();
        }
        
        // Create a Geom_CartesianPoint
        Handle(Geom_CartesianPoint) aPoint = new Geom_CartesianPoint(position);
        if (aPoint.IsNull()) {
            return;
        }
        
        // Create AIS_Point - this is specifically designed for point markers
        m_snapMarkerShape = new AIS_Point(aPoint);
        if (m_snapMarkerShape.IsNull()) {
            return;
        }
        
        // Set marker style and size based on snap type
        Quantity_Color markerColor;
        Aspect_TypeOfMarker markerType;
        
        switch(snapType) {
            case 0x01: // Endpoint
                markerColor = Quantity_NOC_GREEN;
                markerType = Aspect_TOM_O_PLUS;  // Circle with plus
                break;
            case 0x02: // Midpoint
                markerColor = Quantity_NOC_CYAN1;
                markerType = Aspect_TOM_O_STAR;  // Circle with star
                break;
            case 0x04: // Center
                markerColor = Quantity_NOC_RED;
                markerType = Aspect_TOM_RING1;  // Ring
                break;
            default:
                markerColor = Quantity_NOC_YELLOW;
                markerType = Aspect_TOM_X;  // X mark
                break;
        }
        
        // Configure the point marker with error handling
        Handle(Prs3d_Drawer) drawer = m_snapMarkerShape->Attributes();
        if (!drawer.IsNull()) {
            Handle(Prs3d_PointAspect) pointAspect = drawer->PointAspect();
            if (pointAspect.IsNull()) {
                pointAspect = new Prs3d_PointAspect(markerType, markerColor, 3.0);
                drawer->SetPointAspect(pointAspect);
            } else {
                pointAspect->SetTypeOfMarker(markerType);
                pointAspect->SetColor(markerColor);
                pointAspect->SetScale(3.0);  // Make it 3x larger than default
            }
        }
        
        // Display with topmost Z-layer
        m_snapMarkerShape->SetZLayer(Graphic3d_ZLayerId_Topmost);
        m_context->Display(m_snapMarkerShape, Standard_False);
        m_context->Deactivate(m_snapMarkerShape);
        m_hasSnapMarker = true;
        
        // Force view update to show the marker
        m_view->Redraw();
    }
    catch (Standard_Failure const& ex) {
        qDebug() << "Exception in setSnapMarker:" << ex.GetMessageString();
    }
    catch (...) {
        qDebug() << "Unknown exception in setSnapMarker";
    }
}

void OCCTViewer::clearSnapMarker()
{
    try {
        if (!m_snapMarkerShape.IsNull() && !m_context.IsNull()) {
            m_context->Remove(m_snapMarkerShape, Standard_False);
            m_snapMarkerShape.Nullify();
            m_hasSnapMarker = false;
            
            // Force view update to clear the marker
            m_view->Redraw();
        }
    }
    catch (Standard_Failure const& ex) {
        qDebug() << "Exception in clearSnapMarker:" << ex.GetMessageString();
    }
    catch (...) {
        qDebug() << "Unknown exception in clearSnapMarker";
    }
    
    // Clear multiple snap markers
    for (Handle(AIS_Shape)& marker : m_multipleSnapMarkers) {
        if (!marker.IsNull()) {
            m_context->Remove(marker, Standard_False);
        }
    }
    m_multipleSnapMarkers.clear();
    
    if (!m_multipleSnapMarkers.isEmpty()) {
        m_view->Redraw();
    }
}

void OCCTViewer::setMultipleSnapMarkers(const QList<gp_Pnt>& positions)
{
    // Clear existing markers
    for (Handle(AIS_Shape)& marker : m_multipleSnapMarkers) {
        if (!marker.IsNull()) {
            m_context->Remove(marker, Standard_False);
        }
    }
    m_multipleSnapMarkers.clear();
    
    // Create a marker for each position
    for (const gp_Pnt& pos : positions) {
        TopoDS_Vertex vertex = BRepBuilderAPI_MakeVertex(pos);
        Handle(AIS_Shape) marker = new AIS_Shape(vertex);
        
        // Set cyan color for debug markers (different from main snap marker)
        m_context->SetColor(marker, Quantity_NOC_CYAN, Standard_False);
        
        Handle(Prs3d_Drawer) drawer = marker->Attributes();
        drawer->SetPointAspect(new Prs3d_PointAspect(Aspect_TOM_O_POINT, Quantity_NOC_CYAN, 8.0));
        
        m_context->Display(marker, Standard_False);
        m_multipleSnapMarkers.append(marker);
    }
    
    // Single update after all markers added
    if (!positions.isEmpty()) {
        m_view->Redraw();
    }
}

void OCCTViewer::requestRedraw()
{
    if (!m_view.IsNull()) {
        m_view->Redraw();
    }
}

void OCCTViewer::updateOverlay()
{
    update();  // Trigger repaint to redraw overlays
}

gp_Pnt OCCTViewer::worldToScreen(const gp_Pnt& worldPoint)
{
    Standard_Integer aWinX, aWinY;
    
    // Project 3D point to 2D window coordinates
    m_view->Convert(worldPoint.X(), worldPoint.Y(), worldPoint.Z(), aWinX, aWinY);
    
    return gp_Pnt(aWinX, aWinY, 0);
}


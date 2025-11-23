#include "CADCommand.h"
#include "OCCTViewer.h"
#include <Quantity_Color.hxx>

CADCommand::CADCommand(const Handle(AIS_InteractiveContext)& context, OCCTViewer* viewer, QObject* parent)
    : QObject(parent)
    , m_context(context)
    , m_viewer(viewer)
    , m_active(false)
{
}

CADCommand::~CADCommand()
{
    clearPreview();
}

void CADCommand::start()
{
    m_active = true;
    emit statusUpdate(getPrompt());
}

void CADCommand::cancel()
{
    clearPreview();
    m_active = false;
    emit commandCancelled();
    emit statusUpdate("Command cancelled");
}

void CADCommand::displayShape(const TopoDS_Shape& shape)
{
    Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
    m_context->Display(aisShape, Standard_True);  // Immediate update
    if (m_viewer) {
        m_viewer->requestRedraw();
    }
}

void CADCommand::showPreview(const TopoDS_Shape& shape)
{
    // Check if we can reuse the existing shape
    if (!m_previewShape.IsNull()) {
        // Update the shape directly instead of removing/recreating
        m_previewShape->Set(shape);
        m_context->Redisplay(m_previewShape, Standard_False);
    } else {
        // Create new preview shape
        m_previewShape = new AIS_Shape(shape);
        m_previewShape->SetColor(Quantity_Color(1.0, 1.0, 0.0, Quantity_TOC_RGB));
        m_previewShape->SetTransparency(0.5);
        m_previewShape->SetWidth(3.0);
        m_context->Display(m_previewShape, Standard_False);
    }
    
    // Trigger update of the viewer
    m_context->CurrentViewer()->Update();
}

void CADCommand::clearPreview()
{
    if (!m_previewShape.IsNull()) {
        m_context->Remove(m_previewShape, Standard_False);
        m_previewShape.Nullify();
        m_context->CurrentViewer()->Update();
    }
}

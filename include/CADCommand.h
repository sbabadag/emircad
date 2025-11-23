#ifndef CADCOMMAND_H
#define CADCOMMAND_H

#include <QObject>
#include <QString>
#include <gp_Pnt.hxx>
#include <TopoDS_Shape.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>

class OCCTViewer;

class CADCommand : public QObject
{
    Q_OBJECT

public:
    explicit CADCommand(const Handle(AIS_InteractiveContext)& context, OCCTViewer* viewer, QObject* parent = nullptr);
    virtual ~CADCommand();
    
    virtual void start();
    virtual void execute(const gp_Pnt& point) = 0;
    virtual void updatePreview(const gp_Pnt& point) {}  // For dynamic preview during mouse move
    virtual void cancel();
    virtual bool isComplete() const = 0;
    virtual QString getPrompt() const = 0;
    
signals:
    void commandCompleted(const TopoDS_Shape& shape);
    void commandCancelled();
    void statusUpdate(const QString& message);
    
protected:
    void displayShape(const TopoDS_Shape& shape);
    void showPreview(const TopoDS_Shape& shape);
    void clearPreview();
    
    Handle(AIS_InteractiveContext) m_context;
    OCCTViewer* m_viewer;
    Handle(AIS_Shape) m_previewShape;
    bool m_active;
};

#endif // CADCOMMAND_H

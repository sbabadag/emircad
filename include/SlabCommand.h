#ifndef SLABCOMMAND_H
#define SLABCOMMAND_H

#include "CADCommand.h"
#include <QList>

class OCCTViewer;

class SlabCommand : public CADCommand
{
    Q_OBJECT

public:
    explicit SlabCommand(const Handle(AIS_InteractiveContext)& context, OCCTViewer* viewer, QObject* parent = nullptr);
    
    void execute(const gp_Pnt& point) override;
    void updatePreview(const gp_Pnt& point) override;
    bool isComplete() const override;
    QString getPrompt() const override;
    
    void setThickness(double thickness);

private:
    TopoDS_Shape createSlab(const gp_Pnt& corner1, const gp_Pnt& corner2);
    
    QList<gp_Pnt> m_points;
    double m_thickness;
};

#endif // SLABCOMMAND_H

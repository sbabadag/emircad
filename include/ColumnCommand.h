#ifndef COLUMNCOMMAND_H
#define COLUMNCOMMAND_H

#include "CADCommand.h"

class OCCTViewer;

class ColumnCommand : public CADCommand
{
    Q_OBJECT

public:
    explicit ColumnCommand(const Handle(AIS_InteractiveContext)& context, OCCTViewer* viewer, QObject* parent = nullptr);
    
    void execute(const gp_Pnt& point) override;
    void updatePreview(const gp_Pnt& point) override;
    bool isComplete() const override;
    QString getPrompt() const override;
    
    void setDimensions(double width, double depth, double height);

private:
    TopoDS_Shape createColumn(const gp_Pnt& basePoint);
    
    gp_Pnt m_basePoint;
    bool m_pointSet;
    double m_width;
    double m_depth;
    double m_height;
};

#endif // COLUMNCOMMAND_H

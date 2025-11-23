#ifndef TCOLUMN_H
#define TCOLUMN_H

#include "TGraphicObject.h"
#include <gp_Pnt.hxx>

class TColumn;
DEFINE_STANDARD_HANDLE(TColumn, TGraphicObject)

class TColumn : public TGraphicObject
{
public:
    Standard_EXPORT TColumn();
    Standard_EXPORT TColumn(const gp_Pnt& basePoint, double width, double depth, double height);
    Standard_EXPORT virtual ~TColumn();
    
    DEFINE_STANDARD_RTTIEXT(TColumn, TGraphicObject)
    
    Standard_EXPORT virtual ObjectType GetType() const override { return TYPE_COLUMN; }
    Standard_EXPORT virtual QString GetTypeName() const override { return "Column"; }
    Standard_EXPORT virtual TopoDS_Shape BuildShape() override;
    Standard_EXPORT virtual Handle(AIS_Shape) GetAISShape() override;
    
    Standard_EXPORT void SetBasePoint(const gp_Pnt& point);
    Standard_EXPORT gp_Pnt GetBasePoint() const { return m_basePoint; }
    
    Standard_EXPORT void SetDimensions(double width, double depth, double height);
    Standard_EXPORT void GetDimensions(double& width, double& depth, double& height) const;
    
    Standard_EXPORT virtual QString Serialize() const override;
    Standard_EXPORT virtual bool IsValid() const override;

protected:
    gp_Pnt m_basePoint;
    double m_width;
    double m_depth;
    double m_height;
};

#endif // TCOLUMN_H

#ifndef TSLAB_H
#define TSLAB_H

#include "TGraphicObject.h"
#include <gp_Pnt.hxx>

class TSlab;
DEFINE_STANDARD_HANDLE(TSlab, TGraphicObject)

class TSlab : public TGraphicObject
{
public:
    Standard_EXPORT TSlab();
    Standard_EXPORT TSlab(const gp_Pnt& corner1, const gp_Pnt& corner2, double thickness);
    Standard_EXPORT virtual ~TSlab();
    
    DEFINE_STANDARD_RTTIEXT(TSlab, TGraphicObject)
    
    Standard_EXPORT virtual ObjectType GetType() const override { return TYPE_SLAB; }
    Standard_EXPORT virtual QString GetTypeName() const override { return "Slab"; }
    Standard_EXPORT virtual TopoDS_Shape BuildShape() override;
    Standard_EXPORT virtual Handle(AIS_Shape) GetAISShape() override;
    
    Standard_EXPORT void SetCorners(const gp_Pnt& corner1, const gp_Pnt& corner2);
    Standard_EXPORT void GetCorners(gp_Pnt& corner1, gp_Pnt& corner2) const;
    
    Standard_EXPORT void SetThickness(double thickness);
    Standard_EXPORT double GetThickness() const { return m_thickness; }
    
    Standard_EXPORT double GetArea() const;
    
    Standard_EXPORT virtual QString Serialize() const override;
    Standard_EXPORT virtual bool IsValid() const override;

protected:
    gp_Pnt m_corner1;
    gp_Pnt m_corner2;
    double m_thickness;
};

#endif // TSLAB_H

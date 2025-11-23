#ifndef TBEAM_H
#define TBEAM_H

#include "TGraphicObject.h"
#include "SteelProfile.h"
#include <gp_Pnt.hxx>

// Forward declaration for OCCT handle system
class TBeam;
DEFINE_STANDARD_HANDLE(TBeam, TGraphicObject)

/**
 * @brief Beam object class derived from TGraphicObject
 */
class TBeam : public TGraphicObject
{
public:
    // Constructor
    Standard_EXPORT TBeam();
    Standard_EXPORT TBeam(const gp_Pnt& startPoint, const gp_Pnt& endPoint);
    
    // Destructor
    Standard_EXPORT virtual ~TBeam();
    
    // Type information
    DEFINE_STANDARD_RTTIEXT(TBeam, TGraphicObject)
    
    // Override virtual methods
    Standard_EXPORT virtual ObjectType GetType() const override { return TYPE_BEAM; }
    Standard_EXPORT virtual QString GetTypeName() const override { return "Beam"; }
    Standard_EXPORT virtual TopoDS_Shape BuildShape() override;
    Standard_EXPORT virtual Handle(AIS_Shape) GetAISShape() override;
    
    // Beam-specific properties
    Standard_EXPORT void SetStartPoint(const gp_Pnt& point);
    Standard_EXPORT gp_Pnt GetStartPoint() const { return m_startPoint; }
    
    Standard_EXPORT void SetEndPoint(const gp_Pnt& point);
    Standard_EXPORT gp_Pnt GetEndPoint() const { return m_endPoint; }
    
    Standard_EXPORT double GetLength() const;
    Standard_EXPORT gp_Vec GetDirection() const;
    
    // Cross-section options
    Standard_EXPORT void SetRectangularSection(double width, double height);
    Standard_EXPORT void SetProfileSection(SteelProfile::ProfileType type, const QString& size);
    
    Standard_EXPORT bool IsProfileSection() const { return m_useProfile; }
    Standard_EXPORT SteelProfile::ProfileType GetProfileType() const { return m_profileType; }
    Standard_EXPORT QString GetProfileSize() const { return m_profileSize; }
    
    Standard_EXPORT void GetSectionDimensions(double& width, double& height) const;
    
    // Override serialization
    Standard_EXPORT virtual QString Serialize() const override;
    Standard_EXPORT virtual bool Deserialize(const QString& data) override;
    
    // Override validation
    Standard_EXPORT virtual bool IsValid() const override;

protected:
    // Snap point calculation
    Standard_EXPORT void CalculateSnapPoints();

    gp_Pnt m_startPoint;
    gp_Pnt m_endPoint;
    double m_sectionWidth;
    double m_sectionHeight;
    bool m_useProfile;
    SteelProfile::ProfileType m_profileType;
    QString m_profileSize;
};

#endif // TBEAM_H

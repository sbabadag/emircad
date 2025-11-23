#ifndef BEAMCOMMAND_H
#define BEAMCOMMAND_H

#include "CADCommand.h"
#include "SteelProfile.h"
#include <QList>

class OCCTViewer;

class BeamCommand : public CADCommand
{
    Q_OBJECT

public:
    explicit BeamCommand(const Handle(AIS_InteractiveContext)& context, OCCTViewer* viewer, QObject* parent = nullptr);
    
    void execute(const gp_Pnt& point) override;
    void updatePreview(const gp_Pnt& point) override;
    bool isComplete() const override;
    QString getPrompt() const override;
    
    void setDimensions(double width, double height);
    void setProfile(SteelProfile::ProfileType type, const QString& size);
    
    // Get last created beam parameters
    gp_Pnt getLastStartPoint() const { return m_points.size() >= 2 ? m_points[m_points.size()-2] : gp_Pnt(); }
    gp_Pnt getLastEndPoint() const { return m_points.size() >= 1 ? m_points.last() : gp_Pnt(); }
    double getWidth() const { return m_width; }
    double getHeight() const { return m_height; }
    bool usesProfile() const { return m_useProfile; }
    SteelProfile::ProfileType getProfileType() const { return m_profileType; }
    QString getProfileSize() const { return m_profileSize; }

private:
    TopoDS_Shape createBeam(const gp_Pnt& start, const gp_Pnt& end);
    
    QList<gp_Pnt> m_points;
    double m_width;
    double m_height;
    bool m_useProfile;
    SteelProfile::ProfileType m_profileType;
    QString m_profileSize;
};

#endif // BEAMCOMMAND_H

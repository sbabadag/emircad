#ifndef WORKPLANEDIALOG_H
#define WORKPLANEDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include "WorkPlane.h"

class WorkPlaneDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WorkPlaneDialog(QWidget* parent = nullptr);
    
    WorkPlane::PlaneType getSelectedPlaneType() const;
    double getOffset() const;
    bool isVisible() const;

private:
    void setupUI();
    void applyStyles();
    
    QComboBox* m_planeTypeCombo;
    QDoubleSpinBox* m_offsetSpin;
    QCheckBox* m_visibleCheck;
};

#endif // WORKPLANEDIALOG_H

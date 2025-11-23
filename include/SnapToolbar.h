#ifndef SNAPTOOLBAR_H
#define SNAPTOOLBAR_H

#include <QWidget>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include "SnapManager.h"

class SnapToolbar : public QWidget
{
    Q_OBJECT

public:
    explicit SnapToolbar(QWidget *parent = nullptr);
    ~SnapToolbar();
    
    // Get enabled snap types
    int getEnabledSnapTypes() const;
    
    // Set enabled snap types
    void setEnabledSnapTypes(int types);
    
signals:
    void snapTypesChanged(int types);
    
private slots:
    void onSnapCheckChanged();
    void onToggleAllClicked();
    
private:
    void setupUI();
    void updateSnapTypes();
    
    QCheckBox* m_endpointCheck;
    QCheckBox* m_midpointCheck;
    QCheckBox* m_centerCheck;
    QCheckBox* m_vertexCheck;
    QCheckBox* m_nearestCheck;
    QPushButton* m_toggleAllBtn;
    
    bool m_allEnabled;
};

#endif // SNAPTOOLBAR_H

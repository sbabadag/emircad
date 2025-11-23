#ifndef PROPERTIESPANEL_H
#define PROPERTIESPANEL_H

#include <QDockWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QScrollArea>
#include <QCheckBox>
#include <QTabWidget>
#include "TGraphicObject.h"
#include "TBeam.h"
#include "TColumn.h"
#include "TSlab.h"

class PropertiesPanel : public QDockWidget
{
    Q_OBJECT

public:
    explicit PropertiesPanel(QWidget* parent = nullptr);
    ~PropertiesPanel();
    
    void setObject(const Handle(TGraphicObject)& object);
    void clearProperties();
    void setMultipleSelection(int count);

signals:
    void propertyChanged(int objectID);
    void colorChanged(int objectID, int r, int g, int b);
    void layerChanged(int objectID, const QString& layer);
    void nameChanged(int objectID, const QString& name);

private slots:
    void onNameChanged();
    void onDescriptionChanged();
    void onLayerChanged(int index);
    void onMaterialChanged();
    void onColorButtonClicked();
    void onApplyProperties();
    void onVisibilityChanged(int state);
    void onLockChanged(int state);
    
    // Type-specific slots
    void onBeamPropertyChanged();
    void onColumnPropertyChanged();
    void onSlabPropertyChanged();

private:
    void setupUI();
    void createPropertiesTab();
    void createGeometryTab();
    void createMaterialsTab();
    void createAttributesTab();
    void createCommonPropertiesGroup();
    void createGeometryGroup();
    void createTypeSpecificGroup();
    void createStatisticsGroup();
    
    void updateCommonProperties();
    void updateGeometryProperties();
    void updateBeamProperties();
    void updateColumnProperties();
    void updateSlabProperties();
    void updateStatistics();
    
    void enableEditing(bool enabled);

private:
    // Main layout
    QWidget* m_mainWidget;
    QTabWidget* m_tabWidget;
    QVBoxLayout* m_mainLayout;
    
    // Tab pages
    QWidget* m_propertiesTab;
    QWidget* m_geometryTab;
    QWidget* m_materialsTab;
    QWidget* m_attributesTab;
    
    // Properties tab widgets
    QScrollArea* m_propertiesScrollArea;
    QGroupBox* m_commonGroup;
    QLineEdit* m_nameEdit;
    QLineEdit* m_descriptionEdit;
    QComboBox* m_layerCombo;
    QLineEdit* m_materialEdit;
    QLabel* m_objectTypeLabel;
    QLabel* m_objectIDLabel;
    QPushButton* m_colorButton;
    QCheckBox* m_visibleCheckBox;
    QCheckBox* m_lockedCheckBox;
    
    // Geometry group
    QGroupBox* m_geometryGroup;
    QLabel* m_centerPointLabel;
    QLabel* m_boundingBoxLabel;
    
    // Type-specific group
    QGroupBox* m_typeSpecificGroup;
    QVBoxLayout* m_typeSpecificLayout;
    
    // Beam properties
    QLabel* m_beamStartLabel;
    QLabel* m_beamEndLabel;
    QLabel* m_beamLengthLabel;
    QLabel* m_beamDirectionLabel;
    QComboBox* m_beamProfileTypeCombo;
    QComboBox* m_beamProfileSizeCombo;
    QDoubleSpinBox* m_beamWidthSpin;
    QDoubleSpinBox* m_beamHeightSpin;
    QPushButton* m_beamUseProfileButton;
    QPushButton* m_beamUseRectangularButton;
    
    // Column properties
    QLabel* m_columnBaseLabel;
    QDoubleSpinBox* m_columnWidthSpin;
    QDoubleSpinBox* m_columnDepthSpin;
    QDoubleSpinBox* m_columnHeightSpin;
    
    // Slab properties
    QLabel* m_slabCorner1Label;
    QLabel* m_slabCorner2Label;
    QDoubleSpinBox* m_slabThicknessSpin;
    QLabel* m_slabAreaLabel;
    
    // Statistics group
    QGroupBox* m_statisticsGroup;
    QLabel* m_volumeLabel;
    QLabel* m_surfaceAreaLabel;
    QLabel* m_creationTimeLabel;
    QLabel* m_modificationTimeLabel;
    
    // Apply button
    QPushButton* m_applyButton;
    
    // Current object
    Handle(TGraphicObject) m_currentObject;
    bool m_updatingUI;
    int m_selectedColor[3];
};

#endif // PROPERTIESPANEL_H

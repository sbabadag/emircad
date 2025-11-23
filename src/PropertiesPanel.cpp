#include "PropertiesPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QColorDialog>
#include <QMessageBox>
#include <QCheckBox>

PropertiesPanel::PropertiesPanel(QWidget* parent)
    : QDockWidget("Properties", parent)
    , m_updatingUI(false)
{
    m_selectedColor[0] = 200;
    m_selectedColor[1] = 200;
    m_selectedColor[2] = 200;
    
    setupUI();
    clearProperties();
    
    setFeatures(QDockWidget::DockWidgetMovable | 
                QDockWidget::DockWidgetFloatable |
                QDockWidget::DockWidgetClosable);
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
}

PropertiesPanel::~PropertiesPanel()
{
}

void PropertiesPanel::setupUI()
{
    // Create main widget with tab widget
    m_mainWidget = new QWidget(this);
    m_mainLayout = new QVBoxLayout(m_mainWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Create tab widget (Tekla-style)
    m_tabWidget = new QTabWidget(m_mainWidget);
    m_tabWidget->setDocumentMode(true);
    m_mainLayout->addWidget(m_tabWidget);
    
    // Create tab pages
    createPropertiesTab();
    createGeometryTab();
    createMaterialsTab();
    createAttributesTab();
    
    setWidget(m_mainWidget);
    
    // Apply Tekla-like styling
    QString tabStyle = R"(
        QTabWidget::pane {
            border: 1px solid #555;
            background: #2b2b2b;
        }
        QTabBar::tab {
            background: #3c3c3c;
            color: #ffffff;
            padding: 8px 20px;
            margin-right: 2px;
            border: 1px solid #555;
        }
        QTabBar::tab:selected {
            background: #2b2b2b;
            border-bottom: 2px solid #0078d4;
        }
        QTabBar::tab:hover {
            background: #404040;
        }
    )";
    m_tabWidget->setStyleSheet(tabStyle);
}

void PropertiesPanel::createPropertiesTab()
{
    m_propertiesTab = new QWidget();
    m_propertiesScrollArea = new QScrollArea();
    m_propertiesScrollArea->setWidgetResizable(true);
    
    QWidget* scrollContent = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(scrollContent);
    layout->setSpacing(10);
    
    // Create property groups
    createCommonPropertiesGroup();
    createTypeSpecificGroup();
    
    layout->addWidget(m_commonGroup);
    layout->addWidget(m_typeSpecificGroup);
    layout->addStretch();
    
    // Apply button
    m_applyButton = new QPushButton("Apply Changes", scrollContent);
    m_applyButton->setEnabled(false);
    m_applyButton->setStyleSheet("QPushButton { background: #0078d4; color: white; padding: 8px; border: none; } QPushButton:hover { background: #1084d8; }");
    connect(m_applyButton, &QPushButton::clicked, this, &PropertiesPanel::onApplyProperties);
    layout->addWidget(m_applyButton);
    layout->addStretch();
    
    m_propertiesScrollArea->setWidget(scrollContent);
    
    QVBoxLayout* tabLayout = new QVBoxLayout(m_propertiesTab);
    tabLayout->setContentsMargins(0, 0, 0, 0);
    tabLayout->addWidget(m_propertiesScrollArea);
    
    m_tabWidget->addTab(m_propertiesTab, "Properties");
}

void PropertiesPanel::createGeometryTab()
{
    m_geometryTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_geometryTab);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);
    
    createGeometryGroup();
    createStatisticsGroup();
    
    layout->addWidget(m_geometryGroup);
    layout->addWidget(m_statisticsGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(m_geometryTab, "Geometry");
}

void PropertiesPanel::createMaterialsTab()
{
    m_materialsTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_materialsTab);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);
    
    // Material properties group
    QGroupBox* materialGroup = new QGroupBox("Material Properties", m_materialsTab);
    QFormLayout* form = new QFormLayout(materialGroup);
    form->setSpacing(8);
    
    m_materialEdit = new QLineEdit(materialGroup);
    connect(m_materialEdit, &QLineEdit::textChanged, this, &PropertiesPanel::onMaterialChanged);
    form->addRow("Material:", m_materialEdit);
    
    QLabel* gradeLabel = new QLabel("S355");
    form->addRow("Grade:", gradeLabel);
    
    QLabel* finishLabel = new QLabel("Painted");
    form->addRow("Finish:", finishLabel);
    
    m_colorButton = new QPushButton("Select Color", materialGroup);
    m_colorButton->setMinimumHeight(30);
    connect(m_colorButton, &QPushButton::clicked, this, &PropertiesPanel::onColorButtonClicked);
    form->addRow("Color:", m_colorButton);
    
    layout->addWidget(materialGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(m_materialsTab, "Materials");
}

void PropertiesPanel::createAttributesTab()
{
    m_attributesTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_attributesTab);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);
    
    // User attributes group
    QGroupBox* attrGroup = new QGroupBox("User Attributes", m_attributesTab);
    QFormLayout* form = new QFormLayout(attrGroup);
    form->setSpacing(8);
    
    QLineEdit* phaseEdit = new QLineEdit();
    form->addRow("Phase:", phaseEdit);
    
    QLineEdit* lotEdit = new QLineEdit();
    form->addRow("Lot Number:", lotEdit);
    
    QLineEdit* sequenceEdit = new QLineEdit();
    form->addRow("Erection Sequence:", sequenceEdit);
    
    QComboBox* statusCombo = new QComboBox();
    statusCombo->addItems({"Preliminary", "Approved", "Fabricated", "Erected"});
    form->addRow("Status:", statusCombo);
    
    layout->addWidget(attrGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(m_attributesTab, "Attributes");
}

void PropertiesPanel::createCommonPropertiesGroup()
{
    m_commonGroup = new QGroupBox("Common Properties", m_mainWidget);
    QFormLayout* layout = new QFormLayout(m_commonGroup);
    
    // Object type (read-only)
    m_objectTypeLabel = new QLabel("-");
    m_objectTypeLabel->setStyleSheet("font-weight: bold; color: #0066cc;");
    layout->addRow("Type:", m_objectTypeLabel);
    
    // Object ID (read-only)
    m_objectIDLabel = new QLabel("-");
    layout->addRow("ID:", m_objectIDLabel);
    
    // Name
    m_nameEdit = new QLineEdit();
    connect(m_nameEdit, &QLineEdit::textChanged, this, &PropertiesPanel::onNameChanged);
    layout->addRow("Name:", m_nameEdit);
    
    // Description
    m_descriptionEdit = new QLineEdit();
    connect(m_descriptionEdit, &QLineEdit::textChanged, this, &PropertiesPanel::onDescriptionChanged);
    layout->addRow("Description:", m_descriptionEdit);
    
    // Layer
    m_layerCombo = new QComboBox();
    m_layerCombo->addItems({"Default", "Structure", "Architecture", "Foundation", "MEP"});
    m_layerCombo->setEditable(true);
    connect(m_layerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PropertiesPanel::onLayerChanged);
    layout->addRow("Layer:", m_layerCombo);
    
    // Material
    m_materialEdit = new QLineEdit();
    connect(m_materialEdit, &QLineEdit::textChanged, this, &PropertiesPanel::onMaterialChanged);
    layout->addRow("Material:", m_materialEdit);
    
    // Color
    QHBoxLayout* colorLayout = new QHBoxLayout();
    m_colorButton = new QPushButton("Select Color");
    m_colorButton->setFixedHeight(30);
    connect(m_colorButton, &QPushButton::clicked, this, &PropertiesPanel::onColorButtonClicked);
    colorLayout->addWidget(m_colorButton);
    layout->addRow("Color:", colorLayout);
    
    // Visibility
    m_visibleCheckBox = new QCheckBox("Visible");
    m_visibleCheckBox->setChecked(true);
    connect(m_visibleCheckBox, &QCheckBox::stateChanged, this, &PropertiesPanel::onVisibilityChanged);
    layout->addRow("", m_visibleCheckBox);
    
    // Locked
    m_lockedCheckBox = new QCheckBox("Locked");
    connect(m_lockedCheckBox, &QCheckBox::stateChanged, this, &PropertiesPanel::onLockChanged);
    layout->addRow("", m_lockedCheckBox);
    
    m_mainLayout->addWidget(m_commonGroup);
}

void PropertiesPanel::createGeometryGroup()
{
    m_geometryGroup = new QGroupBox("Geometry", m_mainWidget);
    QFormLayout* layout = new QFormLayout(m_geometryGroup);
    
    m_centerPointLabel = new QLabel("-");
    layout->addRow("Center:", m_centerPointLabel);
    
    m_boundingBoxLabel = new QLabel("-");
    m_boundingBoxLabel->setWordWrap(true);
    layout->addRow("Bounding Box:", m_boundingBoxLabel);
    
    m_mainLayout->addWidget(m_geometryGroup);
}

void PropertiesPanel::createTypeSpecificGroup()
{
    m_typeSpecificGroup = new QGroupBox("Type Specific", m_mainWidget);
    m_typeSpecificLayout = new QVBoxLayout(m_typeSpecificGroup);
    m_mainLayout->addWidget(m_typeSpecificGroup);
}

void PropertiesPanel::createStatisticsGroup()
{
    m_statisticsGroup = new QGroupBox("Statistics", m_mainWidget);
    QFormLayout* layout = new QFormLayout(m_statisticsGroup);
    
    m_volumeLabel = new QLabel("-");
    layout->addRow("Volume:", m_volumeLabel);
    
    m_surfaceAreaLabel = new QLabel("-");
    layout->addRow("Surface Area:", m_surfaceAreaLabel);
    
    m_creationTimeLabel = new QLabel("-");
    layout->addRow("Created:", m_creationTimeLabel);
    
    m_modificationTimeLabel = new QLabel("-");
    layout->addRow("Modified:", m_modificationTimeLabel);
    
    m_mainLayout->addWidget(m_statisticsGroup);
}

void PropertiesPanel::setObject(const Handle(TGraphicObject)& object)
{
    m_currentObject = object;
    
    if (object.IsNull()) {
        clearProperties();
        return;
    }
    
    m_updatingUI = true;
    
    enableEditing(true);
    updateCommonProperties();
    updateGeometryProperties();
    updateStatistics();
    
    // Clear previous type-specific widgets
    QLayoutItem* item;
    while ((item = m_typeSpecificLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    // Add type-specific properties
    Handle(TBeam) beam = Handle(TBeam)::DownCast(object);
    if (!beam.IsNull()) {
        updateBeamProperties();
        m_updatingUI = false;
        return;
    }
    
    Handle(TColumn) column = Handle(TColumn)::DownCast(object);
    if (!column.IsNull()) {
        updateColumnProperties();
        m_updatingUI = false;
        return;
    }
    
    Handle(TSlab) slab = Handle(TSlab)::DownCast(object);
    if (!slab.IsNull()) {
        updateSlabProperties();
        m_updatingUI = false;
        return;
    }
    
    m_updatingUI = false;
}

void PropertiesPanel::updateCommonProperties()
{
    if (m_currentObject.IsNull()) return;
    
    m_objectTypeLabel->setText(m_currentObject->GetTypeName());
    m_objectIDLabel->setText(QString::number(m_currentObject->GetID()));
    m_nameEdit->setText(m_currentObject->GetName());
    m_descriptionEdit->setText(m_currentObject->GetDescription());
    
    QString layer = m_currentObject->GetLayer();
    int layerIndex = m_layerCombo->findText(layer);
    if (layerIndex >= 0) {
        m_layerCombo->setCurrentIndex(layerIndex);
    } else {
        m_layerCombo->setEditText(layer);
    }
    
    m_materialEdit->setText(m_currentObject->GetMaterial());
    m_visibleCheckBox->setChecked(m_currentObject->IsVisible());
    m_lockedCheckBox->setChecked(m_currentObject->IsLocked());
    
    int r, g, b;
    m_currentObject->GetColor(r, g, b);
    m_selectedColor[0] = r;
    m_selectedColor[1] = g;
    m_selectedColor[2] = b;
    
    QString colorStyle = QString("background-color: rgb(%1, %2, %3);").arg(r).arg(g).arg(b);
    m_colorButton->setStyleSheet(colorStyle);
}

void PropertiesPanel::updateGeometryProperties()
{
    if (m_currentObject.IsNull()) return;
    
    gp_Pnt center = m_currentObject->GetCenterPoint();
    m_centerPointLabel->setText(QString("(%1, %2, %3)")
        .arg(center.X(), 0, 'f', 1)
        .arg(center.Y(), 0, 'f', 1)
        .arg(center.Z(), 0, 'f', 1));
    
    double xmin, ymin, zmin, xmax, ymax, zmax;
    m_currentObject->GetBoundingBox(xmin, ymin, zmin, xmax, ymax, zmax);
    m_boundingBoxLabel->setText(QString("X[%1, %2]\nY[%3, %4]\nZ[%5, %6]")
        .arg(xmin, 0, 'f', 1).arg(xmax, 0, 'f', 1)
        .arg(ymin, 0, 'f', 1).arg(ymax, 0, 'f', 1)
        .arg(zmin, 0, 'f', 1).arg(zmax, 0, 'f', 1));
}

void PropertiesPanel::updateBeamProperties()
{
    Handle(TBeam) beam = Handle(TBeam)::DownCast(m_currentObject);
    if (beam.IsNull()) return;
    
    QFormLayout* layout = new QFormLayout();
    
    // Start point
    gp_Pnt start = beam->GetStartPoint();
    m_beamStartLabel = new QLabel(QString("(%1, %2, %3)")
        .arg(start.X(), 0, 'f', 1)
        .arg(start.Y(), 0, 'f', 1)
        .arg(start.Z(), 0, 'f', 1));
    layout->addRow("Start Point:", m_beamStartLabel);
    
    // End point
    gp_Pnt end = beam->GetEndPoint();
    m_beamEndLabel = new QLabel(QString("(%1, %2, %3)")
        .arg(end.X(), 0, 'f', 1)
        .arg(end.Y(), 0, 'f', 1)
        .arg(end.Z(), 0, 'f', 1));
    layout->addRow("End Point:", m_beamEndLabel);
    
    // Length
    m_beamLengthLabel = new QLabel(QString("%1 mm (%2 m)")
        .arg(beam->GetLength(), 0, 'f', 1)
        .arg(beam->GetLength() / 1000.0, 0, 'f', 3));
    layout->addRow("Length:", m_beamLengthLabel);
    
    // Direction
    gp_Vec dir = beam->GetDirection();
    m_beamDirectionLabel = new QLabel(QString("(%1, %2, %3)")
        .arg(dir.X(), 0, 'f', 3)
        .arg(dir.Y(), 0, 'f', 3)
        .arg(dir.Z(), 0, 'f', 3));
    layout->addRow("Direction:", m_beamDirectionLabel);
    
    // Section type
    if (beam->IsProfileSection()) {
        QLabel* sectionLabel = new QLabel(QString("<b>Steel Profile: %1</b>")
            .arg(beam->GetProfileSize()));
        layout->addRow("Section:", sectionLabel);
    } else {
        double width, height;
        beam->GetSectionDimensions(width, height);
        QLabel* sectionLabel = new QLabel(QString("<b>Rectangular: %1 × %2 mm</b>")
            .arg(width, 0, 'f', 0)
            .arg(height, 0, 'f', 0));
        layout->addRow("Section:", sectionLabel);
    }
    
    m_typeSpecificLayout->addLayout(layout);
}

void PropertiesPanel::updateColumnProperties()
{
    Handle(TColumn) column = Handle(TColumn)::DownCast(m_currentObject);
    if (column.IsNull()) return;
    
    QFormLayout* layout = new QFormLayout();
    
    // Base point
    gp_Pnt base = column->GetBasePoint();
    m_columnBaseLabel = new QLabel(QString("(%1, %2, %3)")
        .arg(base.X(), 0, 'f', 1)
        .arg(base.Y(), 0, 'f', 1)
        .arg(base.Z(), 0, 'f', 1));
    layout->addRow("Base Point:", m_columnBaseLabel);
    
    // Dimensions
    double width, depth, height;
    column->GetDimensions(width, depth, height);
    
    m_columnWidthSpin = new QDoubleSpinBox();
    m_columnWidthSpin->setRange(50, 5000);
    m_columnWidthSpin->setValue(width);
    m_columnWidthSpin->setSuffix(" mm");
    connect(m_columnWidthSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PropertiesPanel::onColumnPropertyChanged);
    layout->addRow("Width:", m_columnWidthSpin);
    
    m_columnDepthSpin = new QDoubleSpinBox();
    m_columnDepthSpin->setRange(50, 5000);
    m_columnDepthSpin->setValue(depth);
    m_columnDepthSpin->setSuffix(" mm");
    connect(m_columnDepthSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PropertiesPanel::onColumnPropertyChanged);
    layout->addRow("Depth:", m_columnDepthSpin);
    
    m_columnHeightSpin = new QDoubleSpinBox();
    m_columnHeightSpin->setRange(100, 20000);
    m_columnHeightSpin->setValue(height);
    m_columnHeightSpin->setSuffix(" mm");
    connect(m_columnHeightSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PropertiesPanel::onColumnPropertyChanged);
    layout->addRow("Height:", m_columnHeightSpin);
    
    m_typeSpecificLayout->addLayout(layout);
}

void PropertiesPanel::updateSlabProperties()
{
    Handle(TSlab) slab = Handle(TSlab)::DownCast(m_currentObject);
    if (slab.IsNull()) return;
    
    QFormLayout* layout = new QFormLayout();
    
    // Corners
    gp_Pnt corner1, corner2;
    slab->GetCorners(corner1, corner2);
    
    m_slabCorner1Label = new QLabel(QString("(%1, %2, %3)")
        .arg(corner1.X(), 0, 'f', 1)
        .arg(corner1.Y(), 0, 'f', 1)
        .arg(corner1.Z(), 0, 'f', 1));
    layout->addRow("Corner 1:", m_slabCorner1Label);
    
    m_slabCorner2Label = new QLabel(QString("(%1, %2, %3)")
        .arg(corner2.X(), 0, 'f', 1)
        .arg(corner2.Y(), 0, 'f', 1)
        .arg(corner2.Z(), 0, 'f', 1));
    layout->addRow("Corner 2:", m_slabCorner2Label);
    
    // Thickness
    m_slabThicknessSpin = new QDoubleSpinBox();
    m_slabThicknessSpin->setRange(50, 1000);
    m_slabThicknessSpin->setValue(slab->GetThickness());
    m_slabThicknessSpin->setSuffix(" mm");
    connect(m_slabThicknessSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PropertiesPanel::onSlabPropertyChanged);
    layout->addRow("Thickness:", m_slabThicknessSpin);
    
    // Area
    m_slabAreaLabel = new QLabel(QString("%1 m²")
        .arg(slab->GetArea() / 1e6, 0, 'f', 2));
    layout->addRow("Area:", m_slabAreaLabel);
    
    m_typeSpecificLayout->addLayout(layout);
}

void PropertiesPanel::updateStatistics()
{
    if (m_currentObject.IsNull()) return;
    
    double volume = m_currentObject->GetVolume();
    m_volumeLabel->setText(QString("%1 m³").arg(volume / 1e9, 0, 'f', 6));
    
    double area = m_currentObject->GetSurfaceArea();
    m_surfaceAreaLabel->setText(QString("%1 m²").arg(area / 1e6, 0, 'f', 3));
    
    m_creationTimeLabel->setText(m_currentObject->GetCreationTime().toString("yyyy-MM-dd hh:mm:ss"));
    m_modificationTimeLabel->setText(m_currentObject->GetModificationTime().toString("yyyy-MM-dd hh:mm:ss"));
}

void PropertiesPanel::clearProperties()
{
    m_updatingUI = true;
    
    m_objectTypeLabel->setText("-");
    m_objectIDLabel->setText("-");
    m_nameEdit->clear();
    m_descriptionEdit->clear();
    m_layerCombo->setCurrentIndex(0);
    m_materialEdit->clear();
    m_visibleCheckBox->setChecked(true);
    m_lockedCheckBox->setChecked(false);
    
    m_centerPointLabel->setText("-");
    m_boundingBoxLabel->setText("-");
    
    m_volumeLabel->setText("-");
    m_surfaceAreaLabel->setText("-");
    m_creationTimeLabel->setText("-");
    m_modificationTimeLabel->setText("-");
    
    // Clear type-specific widgets
    QLayoutItem* item;
    while ((item = m_typeSpecificLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    enableEditing(false);
    m_updatingUI = false;
}

void PropertiesPanel::setMultipleSelection(int count)
{
    clearProperties();
    m_objectTypeLabel->setText(QString("Multiple Objects (%1)").arg(count));
    m_objectIDLabel->setText("Multiple");
}

void PropertiesPanel::enableEditing(bool enabled)
{
    m_nameEdit->setEnabled(enabled);
    m_descriptionEdit->setEnabled(enabled);
    m_layerCombo->setEnabled(enabled);
    m_materialEdit->setEnabled(enabled);
    m_colorButton->setEnabled(enabled);
    m_visibleCheckBox->setEnabled(enabled);
    m_lockedCheckBox->setEnabled(enabled);
    m_applyButton->setEnabled(enabled);
}

void PropertiesPanel::onNameChanged()
{
    if (m_updatingUI || m_currentObject.IsNull()) return;
    m_currentObject->SetName(m_nameEdit->text());
    emit nameChanged(m_currentObject->GetID(), m_nameEdit->text());
}

void PropertiesPanel::onDescriptionChanged()
{
    if (m_updatingUI || m_currentObject.IsNull()) return;
    m_currentObject->SetDescription(m_descriptionEdit->text());
}

void PropertiesPanel::onLayerChanged(int index)
{
    if (m_updatingUI || m_currentObject.IsNull()) return;
    QString layer = m_layerCombo->currentText();
    m_currentObject->SetLayer(layer);
    emit layerChanged(m_currentObject->GetID(), layer);
}

void PropertiesPanel::onMaterialChanged()
{
    if (m_updatingUI || m_currentObject.IsNull()) return;
    m_currentObject->SetMaterial(m_materialEdit->text());
}

void PropertiesPanel::onColorButtonClicked()
{
    if (m_currentObject.IsNull()) return;
    
    QColor initialColor(m_selectedColor[0], m_selectedColor[1], m_selectedColor[2]);
    QColor color = QColorDialog::getColor(initialColor, this, "Select Object Color");
    
    if (color.isValid()) {
        m_selectedColor[0] = color.red();
        m_selectedColor[1] = color.green();
        m_selectedColor[2] = color.blue();
        
        QString colorStyle = QString("background-color: rgb(%1, %2, %3);")
            .arg(m_selectedColor[0]).arg(m_selectedColor[1]).arg(m_selectedColor[2]);
        m_colorButton->setStyleSheet(colorStyle);
        
        m_currentObject->SetColor(m_selectedColor[0], m_selectedColor[1], m_selectedColor[2]);
        emit colorChanged(m_currentObject->GetID(), m_selectedColor[0], m_selectedColor[1], m_selectedColor[2]);
    }
}

void PropertiesPanel::onVisibilityChanged(int state)
{
    if (m_updatingUI || m_currentObject.IsNull()) return;
    m_currentObject->SetVisible(state == Qt::Checked);
    emit propertyChanged(m_currentObject->GetID());
}

void PropertiesPanel::onLockChanged(int state)
{
    if (m_updatingUI || m_currentObject.IsNull()) return;
    m_currentObject->SetLocked(state == Qt::Checked);
}

void PropertiesPanel::onApplyProperties()
{
    if (m_currentObject.IsNull()) return;
    emit propertyChanged(m_currentObject->GetID());
    QMessageBox::information(this, "Properties", "Properties applied successfully!");
}

void PropertiesPanel::onBeamPropertyChanged()
{
    // Beam properties are mostly read-only for now
    // Can be extended to allow editing
}

void PropertiesPanel::onColumnPropertyChanged()
{
    if (m_updatingUI) return;
    
    Handle(TColumn) column = Handle(TColumn)::DownCast(m_currentObject);
    if (column.IsNull()) return;
    
    column->SetDimensions(
        m_columnWidthSpin->value(),
        m_columnDepthSpin->value(),
        m_columnHeightSpin->value()
    );
    
    updateStatistics();
    emit propertyChanged(m_currentObject->GetID());
}

void PropertiesPanel::onSlabPropertyChanged()
{
    if (m_updatingUI) return;
    
    Handle(TSlab) slab = Handle(TSlab)::DownCast(m_currentObject);
    if (slab.IsNull()) return;
    
    slab->SetThickness(m_slabThicknessSpin->value());
    
    updateStatistics();
    emit propertyChanged(m_currentObject->GetID());
}

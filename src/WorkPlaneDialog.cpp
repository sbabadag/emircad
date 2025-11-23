#include "WorkPlaneDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>

WorkPlaneDialog::WorkPlaneDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Work Plane Settings");
    setupUI();
    applyStyles();
    resize(400, 250);
}

void WorkPlaneDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Plane type selection
    QGroupBox* typeGroup = new QGroupBox("Plane Type");
    QFormLayout* typeLayout = new QFormLayout();
    
    m_planeTypeCombo = new QComboBox();
    m_planeTypeCombo->addItem("XY Plane (Ground)", WorkPlane::XY);
    m_planeTypeCombo->addItem("XZ Plane (Front Elevation)", WorkPlane::XZ);
    m_planeTypeCombo->addItem("YZ Plane (Side Elevation)", WorkPlane::YZ);
    typeLayout->addRow("Type:", m_planeTypeCombo);
    
    typeGroup->setLayout(typeLayout);
    mainLayout->addWidget(typeGroup);
    
    // Offset settings
    QGroupBox* offsetGroup = new QGroupBox("Plane Position");
    QFormLayout* offsetLayout = new QFormLayout();
    
    m_offsetSpin = new QDoubleSpinBox();
    m_offsetSpin->setRange(-100000.0, 100000.0);
    m_offsetSpin->setValue(0.0);
    m_offsetSpin->setSuffix(" mm");
    m_offsetSpin->setDecimals(1);
    m_offsetSpin->setSingleStep(100.0);
    offsetLayout->addRow("Offset:", m_offsetSpin);
    
    offsetGroup->setLayout(offsetLayout);
    mainLayout->addWidget(offsetGroup);
    
    // Visibility
    QGroupBox* displayGroup = new QGroupBox("Display");
    QVBoxLayout* displayLayout = new QVBoxLayout();
    
    m_visibleCheck = new QCheckBox("Show work plane grid");
    m_visibleCheck->setChecked(true);
    displayLayout->addWidget(m_visibleCheck);
    
    displayGroup->setLayout(displayLayout);
    mainLayout->addWidget(displayGroup);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    QPushButton* okButton = new QPushButton("OK");
    QPushButton* cancelButton = new QPushButton("Cancel");
    
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
}

void WorkPlaneDialog::applyStyles()
{
    setStyleSheet(R"(
        QDialog {
            background-color: #2b2b2b;
            color: #ffffff;
        }
        QGroupBox {
            border: 1px solid #555555;
            border-radius: 4px;
            margin-top: 8px;
            padding-top: 8px;
            font-weight: bold;
            color: #ffffff;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }
        QLabel {
            color: #cccccc;
        }
        QComboBox, QDoubleSpinBox {
            background-color: #3c3c3c;
            border: 1px solid #555555;
            border-radius: 3px;
            padding: 5px;
            color: #ffffff;
            min-height: 25px;
        }
        QComboBox::drop-down {
            border: none;
            width: 20px;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid #ffffff;
            margin-right: 5px;
        }
        QComboBox:hover, QDoubleSpinBox:hover {
            border: 1px solid #0d6efd;
        }
        QComboBox QAbstractItemView {
            background-color: #3c3c3c;
            border: 1px solid #555555;
            selection-background-color: #0d6efd;
            color: #ffffff;
        }
        QCheckBox {
            color: #cccccc;
            spacing: 8px;
        }
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border: 1px solid #555555;
            border-radius: 3px;
            background-color: #3c3c3c;
        }
        QCheckBox::indicator:checked {
            background-color: #0d6efd;
            border-color: #0d6efd;
        }
        QCheckBox::indicator:checked:after {
            content: '✓';
            color: white;
        }
        QPushButton {
            background-color: #0d6efd;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 8px 20px;
            font-weight: bold;
            min-width: 80px;
        }
        QPushButton:hover {
            background-color: #0b5ed7;
        }
        QPushButton:pressed {
            background-color: #0a58ca;
        }
    )");
}

WorkPlane::PlaneType WorkPlaneDialog::getSelectedPlaneType() const
{
    return static_cast<WorkPlane::PlaneType>(m_planeTypeCombo->currentData().toInt());
}

double WorkPlaneDialog::getOffset() const
{
    return m_offsetSpin->value();
}

bool WorkPlaneDialog::isVisible() const
{
    return m_visibleCheck->isChecked();
}

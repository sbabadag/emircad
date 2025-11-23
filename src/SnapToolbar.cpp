#include "SnapToolbar.h"
#include <QFrame>

SnapToolbar::SnapToolbar(QWidget *parent)
    : QWidget(parent)
    , m_allEnabled(true)
{
    setupUI();
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, false);
    setWindowOpacity(0.95);
}

SnapToolbar::~SnapToolbar()
{
}

void SnapToolbar::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(3);
    
    // Title bar with toggle all button
    QHBoxLayout* titleLayout = new QHBoxLayout();
    QLabel* titleLabel = new QLabel("Object Snap", this);
    titleLabel->setStyleSheet("font-weight: bold; color: white;");
    
    m_toggleAllBtn = new QPushButton("All On", this);
    m_toggleAllBtn->setMaximumWidth(60);
    m_toggleAllBtn->setStyleSheet("QPushButton { background-color: #3498db; color: white; border: none; padding: 3px; }");
    connect(m_toggleAllBtn, &QPushButton::clicked, this, &SnapToolbar::onToggleAllClicked);
    
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(m_toggleAllBtn);
    mainLayout->addLayout(titleLayout);
    
    // Add separator
    QFrame* line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setStyleSheet("background-color: #555;");
    mainLayout->addWidget(line);
    
    // Snap type checkboxes
    m_endpointCheck = new QCheckBox("Endpoint", this);
    m_endpointCheck->setChecked(true);
    m_endpointCheck->setStyleSheet("QCheckBox { color: #2ecc71; } QCheckBox::indicator { width: 16px; height: 16px; }");
    connect(m_endpointCheck, &QCheckBox::stateChanged, this, &SnapToolbar::onSnapCheckChanged);
    mainLayout->addWidget(m_endpointCheck);
    
    m_midpointCheck = new QCheckBox("Midpoint", this);
    m_midpointCheck->setChecked(true);
    m_midpointCheck->setStyleSheet("QCheckBox { color: #3498db; } QCheckBox::indicator { width: 16px; height: 16px; }");
    connect(m_midpointCheck, &QCheckBox::stateChanged, this, &SnapToolbar::onSnapCheckChanged);
    mainLayout->addWidget(m_midpointCheck);
    
    m_centerCheck = new QCheckBox("Center", this);
    m_centerCheck->setChecked(true);
    m_centerCheck->setStyleSheet("QCheckBox { color: #e74c3c; } QCheckBox::indicator { width: 16px; height: 16px; }");
    connect(m_centerCheck, &QCheckBox::stateChanged, this, &SnapToolbar::onSnapCheckChanged);
    mainLayout->addWidget(m_centerCheck);
    
    m_vertexCheck = new QCheckBox("Vertex", this);
    m_vertexCheck->setChecked(true);
    m_vertexCheck->setStyleSheet("QCheckBox { color: #f1c40f; } QCheckBox::indicator { width: 16px; height: 16px; }");
    connect(m_vertexCheck, &QCheckBox::stateChanged, this, &SnapToolbar::onSnapCheckChanged);
    mainLayout->addWidget(m_vertexCheck);
    
    m_nearestCheck = new QCheckBox("Nearest", this);
    m_nearestCheck->setChecked(false);
    m_nearestCheck->setStyleSheet("QCheckBox { color: white; } QCheckBox::indicator { width: 16px; height: 16px; }");
    connect(m_nearestCheck, &QCheckBox::stateChanged, this, &SnapToolbar::onSnapCheckChanged);
    mainLayout->addWidget(m_nearestCheck);
    
    // Set overall widget style
    setStyleSheet(
        "QWidget { background-color: #2c3e50; color: white; border: 2px solid #34495e; border-radius: 5px; }"
        "QCheckBox { padding: 5px; }"
        "QCheckBox::indicator:checked { background-color: white; border: 1px solid #555; }"
        "QCheckBox::indicator:unchecked { background-color: #444; border: 1px solid #555; }"
    );
    
    setMinimumWidth(180);
    adjustSize();
}

void SnapToolbar::onSnapCheckChanged()
{
    updateSnapTypes();
}

void SnapToolbar::onToggleAllClicked()
{
    m_allEnabled = !m_allEnabled;
    
    m_endpointCheck->setChecked(m_allEnabled);
    m_midpointCheck->setChecked(m_allEnabled);
    m_centerCheck->setChecked(m_allEnabled);
    m_vertexCheck->setChecked(m_allEnabled);
    m_nearestCheck->setChecked(false); // Nearest is off by default
    
    m_toggleAllBtn->setText(m_allEnabled ? "All On" : "All Off");
    
    updateSnapTypes();
}

void SnapToolbar::updateSnapTypes()
{
    int types = 0;
    
    if (m_endpointCheck->isChecked())
        types |= SnapManager::Endpoint;
    if (m_midpointCheck->isChecked())
        types |= SnapManager::Midpoint;
    if (m_centerCheck->isChecked())
        types |= SnapManager::Center;
    if (m_vertexCheck->isChecked())
        types |= SnapManager::Vertex;
    if (m_nearestCheck->isChecked())
        types |= SnapManager::Nearest;
    
    emit snapTypesChanged(types);
}

int SnapToolbar::getEnabledSnapTypes() const
{
    int types = 0;
    
    if (m_endpointCheck->isChecked())
        types |= SnapManager::Endpoint;
    if (m_midpointCheck->isChecked())
        types |= SnapManager::Midpoint;
    if (m_centerCheck->isChecked())
        types |= SnapManager::Center;
    if (m_vertexCheck->isChecked())
        types |= SnapManager::Vertex;
    if (m_nearestCheck->isChecked())
        types |= SnapManager::Nearest;
    
    return types;
}

void SnapToolbar::setEnabledSnapTypes(int types)
{
    m_endpointCheck->setChecked(types & SnapManager::Endpoint);
    m_midpointCheck->setChecked(types & SnapManager::Midpoint);
    m_centerCheck->setChecked(types & SnapManager::Center);
    m_vertexCheck->setChecked(types & SnapManager::Vertex);
    m_nearestCheck->setChecked(types & SnapManager::Nearest);
}

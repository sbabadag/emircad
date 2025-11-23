#include "AssemblyCommand.h"
#include "CADController.h"
#include "TObjectCollection.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

// AssemblyCommand Implementation

AssemblyCommand::AssemblyCommand(CADController* controller, AssemblyMode mode)
    : CADCommand(controller)
    , m_mode(mode)
    , m_complete(false)
{
}

AssemblyCommand::~AssemblyCommand() {
}

void AssemblyCommand::execute(const gp_Pnt& point) {
    Q_UNUSED(point);
    
    switch (m_mode) {
        case CREATE_FROM_SELECTION:
            createAssemblyFromSelection();
            break;
        case ADD_TO_EXISTING:
            addToExistingAssembly();
            break;
        case REMOVE_FROM_ASSEMBLY:
            removeFromAssembly();
            break;
        case EXPLODE_ASSEMBLY:
            explodeAssembly();
            break;
    }
    
    m_complete = true;
}

void AssemblyCommand::preview(const gp_Pnt& point) {
    Q_UNUSED(point);
    // No preview needed for assembly operations
}

void AssemblyCommand::cancel() {
    m_complete = true;
    m_currentAssembly.reset();
    m_selectedObjects.clear();
}

bool AssemblyCommand::isComplete() const {
    return m_complete;
}

QString AssemblyCommand::getCommandName() const {
    switch (m_mode) {
        case CREATE_FROM_SELECTION:
            return "Create Assembly";
        case ADD_TO_EXISTING:
            return "Add to Assembly";
        case REMOVE_FROM_ASSEMBLY:
            return "Remove from Assembly";
        case EXPLODE_ASSEMBLY:
            return "Explode Assembly";
        default:
            return "Assembly Command";
    }
}

void AssemblyCommand::setAssemblyMode(AssemblyMode mode) {
    m_mode = mode;
}

AssemblyCommand::AssemblyMode AssemblyCommand::getAssemblyMode() const {
    return m_mode;
}

void AssemblyCommand::setTargetAssembly(std::shared_ptr<TAssembly> assembly) {
    m_targetAssembly = assembly;
}

void AssemblyCommand::createAssemblyFromSelection() {
    // Get selected objects from controller
    auto collection = m_controller->getObjectCollection();
    if (!collection) return;
    
    // For now, we'll need to implement selection mechanism in controller
    // This is a placeholder showing the concept
    
    if (m_selectedObjects.empty()) {
        // Show dialog to select objects
        // For demonstration, let's assume we have some objects
        return;
    }
    
    // Show assembly configuration dialog
    if (!showAssemblyDialog()) {
        return;
    }
    
    // Create the assembly
    if (m_currentAssembly) {
        // Add to collection
        collection->addObject(m_currentAssembly);
        
        // Optionally remove individual parts from main display
        // (keep them only in the assembly)
        // for (auto& part : m_selectedObjects) {
        //     collection->removeObject(part);
        // }
    }
}

void AssemblyCommand::addToExistingAssembly() {
    if (!m_targetAssembly) return;
    
    for (auto& obj : m_selectedObjects) {
        m_targetAssembly->addPart(obj);
    }
    
    m_targetAssembly->updateCompound();
}

void AssemblyCommand::removeFromAssembly() {
    if (!m_targetAssembly) return;
    
    for (auto& obj : m_selectedObjects) {
        m_targetAssembly->removePart(obj);
    }
    
    m_targetAssembly->updateCompound();
}

void AssemblyCommand::explodeAssembly() {
    if (!m_targetAssembly) return;
    
    auto collection = m_controller->getObjectCollection();
    if (!collection) return;
    
    // Get all parts from assembly
    const auto& parts = m_targetAssembly->getParts();
    
    // Add each part back to the main collection
    for (const auto& part : parts) {
        if (part) {
            collection->addObject(part);
        }
    }
    
    // Remove the assembly
    collection->removeObject(m_targetAssembly);
    m_targetAssembly.reset();
}

bool AssemblyCommand::showAssemblyDialog() {
    AssemblyDialog dialog(m_selectedObjects);
    
    if (dialog.exec() == QDialog::Accepted) {
        // Create new assembly
        m_currentAssembly = std::make_shared<TAssembly>();
        m_currentAssembly->setAssemblyName(dialog.getAssemblyName());
        m_currentAssembly->setAssemblyType(dialog.getAssemblyType());
        
        // Add selected parts
        auto selectedParts = dialog.getSelectedParts();
        for (auto& part : selectedParts) {
            m_currentAssembly->addPart(part);
        }
        
        return true;
    }
    
    return false;
}

// AssemblyDialog Implementation

AssemblyDialog::AssemblyDialog(const std::vector<std::shared_ptr<TGraphicObject>>& selectedObjects,
                               QWidget* parent)
    : QDialog(parent)
    , m_availableParts(selectedObjects)
{
    setWindowTitle("Create Assembly");
    setupUI();
    populatePartsList();
}

void AssemblyDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Assembly name
    QHBoxLayout* nameLayout = new QHBoxLayout();
    nameLayout->addWidget(new QLabel("Assembly Name:"));
    m_nameEdit = new QLineEdit("New Assembly");
    nameLayout->addWidget(m_nameEdit);
    mainLayout->addLayout(nameLayout);
    
    // Assembly type
    QHBoxLayout* typeLayout = new QHBoxLayout();
    typeLayout->addWidget(new QLabel("Assembly Type:"));
    m_typeCombo = new QComboBox();
    m_typeCombo->addItems({
        "Generic",
        "Floor",
        "Frame",
        "Truss",
        "Column Grid",
        "Beam Grid",
        "Wall Panel",
        "Roof Structure",
        "Foundation",
        "Module"
    });
    typeLayout->addWidget(m_typeCombo);
    mainLayout->addLayout(typeLayout);
    
    // Parts list
    mainLayout->addWidget(new QLabel("Parts in Assembly:"));
    m_partsList = new QListWidget();
    m_partsList->setSelectionMode(QAbstractItemView::MultiSelection);
    mainLayout->addWidget(m_partsList);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_okButton = new QPushButton("Create");
    m_cancelButton = new QPushButton("Cancel");
    
    connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    mainLayout->addLayout(buttonLayout);
    
    setMinimumWidth(400);
    setMinimumHeight(300);
}

void AssemblyDialog::populatePartsList() {
    m_partsList->clear();
    
    for (size_t i = 0; i < m_availableParts.size(); ++i) {
        auto part = m_availableParts[i];
        if (part) {
            QString itemText = QString("%1 - %2")
                .arg(i + 1)
                .arg(part->getTypeName());
            
            QListWidgetItem* item = new QListWidgetItem(itemText);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Checked);  // All selected by default
            m_partsList->addItem(item);
        }
    }
}

QString AssemblyDialog::getAssemblyName() const {
    return m_nameEdit->text();
}

QString AssemblyDialog::getAssemblyType() const {
    return m_typeCombo->currentText();
}

std::vector<std::shared_ptr<TGraphicObject>> AssemblyDialog::getSelectedParts() const {
    std::vector<std::shared_ptr<TGraphicObject>> selected;
    
    for (int i = 0; i < m_partsList->count(); ++i) {
        QListWidgetItem* item = m_partsList->item(i);
        if (item && item->checkState() == Qt::Checked) {
            if (i < static_cast<int>(m_availableParts.size())) {
                selected.push_back(m_availableParts[i]);
            }
        }
    }
    
    return selected;
}

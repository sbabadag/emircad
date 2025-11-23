#include "ProfileSelectionDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QDialogButtonBox>

ProfileSelectionDialog::ProfileSelectionDialog(QWidget *parent)
    : QDialog(parent), m_useProfile(true)
{
    setWindowTitle("Select Beam Profile");
    setModal(true);
    resize(500, 600);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Profile type selection
    QGroupBox *typeGroup = new QGroupBox("Profile Type", this);
    QVBoxLayout *typeLayout = new QVBoxLayout(typeGroup);
    
    m_profileTypeCombo = new QComboBox(this);
    m_profileTypeCombo->addItem("IPE - European I-Beam", (int)SteelProfile::IPE);
    m_profileTypeCombo->addItem("HEA - Wide Flange (Light)", (int)SteelProfile::HEA);
    m_profileTypeCombo->addItem("HEB - Wide Flange (Medium)", (int)SteelProfile::HEB);
    m_profileTypeCombo->addItem("HEM - Wide Flange (Heavy)", (int)SteelProfile::HEM);
    m_profileTypeCombo->addItem("RHS - Rectangular Hollow Section", (int)SteelProfile::RHS);
    
    typeLayout->addWidget(m_profileTypeCombo);
    mainLayout->addWidget(typeGroup);
    
    // Size selection
    QGroupBox *sizeGroup = new QGroupBox("Available Sizes", this);
    QVBoxLayout *sizeLayout = new QVBoxLayout(sizeGroup);
    
    m_sizeList = new QListWidget(this);
    m_sizeList->setSelectionMode(QAbstractItemView::SingleSelection);
    sizeLayout->addWidget(m_sizeList);
    
    mainLayout->addWidget(sizeGroup);
    
    // Dimensions display
    QGroupBox *dimGroup = new QGroupBox("Dimensions", this);
    QVBoxLayout *dimLayout = new QVBoxLayout(dimGroup);
    
    m_dimensionsLabel = new QLabel("Select a profile to see dimensions", this);
    m_dimensionsLabel->setWordWrap(true);
    dimLayout->addWidget(m_dimensionsLabel);
    
    mainLayout->addWidget(dimGroup);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    QPushButton *useRectangularBtn = new QPushButton("Use Rectangular Section", this);
    buttonLayout->addWidget(useRectangularBtn);
    
    buttonLayout->addStretch();
    
    QDialogButtonBox *dialogButtons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonLayout->addWidget(dialogButtons);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(m_profileTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ProfileSelectionDialog::onProfileTypeChanged);
    connect(m_sizeList, &QListWidget::itemSelectionChanged,
            this, &ProfileSelectionDialog::updateDimensions);
    connect(m_sizeList, &QListWidget::itemDoubleClicked,
            this, &ProfileSelectionDialog::accept);
    connect(useRectangularBtn, &QPushButton::clicked,
            this, &ProfileSelectionDialog::onUseRectangular);
    connect(dialogButtons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(dialogButtons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    // Initialize with IPE profiles
    updateSizeList();
    m_sizeList->setCurrentRow(6); // Default to IPE 200
}

SteelProfile::ProfileType ProfileSelectionDialog::getSelectedProfileType() const
{
    int index = m_profileTypeCombo->currentIndex();
    return (SteelProfile::ProfileType)m_profileTypeCombo->itemData(index).toInt();
}

QString ProfileSelectionDialog::getSelectedSize() const
{
    QListWidgetItem *item = m_sizeList->currentItem();
    if (item) {
        return item->text();
    }
    return "IPE 200";
}

void ProfileSelectionDialog::onProfileTypeChanged(int index)
{
    Q_UNUSED(index);
    updateSizeList();
}

void ProfileSelectionDialog::onSizeSelected()
{
    m_useProfile = true;
    accept();
}

void ProfileSelectionDialog::onUseRectangular()
{
    m_useProfile = false;
    accept();
}

void ProfileSelectionDialog::updateSizeList()
{
    m_sizeList->clear();
    
    SteelProfile::ProfileType type = getSelectedProfileType();
    QStringList sizes = SteelProfile::getAvailableSizes(type);
    
    for (const QString& size : sizes) {
        m_sizeList->addItem(size);
    }
    
    if (m_sizeList->count() > 0) {
        m_sizeList->setCurrentRow(0);
        updateDimensions();
    }
}

void ProfileSelectionDialog::updateDimensions()
{
    QListWidgetItem *item = m_sizeList->currentItem();
    if (!item) {
        m_dimensionsLabel->setText("Select a profile to see dimensions");
        return;
    }
    
    QString size = item->text();
    SteelProfile::ProfileType type = getSelectedProfileType();
    SteelProfile::Dimensions dim = SteelProfile::getDimensions(type, size);
    
    QString dimText;
    if (type == SteelProfile::RHS) {
        dimText = QString("<b>%1</b><br>"
                         "Height: %2 mm<br>"
                         "Width: %3 mm<br>"
                         "Wall Thickness: %4 mm")
                         .arg(size)
                         .arg(dim.height, 0, 'f', 1)
                         .arg(dim.width, 0, 'f', 1)
                         .arg(dim.thickness, 0, 'f', 1);
    } else {
        dimText = QString("<b>%1</b><br>"
                         "Height (h): %2 mm<br>"
                         "Width (b): %3 mm<br>"
                         "Web Thickness (tw): %4 mm<br>"
                         "Flange Thickness (tf): %5 mm<br>"
                         "Root Radius (r): %6 mm")
                         .arg(size)
                         .arg(dim.height, 0, 'f', 1)
                         .arg(dim.width, 0, 'f', 1)
                         .arg(dim.webThickness, 0, 'f', 1)
                         .arg(dim.flangeThickness, 0, 'f', 1)
                         .arg(dim.radius, 0, 'f', 1);
    }
    
    m_dimensionsLabel->setText(dimText);
}

#ifndef PROFILESELECTIONDIALOG_H
#define PROFILESELECTIONDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QListWidget>
#include <QLabel>
#include "SteelProfile.h"

class ProfileSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProfileSelectionDialog(QWidget *parent = nullptr);
    
    SteelProfile::ProfileType getSelectedProfileType() const;
    QString getSelectedSize() const;
    bool useProfile() const { return m_useProfile; }

private slots:
    void onProfileTypeChanged(int index);
    void onSizeSelected();
    void onUseRectangular();

private:
    QComboBox *m_profileTypeCombo;
    QListWidget *m_sizeList;
    QLabel *m_dimensionsLabel;
    bool m_useProfile;
    
    void updateSizeList();
    void updateDimensions();
};

#endif // PROFILESELECTIONDIALOG_H

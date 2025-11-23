#ifndef ASSEMBLYCOMMAND_H
#define ASSEMBLYCOMMAND_H

#include "CADCommand.h"
#include "TAssembly.h"
#include <memory>
#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>

/**
 * @brief Command for creating assemblies from existing objects
 * 
 * Workflow:
 * 1. User selects multiple objects (beams, columns, slabs)
 * 2. User creates assembly from selection
 * 3. Assembly can be manipulated as a single unit
 */
class AssemblyCommand : public CADCommand {
public:
    enum AssemblyMode {
        CREATE_FROM_SELECTION,
        ADD_TO_EXISTING,
        REMOVE_FROM_ASSEMBLY,
        EXPLODE_ASSEMBLY
    };

    explicit AssemblyCommand(CADController* controller, AssemblyMode mode = CREATE_FROM_SELECTION);
    virtual ~AssemblyCommand();

    virtual void execute(const gp_Pnt& point) override;
    virtual void preview(const gp_Pnt& point) override;
    virtual void cancel() override;
    virtual bool isComplete() const override;
    virtual QString getCommandName() const override;

    void setAssemblyMode(AssemblyMode mode);
    AssemblyMode getAssemblyMode() const;

    void setTargetAssembly(std::shared_ptr<TAssembly> assembly);

private:
    AssemblyMode m_mode;
    std::shared_ptr<TAssembly> m_currentAssembly;
    std::shared_ptr<TAssembly> m_targetAssembly;
    std::vector<std::shared_ptr<TGraphicObject>> m_selectedObjects;
    bool m_complete;
    
    void createAssemblyFromSelection();
    void addToExistingAssembly();
    void removeFromAssembly();
    void explodeAssembly();
    
    bool showAssemblyDialog();
};

/**
 * @brief Dialog for assembly creation and configuration
 */
class AssemblyDialog : public QDialog {
    Q_OBJECT

public:
    explicit AssemblyDialog(const std::vector<std::shared_ptr<TGraphicObject>>& selectedObjects,
                           QWidget* parent = nullptr);
    
    QString getAssemblyName() const;
    QString getAssemblyType() const;
    std::vector<std::shared_ptr<TGraphicObject>> getSelectedParts() const;

private:
    QLineEdit* m_nameEdit;
    QComboBox* m_typeCombo;
    QListWidget* m_partsList;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    
    std::vector<std::shared_ptr<TGraphicObject>> m_availableParts;
    
    void setupUI();
    void populatePartsList();
};

#endif // ASSEMBLYCOMMAND_H

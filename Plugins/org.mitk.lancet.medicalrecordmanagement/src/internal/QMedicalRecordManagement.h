/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QMedicalRecordManagement_h
#define QMedicalRecordManagement_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_QMedicalRecordManagementControls.h"

class QMedicalRecordInfoDialog;
namespace lancet 
{
  class IMedicalRecordsProperty;
  class IMedicalRecordsService;
}
/**
  \brief QMedicalRecordManagement

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class QMedicalRecordManagement : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  static const std::string VIEW_ID;  
  ~QMedicalRecordManagement() override;

  static lancet::IMedicalRecordsService* GetService();

private slots:
	void NewMedicalRecordInfoDialog();
	void ModifMedicalRecordInfoDialog();
	void OpenMedicalRecord();
	void CloseMedicalRecord();

protected:
  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void SetFocus() override;

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
                                  const QList<mitk::DataNode::Pointer> &nodes) override;

  /// \brief Called when the user clicks the GUI button
  void DoImageProcessing();

	void ConnectToService();
	void DisConnectToService();

	void setCaseOpened(bool b);
private:
	bool isClose;
protected Q_SLOTS:

  void Slot_MedicalRecordsPropertySelect(lancet::IMedicalRecordsProperty*);

private:

  QMedicalRecordInfoDialog* m_MedicalRecordInfoDialog;
  Ui::QMedicalRecordManagementControls m_Controls;
};

#endif // QMedicalRecordManagement_h

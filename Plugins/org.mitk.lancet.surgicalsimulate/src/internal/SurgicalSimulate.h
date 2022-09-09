/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef SurgicalSimulate_h
#define SurgicalSimulate_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>


#include "lancetNavigationObjectVisualizationFilter.h"
#include "mitkTrackingDeviceSource.h"
#include "ui_SurgicalSimulateControls.h"

/**
  \brief SurgicalSimulate

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class SurgicalSimulate : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  static const std::string VIEW_ID;

public slots:
  // \brief Step1:Use a NDI Vega Tracking Device
  void UseKuka();

  void OnKukaVisualizeTimer();

protected:
  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void SetFocus() override;

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
                                  const QList<mitk::DataNode::Pointer> &nodes) override;

  /// \brief Step1:Use a NDI Vega Tracking Device
  void UseVega();

  

  Ui::SurgicalSimulateControls m_Controls;

  //vega trackingDeviceSource
  mitk::TrackingDeviceSource::Pointer m_VegaSource;
  //kuka trackingDeviceSource
  mitk::TrackingDeviceSource::Pointer m_KukaSource;

  lancet::NavigationObjectVisualizationFilter::Pointer m_KukaVisualizer;
  QTimer* m_KukaVisualizeTimer{ nullptr };
  mitk::NavigationToolStorage::Pointer  m_KukaToolStorage;
};

#endif // SurgicalSimulate_h

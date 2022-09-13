/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "SurgicalSimulate.h"

// Qt
#include <QMessageBox>

// mitk image
#include <mitkImage.h>
#include <mitkAffineTransform3D.h>
#include <mitkMatrixConvert.h>

//igt
#include <lancetVegaTrackingDevice.h>
#include <kukaRobotDevice.h>

#include "lancetTrackingDeviceSourceConfigurator.h"
#include "mitkNavigationToolStorageDeserializer.h"
#include <QtWidgets\qfiledialog.h>

const std::string SurgicalSimulate::VIEW_ID = "org.mitk.views.surgicalsimulate";

void SurgicalSimulate::SetFocus()
{
  m_Controls.pushButton_connectKuka->setFocus();
}

void SurgicalSimulate::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  connect(m_Controls.pushButton_connectKuka, &QPushButton::clicked, this, &SurgicalSimulate::UseKuka);
  connect(m_Controls.pushButton_connectVega, &QPushButton::clicked, this, &SurgicalSimulate::UseVega);
  connect(m_Controls.pushButton_captureRobot, &QPushButton::clicked, this, &SurgicalSimulate::OnRobotCapture);
}

void SurgicalSimulate::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
                                                const QList<mitk::DataNode::Pointer> &nodes)
{
  // iterate all selected objects, adjust warning visibility
  // foreach (mitk::DataNode::Pointer node, nodes)
  // {
  //   if (node.IsNotNull() && dynamic_cast<mitk::Image *>(node->GetData()))
  //   {
  //     m_Controls.labelWarning->setVisible(false);
  //     m_Controls.buttonPerformImageProcessing->setEnabled(true);
  //     return;
  //   }
  // }
  //
  // m_Controls.labelWarning->setVisible(true);
  // m_Controls.buttonPerformImageProcessing->setEnabled(false);
}

void SurgicalSimulate::UseVega()
{
  //read in filename
  QString filename = QFileDialog::getOpenFileName(nullptr, tr("Open Tool Storage"), "/", tr("Tool Storage Files (*.IGTToolStorage)"));
  if (filename.isNull()) return;

  //read tool storage from disk
  std::string errorMessage = "";
  mitk::NavigationToolStorageDeserializer::Pointer myDeserializer = mitk::NavigationToolStorageDeserializer::New(GetDataStorage());
  m_VegaToolStorage = myDeserializer->Deserialize(filename.toStdString());
  m_VegaToolStorage->SetName(filename.toStdString());

  //! [UseKuka 1]
  //Here we want to use the Kuka robot as a tracking device. Therefore we instantiate a object of the class
  //KukaRobotDevice and make some settings which are necessary for a proper connection to the device.
  MITK_INFO << "Vega tracking";
  //QMessageBox::warning(nullptr, "Warning", "You have to set the parameters for the NDITracking device inside the code (QmitkIGTTutorialView::OnStartIGT()) before you can use it.");
  lancet::NDIVegaTrackingDevice::Pointer vegaTrackingDevice = lancet::NDIVegaTrackingDevice::New();  //instantiate

  //Create Navigation Data Source with the factory class, and the visualize filter.
  lancet::TrackingDeviceSourceConfiguratorLancet::Pointer kukaSourceFactory =
    lancet::TrackingDeviceSourceConfiguratorLancet::New(m_VegaToolStorage, vegaTrackingDevice);

  m_VegaSource = kukaSourceFactory->CreateTrackingDeviceSource(m_VegaVisualizer);

  m_VegaSource->Connect();
  m_VegaSource->StartTracking();

  //update visualize filter by timer
  if (m_VegaVisualizeTimer == nullptr)
  {
    m_VegaVisualizeTimer = new QTimer(this);  //create a new timer
  }
  connect(m_VegaVisualizeTimer, SIGNAL(timeout()), this, SLOT(OnVegaVisualizeTimer())); //connect the timer to the method OnTimer()

  m_VegaVisualizeTimer->start(100);  //Every 100ms the method OnTimer() is called. -> 10fps
}

void SurgicalSimulate::GeneratePoses()
{
}

void SurgicalSimulate::CapturePose(bool translationOnly)
{
  //Output sequence is the same as AddTool sequence
  //get navigation data of flange in robot coords,
  mitk::NavigationData::Pointer nd_robot2flange = m_KukaSource->GetOutput(0);
  
  //get navigation data of RobotEndRF in ndi coords,
  mitk::NavigationData::Pointer nd_Ndi2RobotEndRF = m_VegaSource->GetOutput(m_VegaToolStorage->GetToolIndexByName("RobotEndRF"));
  //get navigation data of RobotBaseRF in ndi coords,
  mitk::NavigationData::Pointer nd_Ndi2RobotBaseRF = m_VegaSource->GetOutput(m_VegaToolStorage->GetToolIndexByName("RobotBaseRF"));
  //get navigation data RobotEndRF in reference frame RobotBaseRF
  mitk::NavigationData::Pointer nd_RobotBaseRF2RobotEndRF = GetNavigationDataInRef(nd_Ndi2RobotEndRF, nd_Ndi2RobotBaseRF);
  
  //add nd to registration module
  m_RobotRegistration->AddPose(nd_robot2flange, nd_RobotBaseRF2RobotEndRF, translationOnly);
}

mitk::NavigationData::Pointer SurgicalSimulate::GetNavigationDataInRef(mitk::NavigationData::Pointer nd,
  mitk::NavigationData::Pointer nd_ref)
{
  mitk::NavigationData::Pointer res;
  res->Graft(nd);
  res->Compose(nd_ref->GetInverse());
  return res;
}

void SurgicalSimulate::UseKuka()
{
  //read in filename
  QString filename = QFileDialog::getOpenFileName(nullptr, tr("Open Tool Storage"), "/", tr("Tool Storage Files (*.IGTToolStorage)"));
  if (filename.isNull()) return;

  //read tool storage from disk
  std::string errorMessage = "";
  mitk::NavigationToolStorageDeserializer::Pointer myDeserializer = mitk::NavigationToolStorageDeserializer::New(GetDataStorage());
  m_KukaToolStorage = myDeserializer->Deserialize(filename.toStdString());
  m_KukaToolStorage->SetName(filename.toStdString());

  //! [UseKuka 1]
  //Here we want to use the Kuka robot as a tracking device. Therefore we instantiate a object of the class
  //KukaRobotDevice and make some settings which are necessary for a proper connection to the device.
  MITK_INFO << "Kuka tracking";
  //QMessageBox::warning(nullptr, "Warning", "You have to set the parameters for the NDITracking device inside the code (QmitkIGTTutorialView::OnStartIGT()) before you can use it.");
  lancet::KukaRobotDevice::Pointer kukaTrackingDevice = lancet::KukaRobotDevice::New();  //instantiate

  //Create Navigation Data Source with the factory class, and the visualize filter.
  lancet::TrackingDeviceSourceConfiguratorLancet::Pointer kukaSourceFactory =
    lancet::TrackingDeviceSourceConfiguratorLancet::New(m_KukaToolStorage, kukaTrackingDevice);

  m_KukaSource = kukaSourceFactory->CreateTrackingDeviceSource(m_KukaVisualizer);

  m_KukaSource->Connect();
  m_KukaSource->StartTracking();

  //update visualize filter by timer
  if (m_KukaVisualizeTimer == nullptr)
  {
    m_KukaVisualizeTimer = new QTimer(this);  //create a new timer
  }
  connect(m_KukaVisualizeTimer, SIGNAL(timeout()), this, SLOT(OnKukaVisualizeTimer())); //connect the timer to the method OnTimer()

  m_KukaVisualizeTimer->start(100);  //Every 100ms the method OnTimer() is called. -> 10fps
}

void SurgicalSimulate::OnKukaVisualizeTimer()
{
  //Here we call the Update() method from the Visualization Filter. Internally the filter checks if
 //new NavigationData is available. If we have a new NavigationData the cone position and orientation
 //will be adapted.
  m_KukaVisualizer->Update();

  // auto geo = this->GetDataStorage()->ComputeBoundingGeometry3D(this->GetDataStorage()->GetAll());
  // mitk::RenderingManager::GetInstance()->InitializeViews(geo);
  this->RequestRenderWindowUpdate();
}

void SurgicalSimulate::OnVegaVisualizeTimer()
{
  //Here we call the Update() method from the Visualization Filter. Internally the filter checks if
 //new NavigationData is available. If we have a new NavigationData the cone position and orientation
 //will be adapted.
  m_VegaVisualizer->Update();

  // auto geo = this->GetDataStorage()->ComputeBoundingGeometry3D(this->GetDataStorage()->GetAll());
  // mitk::RenderingManager::GetInstance()->InitializeViews(geo);
  this->RequestRenderWindowUpdate();
}

void SurgicalSimulate::OnRobotCapture()
{
  if (m_IndexOfRobotCapture<5) //The first five translations, 
  {
    CapturePose(true);
    //Increase the count each time you click the button
    m_IndexOfRobotCapture++;
    MITK_INFO << "OnRobotCapture: " << m_IndexOfRobotCapture;
  }
  else if (m_IndexOfRobotCapture < 10)//the last five rotations
  {
    CapturePose(false);
    //Increase the count each time you click the button
    m_IndexOfRobotCapture++;
    MITK_INFO << "OnRobotCapture: " << m_IndexOfRobotCapture;
  }
  else
  {
    m_RobotRegistration->Regist();
    vtkMatrix4x4* matrix4x4 = nullptr;
    m_RobotRegistration->GetRegistraionMatrix(matrix4x4);
    MITK_INFO << "OnRobotCapture finish: " << m_IndexOfRobotCapture;
    matrix4x4->Print(std::cout);

    mitk::AffineTransform3D::Pointer affine_transform = mitk::AffineTransform3D::New();

    mitk::TransferVtkMatrixToItkTransform(matrix4x4, affine_transform.GetPointer());

    m_KukaVisualizer->SetOffset(0, affine_transform);
  }


  

}


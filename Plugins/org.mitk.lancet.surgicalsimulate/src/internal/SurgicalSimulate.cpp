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
  //! [UseVega 1]
  //Here we want to use the NDI Polaris tracking device. Therefore we instantiate a object of the class
      //NDITrackingDevice and make some settings which are necessary for a proper connection to the device.
  MITK_INFO << "NDI tracking";
  QMessageBox::warning(nullptr, "Warning", "You have to set the parameters for the NDITracking device inside the code (QmitkIGTTutorialView::OnStartIGT()) before you can use it.");
  lancet::NDIVegaTrackingDevice ::Pointer vega = lancet::NDIVegaTrackingDevice::New();  //instantiate
  vega->SethostName("192.168.1.10");//set the device IP
  vega->SetTrackingFrequency(2);//set tracking frequency to 60hz
  
  //! [UseVega 2]
  //The tools represent the sensors of the tracking device. In this case we have a Probe and a Object Reference Frame(ObjRF).
  //The TrackingDevice object it self fills the tool with data. So we have to add the tool to the
  //TrackingDevice object.
  // The ndi vega system needs a ".rom" file which describes the geometry of the markers related to the tool tip.
  //NDI provides an own software (NDI Cygna 6D) to generate those files.
  vega->AddTool("Probe", "c:\\myinstrument.rom");//todo change rom file path
  vega->AddTool("ObjRF", "c:\\myinstrument.rom");//todo change rom file path
  

  //! [UseVega 3]
    //The tracking device object is used for the physical connection to the device. To use the
    //data inside of our tracking pipeline we need a source. This source encapsulate the tracking device
    //and provides objects of the type mitk::NavigationData as output. The NavigationData objects stores
    //position, orientation, if the data is valid or not and special error informations in a covariance
    //matrix.
    //
    //Typically the start of a pipeline is a TrackingDeviceSource. To work correct we have to set a
    //TrackingDevice object. Attention you have to set the tools before you set the whole TrackingDevice
    //object to the TrackingDeviceSource because the source need to know how many outputs should be
    //generated.
  m_VegaSource = mitk::TrackingDeviceSource::New();   //We need the filter objects to stay alive,
  //therefore they must be members.
  m_VegaSource->SetTrackingDevice(vega); //Here we set the tracking device to the source of the pipeline.

   //! [UseVega 4]
  m_KukaSource->Connect();                  //Now we connect to the tracking system.
  //Note we do not call this on the TrackingDevice object
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

  auto geo = this->GetDataStorage()->ComputeBoundingGeometry3D(this->GetDataStorage()->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeViews(geo);
  this->RequestRenderWindowUpdate();
}


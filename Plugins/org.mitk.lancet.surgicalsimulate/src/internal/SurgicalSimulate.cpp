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
#include <NDIVegaTrackingDevice.h>
#include <kukaRobotDevice.h>

const std::string SurgicalSimulate::VIEW_ID = "org.mitk.views.surgicalsimulate";

void SurgicalSimulate::SetFocus()
{
  m_Controls.pushButton_ndiconnect->setFocus();
}

void SurgicalSimulate::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  //connect(m_Controls.buttonPerformImageProcessing, &QPushButton::clicked, this, &SurgicalSimulate::DoImageProcessing);
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
  //! [UseKuka 1]
  //Here we want to use the Kuka robot as a tracking device. Therefore we instantiate a object of the class
      //NDITrackingDevice and make some settings which are necessary for a proper connection to the device.
  MITK_INFO << "NDI tracking";
  QMessageBox::warning(nullptr, "Warning", "You have to set the parameters for the NDITracking device inside the code (QmitkIGTTutorialView::OnStartIGT()) before you can use it.");
  lancet::KukaRobotDevice::Pointer kuka = lancet::KukaRobotDevice::New();  //instantiate

  //! [UseKuka 2]
  //The tools represent the sensors of the tracking device. In this case we have a test tool define a line.
  //The TrackingDevice object it self fills the tool with data. So we have to add the tool to the
  //TrackingDevice object.
  // The kuka system needs a tcp file which describes the geometry of the end tool related to the robot flange.
  //the Tcp should either be calibrated or use hardware design value.
  kuka->AddTool("TestLine");//todo tcp should be set later

  //! [UseKuka 3]
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
  m_KukaSource = mitk::TrackingDeviceSource::New();   //We need the filter objects to stay alive,
  //therefore they must be members.
  m_KukaSource->SetTrackingDevice(kuka); //Here we set the tracking device to the source of the pipeline.

  //! [UseKuka 4]
  m_KukaSource->Connect();                  //Now we connect to the tracking system.
  //Note we do not call this on the TrackingDevice object
}


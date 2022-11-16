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
#include "QLinkingHardware.h"

// Qt

#include <QDir>
#include <QMessageBox>

// mitk image
#include <mitkImage.h>

#include <berryIQtStyleManager.h>
#include "org_mitk_lancet_linkinghardware_Activator.h"
#include <lancetIDevicesAdministrationService.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextCodec>
#include <QJsonValue> 
#include <QJsonObject> 
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>

const std::string QLinkingHardware::VIEW_ID = "org.mitk.views.qlinkinghardware";

void QLinkingHardware::SetFocus(){}
QLinkingHardware::~QLinkingHardware()
{
}
void QLinkingHardware::CreateQtPartControl(QWidget *parent)
{ 
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent); 
  m_Controls.pushButton_success->setEnabled(true);
  auto test_dir = QDir(":/org.mitk.lancet.linkinghardware/");
  QFile qss(test_dir.absoluteFilePath("linkinghardware.qss"));

  if (!qss.open(QIODevice::ReadOnly))
  {
	  
	  qWarning() << __func__ << __LINE__ << ":" << "error load file "
		  << test_dir.absoluteFilePath("linkinghardware.qss") << "\n"
		  << "error: " << qss.errorString();
  }
  // pos
  qInfo() << "log.file.pos " << qss.pos();

  m_Controls.widget->setStyleSheet(QLatin1String(qss.readAll()));
  qss.close();

  connect(m_Controls.pushButton_auto, &QPushButton::clicked, this, &QLinkingHardware::on_pb_auto_clicked);
  connect(m_Controls.pushButton_success, &QPushButton::clicked, this, &QLinkingHardware::on_pb_success_clicked);
  
  if (this->GetService())
  {
	  this->ConnectToService();
  }
}
lancet::IDevicesAdministrationService* QLinkingHardware::GetService() const
{
	auto context = mitk::PluginActivator::GetPluginContext();
	auto serviceRef = context->getServiceReference<lancet::IDevicesAdministrationService>();
	return context->getService<lancet::IDevicesAdministrationService>(serviceRef);
}
void QLinkingHardware::ConnectToService()
{
	auto sender = this->GetService();
	if (sender)
	{
		lancet::IDevicesAdministrationService* o = sender;
		QObject::connect(o, &lancet::IDevicesAdministrationService::TrackingDeviceStateChange,
			this, &QLinkingHardware::Slot_IDevicesGetStatus);
	}
}
void QLinkingHardware::Slot_IDevicesGetStatus(std::string name, lancet::TrackingDeviceManage::TrackingDeviceState State)
{
    MITK_INFO << "QLinkingHardware:" << __func__ << ": log";
    auto scanner = this->GetService()->GetConnector();
    bool isConnected = State & lancet::TrackingDeviceManage::TrackingDeviceState::Connected;
    this->setStartHardware(name, isConnected);
}
void QLinkingHardware::setStartHardware(std::string name, bool isConnected)
{
	MITK_INFO << "QLinkingHardware:" << __func__  << ": log";
    if (name == "Vega")
    {
        m_Controls.checkBox_startNDI->setChecked(isConnected);
    }
    if (name == "Robot")
    {
        m_Controls.checkBox_startRobot->setChecked(isConnected);
        if (true == isConnected)
        {
            auto connector = this->GetService()->GetConnector();
            auto robot = connector->GetTrackingDevice("Robot");
            mitk::NavigationData::Pointer rob_move = connector->GetTrackingDeviceSource("Robot")->GetOutput(0);
            m_RobotStartPosition = rob_move->GetPosition();
            m_updateTimer.start();
            connect(&this->m_updateTimer, &QTimer::timeout, this, &QLinkingHardware::startCheckRobotMove);
        }
    }

    // ����豸�����ӳɹ�����ô������תͨ��
    bool isPass = m_Controls.checkBox_startNDI->isChecked() && m_Controls.checkBox_startRobot->isChecked();
    m_Controls.pushButton_success->setEnabled(isPass);
    if (m_Controls.pushButton_success->isEnabled())
    {
        m_Controls.pushButton_auto->setText(QString::fromLocal8Bit("���ɹ�"));
    }
    else
    {
        m_Controls.pushButton_auto->setEnabled(true);
        m_Controls.pushButton_auto->setText(QString::fromLocal8Bit("���¼��"));
    }
}

void QLinkingHardware::ReadFileName()
{
    auto test_dir = QDir(":/org.mitk.lancet.linkinghardware/");
    QFile file(test_dir.absoluteFilePath("config.json"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }
    QByteArray array = file.readAll();
    QJsonParseError json_error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(array, &json_error);

    if (json_error.error == QJsonParseError::NoError)
    {
        QJsonObject rootObj = jsonDoc.object();
        QJsonArray  JsonArray;
        if (rootObj.contains("Device") && rootObj.value("Device").isObject())
        {
            QJsonObject  childJson = rootObj.value("Device").toObject();
            JsonArray = childJson.value("IGTToolStorage").toArray();
            auto connector = this->GetService()->GetConnector(); 
            for (int i = 0; i < JsonArray.size(); i++)
            {
                QJsonObject childObj = JsonArray[i].toObject();
                if (childObj.contains("ndifilename") && childObj.value("ndifilename").isString())
                {
                    filename = childObj.value("ndifilename").toString();
                    QDir dir(filename);
                    MITK_INFO << dir.absolutePath().toStdString();
                    MITK_INFO  << filename;
                    if (false == connector->IsInstallTrackingDevice("Vega"))
                    {
                        if (false == connector->InstallTrackingDevice("Vega", filename.toStdString(), this->GetDataStorage()))
                        {
                            std::cout << "install robot tracking device faild! error: " << connector->GetErrorString();
                        }
                        else
                        {
                            MITK_INFO << "Connecting to the Vega";
                            connector->GetTrackingDevice("Vega")->OpenConnection();
                            connector->GetTrackingDevice("Vega")->StartTracking();
                        }
                    }
                }

                if (childObj.contains("robfilename") && childObj.value("robfilename").isString())
                {
                    filename = childObj.value("robfilename").toString();
                    MITK_INFO  << filename; 
                    if (false == connector->IsInstallTrackingDevice("Robot"))
                    {
                        if (false == connector->InstallTrackingDevice("Robot", filename.toStdString(), this->GetDataStorage()))
                        {
                            std::cout << "install robot tracking device faild! error: " << connector->GetErrorString();
                        }
                        else
                        {
                            MITK_INFO << "Connecting to the Robot";
                            connector->GetTrackingDevice("Robot")->OpenConnection();
                            connector->GetTrackingDevice("Robot")->StartTracking();
                        }
                    }
                }
                auto geo = this->GetDataStorage()->ComputeBoundingGeometry3D(this->GetDataStorage()->GetAll());
                mitk::RenderingManager::GetInstance()->InitializeViews(geo);
            }
        }
        if (filename.isNull()) return;
    }
}

void QLinkingHardware::on_pb_auto_clicked()
{
    MITK_INFO << "QLinkingHardware:" << __func__ << ": log";
    m_Controls.pushButton_auto->setEnabled(false);
    m_Controls.pushButton_auto->setText(QString::fromLocal8Bit("���ڼ��"));
	if (isauto)
    {
        ReadFileName();
    }
    else
    {
        auto geo = this->GetDataStorage()->ComputeBoundingGeometry3D(this->GetDataStorage()->GetAll());
        mitk::RenderingManager::GetInstance()->InitializeViews(geo);
    }
}
void QLinkingHardware::on_pb_success_clicked()
{
	//Log::write("QLinkingHardware::on_pushButton_success_clicked");
	MITK_INFO << "QLinkingHardware:" << __func__ << ": log";
}
void QLinkingHardware::startCheckRobotMove()
{
    MITK_INFO << "QLinkingHardware:" << __func__ << ": log";
    auto connector = this->GetService()->GetConnector();
    auto robot = connector->GetTrackingDevice("Robot");
    bool isRobotValid = false;
    if (robot)
    {
        if (connector->IsInstallTrackingDevice("Robot"))
        {
            isRobotValid = true;
            mitk::NavigationData::Pointer rob_move = connector->GetTrackingDeviceSource("Robot")->GetOutput(0);
            if (false == rob_move->IsDataValid()) 
            {
                return;
            }
            double Before_move[3]{ this->m_RobotStartPosition[0],this->m_RobotStartPosition[1],this->m_RobotStartPosition[2]};
            double After_move[3]{ rob_move->GetPosition()[0],rob_move->GetPosition()[1],rob_move->GetPosition()[2] };
            double MovingDistance;
            MovingDistance = sqrt((Before_move[0] - After_move[0]) * (Before_move[0] - After_move[0]) +
                (Before_move[1] - After_move[1]) * (Before_move[1] - After_move[1]) +
                (Before_move[2] - After_move[2]) * (Before_move[2] - After_move[2]));
            if (MovingDistance > 5.0)
            {
                m_Controls.checkBox_free->setChecked(true);
                this->m_updateTimer.stop();
            }
        }
    }  
}
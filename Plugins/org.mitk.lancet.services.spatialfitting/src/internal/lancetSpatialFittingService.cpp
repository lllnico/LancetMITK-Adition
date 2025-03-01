#include "lancetSpatialFittingService.h"
#include "lancetSpatialFittingGlobal.h"
#include "core/lancetSpatialFittingPipelineManager.h"
#include "internal/lancetSpatialFittingRoboticsRegisterModel.h"
#include <lancetIDevicesAdministrationService.h>
#include <internal/lancetTrackingDeviceManage.h>
namespace lancet
{
	SpatialFittingService::SpatialFittingService()
	{
		this->Initialize();
		connect(
			this->GetDeviceService()->GetConnector().GetPointer(),  // sender
			SIGNAL(TrackingDeviceStateChange(std::string, lancet::TrackingDeviceManage::TrackingDeviceState)),
			this, // recv
			SLOT(onDeviceConnectState_change(std::string, lancet::TrackingDeviceManage::TrackingDeviceState)));
	}

	void SpatialFittingService::Initialize()
	{
		MITK_DEBUG << "log";
		if (this->GetRoboticsRegisterModel().IsNull())
		{
			lancet::spatial_fitting::RoboticsRegisterModel::Pointer model
				= lancet::spatial_fitting::RoboticsRegisterModel::Pointer(new lancet::spatial_fitting::RoboticsRegisterModel);

			this->SetRoboticsRegisterModel(model);
		}

		auto toolDataStorage = this->GetDeviceService()->GetConnector()->GetNavigationToolStorage("Vega");
		if (toolDataStorage.IsNotNull())
		{
			for (int index = 0; index < toolDataStorage->GetToolCount(); ++index)
			{
				MITK_ERROR << "read tool data storage [" << index << "]: " << toolDataStorage->GetTool(index)->GetToolName();
			}
		}

		auto toolDataStorage2 = this->GetDeviceService()->GetConnector()->GetNavigationToolStorage("Kuka");
		if (toolDataStorage2.IsNotNull())
		{
			for (int index = 0; index < toolDataStorage2->GetToolCount(); ++index)
			{
				MITK_ERROR << "read tool data storage [" << index << "]: " << toolDataStorage2->GetTool(index)->GetToolName();
			}
		}
			auto ndidatasource = this->GetDeviceService()->GetConnector()->GetTrackingDeviceSource("Vega");
			this->GetRoboticsRegisterModel()->SetNdiNavigationDataSource(ndidatasource);

			auto robotdatasource = this->GetDeviceService()->GetConnector()->GetTrackingDeviceSource("Kuka");
			this->GetRoboticsRegisterModel()->SetRoboticsNavigationDataSource(robotdatasource);

			this->GetRoboticsRegisterModel()->ConfigureRegisterPipeline();

			this->GetRoboticsRegisterModel()->ConfigureVerifyPipeline();
	}

	void SpatialFittingService::onDeviceConnectState_change(std::string name, lancet::TrackingDeviceManage::TrackingDeviceState state)
	{
	  MITK_INFO << "log.name " << name << "; log.state " << state;
	  if(state == lancet::TrackingDeviceManage::UnInstall)
	  {
		MITK_WARN << "The target is not ready for effective resources.";
		return;
	  }

	  this->Initialize();
	}
}



#ifndef KUKAROBOTDEVICE_H
#define KUKAROBOTDEVICE_H
//mitk
#include <mitkCommon.h>
#include <mitkTrackingDevice.h>

//KUKA ROBOT API
#include "MitkLancetIGTExports.h"
#include "robotapi.h"

//qt
#include <QObject>
#include <QThread>
#include <QMetaType>
#include <QPointer>

#include "mitkNDIPassiveTool.h"
#include "mitkTrackingTool.h"


/** Documentation
  * \brief superclass for specific KUKA Robot Devices that use socket communication.
  *
  * implements the TrackingDevice interface for Kuka robot devices ()
  *
  * \ingroup Robot
 */
class MITKLANCETIGT_EXPORT  KukaRobotDevice :public QObject,public  mitk::TrackingDevice
{
	Q_OBJECT
public:
	mitkClassMacro(KukaRobotDevice, TrackingDevice);
	itkFactorylessNewMacro(Self);
	itkCloneMacro(Self);

  typedef std::vector<mitk::TrackingTool::Pointer> Tool6DContainerType;

	itkGetMacro(IsConnected, bool);
  /**
     * @brief Opens the connection to the device. This have to be done before the tracking is started.
     */

	bool OpenConnection() override;
  /**
     * @brief Closes the connection and clears all resources.
     */
	bool CloseConnection() override;
  /**
      * @brief Starts the tracking.
      * @return Returns true if the tracking is started. Returns false if there was an error.
      */
	bool StartTracking() override;
  /**
     * \param toolNumber The number of the tool which should be given back.
     * \return Returns the tool which the number "toolNumber". Returns nullptr, if there is
     * no tool with this number.
     */
	mitk::TrackingTool* GetTool(unsigned toolNumber) const override;
  mitk::TrackingTool* GetToolByName(std::string name) const override;

  mitk::TrackingTool* AddTool(const char* toolName, const char* fileName);
	unsigned GetToolCount() const override;

	void TrackTools();

	std::array<double,6> GetTrackingData();

  //robotDevice api, move to RobotDevice abstract class later
  bool RequestExecOperate(const QString& funname, const QStringList& param);

protected:
  KukaRobotDevice();
  virtual ~KukaRobotDevice() override;

  /**
    * \brief Add a passive 6D tool to the list of tracked tools and add tool tcp to robot. This method is used by AddTool
    * @throw mitk::IGTHardwareException Throws an exception if there are errors while adding the tool.
    * \warning adding tools is not possible in tracking mode, only in setup and ready.
    */
  virtual bool InternalAddTool(mitk::TrackingTool* tool); //todo create new tcp in robot

private slots:
	void IsRobotConnected(bool isConnect);

private:
	static void heartbeatThreadWorker(KukaRobotDevice* _this);
  void ThreadStartTracking();
private:
	bool m_IsConnected = false;
	TrackingDeviceState m_State = Setup;///< current object state (Setup, Ready or Tracking)
	RobotApi m_RobotApi;
	QPointer<QThread> m_Heartbeat;
	std::array<double, 6> m_TrackingData = {};
  std::string m_IpAddress { "172.31.1.148" } ;
  std::string m_Port  {"30300"};
  std::string m_RemoteIpAddress  {"172.31.1.147"};
  std::string m_RemotePort {"30300"};
  std::string m_DeviceName{"KUKA"};

  //track
  mutable std::mutex m_ToolsMutex; ///< mutex for coordinated access of tool container
  Tool6DContainerType m_6DTools; ///< container for all tracking tools
  ///< creates tracking thread that continuously polls serial interface for new tracking data
  std::thread m_Thread;                            ///< ID of tracking thread
};




#endif // KUKAROBOTDEVICE_H

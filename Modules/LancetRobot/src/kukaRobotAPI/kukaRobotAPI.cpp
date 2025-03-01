#include "kukaRobotAPI/kukaRobotAPI.h"

#include <sys/timeb.h>

#include "kukaRobotAPI/defaultProtocol.h"
#include "kukaRobotAPI/robotInfoProtocol.h"

#define PI acos(-1)

std::string lancet::KukaRobotAPI::GetApiRevision()
{
  return "0.1.0";
}

bool lancet::KukaRobotAPI::Connect(int tcpPort, std::string robotIP, int udpPort)
{
  //connect tcp to send robot command and receive result
  if (m_TcpConnection.connect(tcpPort))
  {
    m_IsConnected = true;
    //Send heartbeat packets through TCP to maintain connection with kuka robot application;
    m_TcpHeartBeatThread = std::thread(&KukaRobotAPI::sendTcpHeartBeat, this);
    m_ReadRobotResultThread = std::thread(&KukaRobotAPI::threadReadRobotResult, this);
  }

  //connect udp to receive robot info
  m_UdpConnection.connect(robotIP, udpPort);

  return true;
}

void lancet::KukaRobotAPI::DisConnect()
{
  m_IsConnected = false;
  m_TcpHeartBeatThread.join();
  m_ReadRobotResultThread.join();
  m_TcpConnection.disconnect();
  m_UdpConnection.disconnect();
}

bool lancet::KukaRobotAPI::SendCommandNoPara(std::string cmd) const
{
  DefaultProtocol protocol;
  protocol.operateType = cmd;
  protocol.timestamp = timeStamp();
  //record to log
  std::stringstream jsnString;
  protocol.ToJsonObj().stringify(jsnString); //dont use indent here,jsnString send to robot and keep it one line;
  MITK_INFO << jsnString.str();
  //send to robot
  const auto res = m_TcpConnection.write(jsnString.str());
  return res;
}

ResultProtocol lancet::KukaRobotAPI::GetCommandResult()
{
  ResultProtocol result{};
  if (m_MsgQueue.empty())
  {
    return result;
  }
  m_MsgQueueMutex.lock();
  std::string msg = m_MsgQueue.front();
  m_MsgQueue.pop();
  m_MsgQueueMutex.unlock();

  Poco::JSON::Parser parser;
  parser.reset();

  Poco::Dynamic::Var var_result = parser.parse(msg);
  auto pObj = var_result.extract<Poco::JSON::Object::Ptr>();
  //record to log
  std::stringstream jsnString;
  pObj->stringify(jsnString, 2);
  MITK_INFO << jsnString.str();

  result.FromJsonObj(*pObj);
  return result;
}

RobotInformationProtocol lancet::KukaRobotAPI::GetRobotInfo()
{
  RobotInformationProtocol info{};

  Poco::JSON::Parser parser;
  parser.reset();
  Poco::Dynamic::Var result = parser.parse(m_UdpConnection.read());//todo crush when disconnect ,add err handle
  auto pObj = result.extract<Poco::JSON::Object::Ptr>();
  if (pObj.isNull())
  {
    MITK_ERROR << "GetRobotInfo Failed: Can not parse udp massage to json Object,empty info returned";
    return info;
  }

  if (!info.FromJsonObj(*pObj))
  {
    MITK_ERROR << "GetRobotInfo Failed: json format error,empty info returned.please check udp massage below:";
    std::stringstream ss;
    pObj->stringify(ss, 2);
    MITK_ERROR << ss.str();
  }
  return info;
}

bool lancet::KukaRobotAPI::AddFrame(std::string name, std::array<double, 6> xyzabc) const
{
  DefaultProtocol protocol;
  protocol.operateType = "AddFrame";
  protocol.timestamp = timeStamp();
  protocol.param.target = std::move(name);
  protocol.param.FromArray(xyzabc);
  //convet obj to json format string
  std::stringstream jsnString;
  protocol.ToJsonObj().stringify(jsnString);
  std::cout << jsnString.str() << std::endl;
  //send to robot
  const bool res = m_TcpConnection.write(jsnString.str());

  return res;
}

bool lancet::KukaRobotAPI::SetMotionFrame(std::string name)
{
  DefaultProtocol protocol;
  protocol.operateType = "SetMotionFrame";
  protocol.timestamp = timeStamp();
  protocol.param.target = std::move(name);
  //convet obj to json format string
  std::stringstream jsnString;
  protocol.ToJsonObj().stringify(jsnString);
  std::cout << jsnString.str() << std::endl;
  //send to robot
  //todo handle write error
  return m_TcpConnection.write(jsnString.str());
}

bool lancet::KukaRobotAPI::RunBrakeTest() const
{
  return SendCommandNoPara("RunBrakeTest");
}

bool lancet::KukaRobotAPI::MoveStop()
{
  return SendCommandNoPara("MoveStop");
}

bool lancet::KukaRobotAPI::MovePTP(vtkMatrix4x4* target)
{
  DefaultProtocol protocol;
  protocol.operateType = "MovePTP";
  protocol.timestamp = timeStamp();
  //
  std::array<double, 6> xyzabc = kukaTransformMatrix2xyzabc(target);
  protocol.param.FromArray(xyzabc);
  //convet obj to json format string
  std::stringstream jsnString;
  protocol.ToJsonObj().stringify(jsnString);
  std::cout << jsnString.str() << std::endl;
  //send to robot
  return m_TcpConnection.write(jsnString.str());
}

bool lancet::KukaRobotAPI::MovePTP(std::array<double, 6> xyzabc)
{
	DefaultProtocol protocol;
	protocol.operateType = "MovePTP";
	protocol.timestamp = timeStamp();
	protocol.param.FromArray(xyzabc);
	//convet obj to json format string
	std::stringstream jsnString;
	protocol.ToJsonObj().stringify(jsnString);
	std::cout << jsnString.str() << std::endl;
	//send to robot
	return m_TcpConnection.write(jsnString.str());
}

bool lancet::KukaRobotAPI::HandGuiding() const
{
  return SendCommandNoPara("HandGuiding");
}

lancet::KukaRobotAPI::~KukaRobotAPI()
{
  this->DisConnect();
}

void lancet::KukaRobotAPI::sendTcpHeartBeat() const
{
  while (m_IsConnected)
  {
    auto res = m_TcpConnection.write("heartBeat");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    //MITK_INFO << "-";
  }
}

void lancet::KukaRobotAPI::threadReadRobotResult()
{
  while (m_IsConnected)
  {
    std::string msg;
    if (m_TcpConnection.read(msg))
    {
      m_MsgQueueMutex.lock();
      m_MsgQueue.push(msg);
      m_MsgQueueMutex.unlock();
      MITK_INFO << "MsgQueue Size: " << m_MsgQueue.size();
    }
  }
}

long lancet::KukaRobotAPI::timeStamp()
{
  timeb t{};
  ftime(&t);
  return t.time * 1000 + t.millitm;
}

std::vector<double> lancet::KukaRobotAPI::kukamatrix2angle(const double matrix3x3[3][3])
{
  double nx = matrix3x3[0][0];
  double ny = matrix3x3[0][1];
  double nz = matrix3x3[0][2];
  double ox = matrix3x3[1][0];
  double oy = matrix3x3[1][1];
  double oz = matrix3x3[1][2];
  double ax = matrix3x3[2][0];
  double ay = matrix3x3[2][1];
  double az = matrix3x3[2][2];

  //���Rx��Ry��Rz
  //call Atan2(ax,sqrt(power(nx,2)+power(ox,2)),ry1)
  //double cry = (double)sqrt(nx * nx + ox * ox);
  double rx, ry, rz, rx2, ry2, rz2, rx_Origin, ry_Origin, rz_Origin;
  double FlagRx = 0.0, FlagRy = 0.0, FlagRz = 0.0;
  ry = atan2(ax, sqrt(nx * nx + ox * ox));
  if (ry == 90)
  {
    rx = 0;
    //call Atan2(ny,-nz,rz1)
    rz = (atan2(ny, -nz));
  }
  else if (ry == -90)
  {
    rx = 0;
    rz = (atan2(ny, nz));
    //call Atan2(ny,nz,rz1);
  }
  else
  {
    //call Atan2(-ay/cos(ry1),az/cos(ry1),rx1)
    //call Atan2(-ox/cos(ry1),nx/cos(ry1),rz1)
    rx = (atan2(-ay / cos(ry), az / cos(ry)));
    rz = (atan2(-ox / cos(ry), nx / cos(ry)));
  }

  //call Atan2(ax,-sqrt(power(nx,2)+power(ox,2)),ry2)
  ry2 = (atan2(ax, -sqrt(nx * nx + ox * ox)));
  if (ry2 == 90)
  {
    rx2 = 0;
    //call Atan2(ny,-nz,rz1)
    rz2 = (atan2(ny, -nz));
  }
  else if (ry2 == -90)
  {
    rx2 = 0;
    rz2 = (atan2(ny, nz));
    //call Atan2(ny,nz,rz1);
  }
  else
  {
    //call Atan2(-ay/cos(ry1),az/cos(ry1),rx1)
    //call Atan2(-ox/cos(ry1),nx/cos(ry1),rz1)
    rx2 = (atan2(-ay / cos(ry2), az / cos(ry2)));
    rz2 = (atan2(-ox / cos(ry2), nx / cos(ry2)));
  }

  std::cout << "rz11:" << -rz * (180.0 / PI) << "   ry:" << -ry * (180.0 / PI) << "   rx:" << -rx * (180.0 / PI) <<
    "\n";
  std::cout << "rz22:" << -rz2 * (180.0 / PI) << "   ry:" << -ry2 * (180.0 / PI) << "   rx:" << -rx2 * (180.0 / PI)
    << "\n";
  if (abs(rx) + abs(ry) + abs(rz) <= abs(rx2) + abs(ry2) + abs(rz2))
  {
    rx_Origin = -rx;
    ry_Origin = -ry;
    rz_Origin = -rz;
  }
  else
  {
    rx_Origin = -rx2;
    ry_Origin = -ry2;
    rz_Origin = -rz2;
  }
  std::vector<double> i;


  if (abs(ry * (180.0 / PI)) < 90)
  {
    i.push_back(-rz * (180.0 / PI));
    i.push_back(-ry * (180.0 / PI));
    i.push_back(-rx * (180.0 / PI));
  }
  else
  {
    i.push_back(-rz2 * (180.0 / PI));
    i.push_back(-ry2 * (180.0 / PI));
    i.push_back(-rx2 * (180.0 / PI));
  }
  //MITK_INFO << i[0] << "," << i[1] << "," << i[2];
  return i;
}

std::array<double, 6> lancet::KukaRobotAPI::kukaTransformMatrix2xyzabc(vtkMatrix4x4* matrix4x4)
{
  const double R[3][3] = {
    {matrix4x4->GetElement(0, 0), matrix4x4->GetElement(0, 1), matrix4x4->GetElement(0, 2)},
    {matrix4x4->GetElement(1, 0), matrix4x4->GetElement(1, 1), matrix4x4->GetElement(1, 2)},
    {matrix4x4->GetElement(2, 0), matrix4x4->GetElement(2, 1), matrix4x4->GetElement(2, 2)}
  };

  std::vector<double> abc = kukamatrix2angle(R);
  std::array<double, 6> res{};

  res[0] = matrix4x4->GetElement(0, 3); ///< x
  res[1] = matrix4x4->GetElement(1, 3); ///< y
  res[2] = matrix4x4->GetElement(2, 3); ///< z

  res[3] = abc[0]; ///< a
  res[4] = abc[1]; ///< b
  res[5] = abc[2]; ///< c

  return res;
}

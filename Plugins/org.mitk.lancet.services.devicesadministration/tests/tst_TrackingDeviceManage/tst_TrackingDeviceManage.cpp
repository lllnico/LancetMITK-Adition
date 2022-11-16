#include <QTest>

#include <src/internal/lancetTrackingDeviceManage.h>

#include <mitkTrackingDevice.h>
#include <mitkNavigationToolStorage.h>

/// �ⲿ����Ĳ�������Ŀ¼
/// eg. TEST_DATAS_DIR_PATH >> Y:/LancetMitk_SBD/MITK-build/bin/tst_datas
#define TEST_DATAS_DIR_PATH	QDir("../../../../bin/tst_datas").absolutePath()

/// ���ݲ������������ⲿ����Ĳ��������ļ��ľ���·��
/// eg. TEST_DATAS_FILENAME_PATH("test.IGT") >> Y:/LancetMitk_SBD/MITK-build/bin/tst_datas/test.IGT
#define TEST_DATAS_FILENAME_PATH(filename)	QDir("../../../../bin/tst_datas").absoluteFilePath(filename)

class tst_TrackingDeviceManage 
  : public QObject 
{
  Q_OBJECT
public:
  struct TestTrackingDeviceManageData
  {
		QString deviceName;
    int loadErrorCode; // *.IGT�ļ����ش������
    QString toolDataStroageFileName;

		TestTrackingDeviceManageData()
			: loadErrorCode(0), deviceName(), toolDataStroageFileName() {}
		TestTrackingDeviceManageData(QString devName, QString toolDataStroageFile, int ec = 0) 
			: deviceName(devName)
			, toolDataStroageFileName(toolDataStroageFile)
			, loadErrorCode(ec)
		{
		}
	};

  /**
   * ���߼��������ݽṹ��
   */
	struct TestToolDataStorageData
	{
		int toolCount; // ����
    int loadErrorCode; // *.IGT�ļ����ش������
		QString toolDataStroageFileName; // *.IGT �ļ�
    QList<QString> listToolNames; // ���߰��еĹ���Ԫ�����ƣ���{"Probe"��"Markers"}

    TestToolDataStorageData()
      : toolCount(0), loadErrorCode(0), toolDataStroageFileName(), listToolNames(){}

    TestToolDataStorageData(int c, QString fp, QList<QString> tools, int ec = 0) {
      this->loadErrorCode = ec;
      this->toolCount = c;
      this->toolDataStroageFileName = fp;
      this->listToolNames = tools;
    }
	};
private slots:
  void initTestCase()
  {
		qDebug() << "QDir.tst.TEST_DATAS_DIR_PATH " << TEST_DATAS_DIR_PATH;
		qDebug() << "QDir.tst.TEST_DATAS_FILENAME_PATH(fp.IGT) " << TEST_DATAS_FILENAME_PATH("fp.IGT");
  }

  void tst_InstallTrackingDevice_ToolDataStorage_Success_data()
  {
    QTest::addColumn<TestToolDataStorageData>("ToolDataStorageFiles");

    QTest::newRow("ToolDataStorage_Success_001") << TestToolDataStorageData(1, "", {});
		QTest::newRow("ToolDataStorage_Success_002") << TestToolDataStorageData(1, "", {});
		QTest::newRow("ToolDataStorage_Success_003") << TestToolDataStorageData(1, "", {});
		QTest::newRow("ToolDataStorage_Success_004") << TestToolDataStorageData(1, "", {});
  }
  void tst_InstallTrackingDevice_ToolDataStorage_Faild_data()
  {
		QTest::addColumn<TestToolDataStorageData>("ToolDataStorageFiles");

		QTest::newRow("ToolDataStorage_Faild_001") << TestToolDataStorageData(1, "", {});
		QTest::newRow("ToolDataStorage_Faild_002") << TestToolDataStorageData(1, "", {});
		QTest::newRow("ToolDataStorage_Faild_003") << TestToolDataStorageData(1, "", {});
		QTest::newRow("ToolDataStorage_Faild_004") << TestToolDataStorageData(1, "", {});
	}
	void tst_InstallTrackingDevice_Success_data()
	{
		QTest::addColumn<TestTrackingDeviceManageData>("TrackingDeviceManageData");

		QTest::newRow("InstallTrackingDevice_Success_001") << TestTrackingDeviceManageData("", "");
		QTest::newRow("InstallTrackingDevice_Success_002") << TestTrackingDeviceManageData("", "");
		QTest::newRow("InstallTrackingDevice_Success_003") << TestTrackingDeviceManageData("", "");
		QTest::newRow("InstallTrackingDevice_Success_004") << TestTrackingDeviceManageData("", "");
	}
	void tst_InstallTrackingDevice_Faild_data()
	{
		QTest::addColumn<TestTrackingDeviceManageData>("TrackingDeviceManageData");

		QTest::newRow("InstallTrackingDevice_Faild_001") << TestTrackingDeviceManageData("", "");
		QTest::newRow("InstallTrackingDevice_Faild_002") << TestTrackingDeviceManageData("", "");
		QTest::newRow("InstallTrackingDevice_Faild_003") << TestTrackingDeviceManageData("", "");
		QTest::newRow("InstallTrackingDevice_Faild_004") << TestTrackingDeviceManageData("", "");
	}

  void tst_InstallTrackingDevice_ToolDataStorage_Success()
  {
    QFETCH(TestToolDataStorageData, ToolDataStorageData);

    std::string deviceName = "Robot";

    lancet::TrackingDeviceManage::Pointer trackingDeviceManage = 
      lancet::TrackingDeviceManage::Pointer(new lancet::TrackingDeviceManage());

    /// <1: ��ȫ��ⱻ����Ŀ��
    QCOMPARE(true, trackingDeviceManage.IsNotNull());

    /// <2: ���Ա������豸��װ�ɹ�
    QCOMPARE(true, trackingDeviceManage->InstallTrackingDevice(deviceName, ToolDataStorageData.toolDataStroageFileName.toStdString()));

		/// <3: ��֤�豸�������ĸ�������
		auto deviceInstance = trackingDeviceManage->GetTrackingDevice(deviceName);
    /// <3.1: ���Ա������豸��װ�ɹ�����豸ʵ�������ڴ�, GetTrackingDevice ���ز�����ΪNull
		QCOMPARE(true, deviceInstance.IsNotNull());
		/// <3.2: �豸��װ�ɹ���IsInstallTrackingDevice ����true
		QCOMPARE(true, trackingDeviceManage->IsInstallTrackingDevice(deviceName));
		/// <3.3: �豸��װ�ɹ�����û��ʹ�豸����ͨ��ͨ�������� IsTrackingDeviceConnected ����false
		QCOMPARE(false, trackingDeviceManage->IsTrackingDeviceConnected(deviceName));
		/// <3.3: �豸��װ�ɹ�����û��ʹ�豸��ʼ���������� IsStartTrackingDevice ����false
		QCOMPARE(false, trackingDeviceManage->IsStartTrackingDevice(deviceName));
		/// <3.4: �豸��װ�ɹ��������豸������0��Ϊ1
		QCOMPARE(1, trackingDeviceManage->GetInstallTrackingDeviceSize());
    /// <3.5: �豸��װ�ɹ�����������װ�豸û�в����κδ������
    QCOMPARE(trackingDeviceManage->GetErrorCode(), ToolDataStorageData.loadErrorCode);

		/// <4: ���Ա������豸��װ�ɹ���Ĺ������ݼ��Ƿ���ȷ
    auto deviceToolDataStorageInstance = trackingDeviceManage->GetNavigationToolStorage(deviceName);
    /// <4.1: ����Ŀ���豸�Ĺ������ݼ��ڴ�����Ƿ���ȷ
		QCOMPARE(true, deviceToolDataStorageInstance.IsNotNull());
		/// <4.2: ����Ŀ���豸�Ĺ������ݼ����������Ƿ���ȷ
    QCOMPARE(ToolDataStorageData.toolCount, deviceToolDataStorageInstance->GetToolCount());
    /// <4.3: ����Ŀ���豸�Ĺ������ݼ��Ĺ��߼��Ƿ���ȷ
    for (QString toolName : ToolDataStorageData.listToolNames) 
    {
      QCOMPARE(true, deviceToolDataStorageInstance->GetToolByName(toolName.toStdString()).IsNotNull());
    }

    /// <5: ��֤�豸�������˳��豸�ӿ�
    QCOMPARE(true, trackingDeviceManage->UnInstallTrackingDevice(deviceName));
    /// <5.1: �豸ж�سɹ����豸���������� 1 ��Ϊ 0
    QCOMPARE(0, trackingDeviceManage->GetInstallTrackingDeviceSize());
  }

  void tst_InstallTrackingDevice_ToolDataStorage_Faild()
	{
		QFETCH(TestToolDataStorageData, ToolDataStorageData);
		std::string deviceName = "Robot";

		lancet::TrackingDeviceManage::Pointer trackingDeviceManage =
			lancet::TrackingDeviceManage::Pointer(new lancet::TrackingDeviceManage());

		/// <1: ��ȫ��ⱻ����Ŀ��
		QCOMPARE(true, trackingDeviceManage.IsNotNull());

		/// <2: ���Ա������豸��װʧ��
		QCOMPARE(true, trackingDeviceManage->InstallTrackingDevice(deviceName, ToolDataStorageData.toolDataStroageFileName.toStdString()));

    /// <2.1: �Աȴ������
    QCOMPARE(trackingDeviceManage->GetErrorCode(), ToolDataStorageData.loadErrorCode);

    /// <2.2: �豸��װʧ�ܣ��� IsInstallTrackingDevice �ӿڷ���false
    QCOMPARE(false ,trackingDeviceManage->IsInstallTrackingDevice(deviceName));

		/// <2.3: �豸��װʧ�ܣ��� IsTrackingDeviceConnected �ӿڷ���false
		QCOMPARE(false, trackingDeviceManage->IsTrackingDeviceConnected(deviceName));

		/// <2.4: �豸��װʧ�ܣ��� GetInstallTrackingDeviceSize �ӿڷ��� 0
		QCOMPARE(0, trackingDeviceManage->GetInstallTrackingDeviceSize());
  }

  void tst_InstallTrackingDevice_Success()
  {
		QFETCH(TestTrackingDeviceManageData, TrackingDeviceManageData);
		std::string deviceName = "Robot";

		lancet::TrackingDeviceManage::Pointer trackingDeviceManage =
			lancet::TrackingDeviceManage::Pointer(new lancet::TrackingDeviceManage());

		/// <1: ��ȫ��ⱻ����Ŀ��
		QCOMPARE(true, trackingDeviceManage.IsNotNull());

		/// <2: ���Ա������豸��װ�ɹ�
		QCOMPARE(true, trackingDeviceManage->InstallTrackingDevice(deviceName, TrackingDeviceManageData.toolDataStroageFileName.toStdString()));

		/// <3: ��֤�豸�������ĸ�������
		auto deviceInstance = trackingDeviceManage->GetTrackingDevice(deviceName);
		/// <3.1: ���Ա������豸��װ�ɹ�����豸ʵ�������ڴ�, GetTrackingDevice ���ز�����ΪNull
		QCOMPARE(true, deviceInstance.IsNotNull());
		/// <3.2: �豸��װ�ɹ���IsInstallTrackingDevice ����true
		QCOMPARE(true, trackingDeviceManage->IsInstallTrackingDevice(deviceName));
		/// <3.3: �豸��װ�ɹ�����û��ʹ�豸����ͨ��ͨ�������� IsTrackingDeviceConnected ����false
		QCOMPARE(false, trackingDeviceManage->IsTrackingDeviceConnected(deviceName));
		/// <3.3: �豸��װ�ɹ�����û��ʹ�豸��ʼ���������� IsStartTrackingDevice ����false
		QCOMPARE(false, trackingDeviceManage->IsStartTrackingDevice(deviceName));
		/// <3.4: �豸��װ�ɹ��������豸������0��Ϊ1
		QCOMPARE(1, trackingDeviceManage->GetInstallTrackingDeviceSize());
		/// <3.5: �豸��װ�ɹ�����������װ�豸û�в����κδ������
		QCOMPARE(trackingDeviceManage->GetErrorCode(), TrackingDeviceManageData.loadErrorCode);
		/// <3.6: ��֤��������-���ӻ����������¼��ʱ��
		QCOMPARE(500, trackingDeviceManage->GetNavigationDataFilterInterval());

		/// <4: ��֤�豸��Դ����
		/// <4.1: ��֤�豸���ӻ�����������
		QCOMPARE(true, trackingDeviceManage->GetNavigationDataToNavigationDataFilter(deviceName).IsNotNull());
		/// <4.2: ��֤�豸���ݹܵ�Դ����
		QCOMPARE(true, trackingDeviceManage->GetTrackingDeviceSource(deviceName).IsNotNull());
		/// <4.3: ��֤�豸���߰�����
		QCOMPARE(true, trackingDeviceManage->GetNavigationToolStorage(deviceName).IsNotNull());


		/// <5: ��֤�豸�������˳��豸�ӿ�
		QCOMPARE(true, trackingDeviceManage->UnInstallTrackingDevice(deviceName));
		/// <5.1: �豸ж�سɹ����豸���������� 1 ��Ϊ 0
		QCOMPARE(0, trackingDeviceManage->GetInstallTrackingDeviceSize());
		/// <5.2: �豸ж�سɹ����������ڲ�����״̬Ϊ0
		QCOMPARE(0, trackingDeviceManage->GetErrorCode());
  }

	void tst_InstallTrackingDevice_Faild()
	{
		QFETCH(TestTrackingDeviceManageData, TrackingDeviceManageData);
		std::string deviceName = "Robot";

		lancet::TrackingDeviceManage::Pointer trackingDeviceManage =
			lancet::TrackingDeviceManage::Pointer(new lancet::TrackingDeviceManage());

		/// <1: ��ȫ��ⱻ����Ŀ��
		QCOMPARE(true, trackingDeviceManage.IsNotNull());

		/// <2: ���Ա������豸��װ�ɹ�
		QCOMPARE(false, trackingDeviceManage->InstallTrackingDevice(deviceName, TrackingDeviceManageData.toolDataStroageFileName.toStdString()));

		/// <3: ��֤�豸�������ĸ�������
		auto deviceInstance = trackingDeviceManage->GetTrackingDevice(deviceName);
		/// <3.1: ���Ա������豸��װʧ�ܺ���豸ʵ�������ڴ�, GetTrackingDevice ����ΪNull
		QCOMPARE(false, deviceInstance.IsNotNull());
		/// <3.2: �豸��װʧ�ܣ�IsInstallTrackingDevice ����false
		QCOMPARE(false, trackingDeviceManage->IsInstallTrackingDevice(deviceName));
		/// <3.3: �豸��װʧ�ܣ����� IsTrackingDeviceConnected ����false
		QCOMPARE(false, trackingDeviceManage->IsTrackingDeviceConnected(deviceName));
		/// <3.3: �豸��װʧ�ܣ����� IsStartTrackingDevice ����false
		QCOMPARE(false, trackingDeviceManage->IsStartTrackingDevice(deviceName));
		/// <3.4: �豸��װʧ�ܣ������豸����Ϊ0
		QCOMPARE(0, trackingDeviceManage->GetInstallTrackingDeviceSize());
		/// <3.5: �豸��װʧ�ܣ���������װ�豸�����������
		QCOMPARE(trackingDeviceManage->GetErrorCode() != 0, true);
		/// <3.6: ��֤��������-���ӻ����������¼��ʱ��
		QCOMPARE(500, trackingDeviceManage->GetNavigationDataFilterInterval());

		/// <4: ��֤�豸��Դ����
		/// <4.1: ��֤�豸���ӻ�����������
		QCOMPARE(false, trackingDeviceManage->GetNavigationDataToNavigationDataFilter(deviceName).IsNotNull());
		/// <4.2: ��֤�豸���ݹܵ�Դ����
		QCOMPARE(false, trackingDeviceManage->GetTrackingDeviceSource(deviceName).IsNotNull());
		/// <4.3: ��֤�豸���߰�����
		QCOMPARE(false, trackingDeviceManage->GetNavigationToolStorage(deviceName).IsNotNull());


		/// <5: ��֤�豸�������˳��豸�ӿ�
		QCOMPARE(false, trackingDeviceManage->UnInstallTrackingDevice(deviceName));
	}

	void tst_ConnectedOrWorkingToTrackingDevice_Success()
	{
		// TODO: 
	}

	void tst_ConnectedOrWorkingToTrackingDevice_Faild()
	{
		// TODO:
	}
};

Q_DECLARE_METATYPE(tst_TrackingDeviceManage::TestToolDataStorageData)
Q_DECLARE_METATYPE(tst_TrackingDeviceManage::TestTrackingDeviceManageData)
QTEST_MAIN(tst_TrackingDeviceManage)
#include "tst_TrackingDeviceManage.moc"
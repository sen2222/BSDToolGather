#include "otapacketapp.h"


OtaPacketApp::OtaPacketApp()
{
    QString outputDirTmp =CONFIG_READ_STRING(CONFIG_SECTION_PACKET_APP, CONFIG_KEY_PACKET_OUTPUT_PATH, "out/packet_app");
    outputDir = QDir::currentPath() + "/" + outputDirTmp;

    if (!QDir(outputDir).exists())
    {
        QDir().mkpath(outputDir);
    }
}
OtaPacketApp::~OtaPacketApp()
{
}

uint64_t OtaPacketApp::packetGetFileSize(QString& filePath)
{
   QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        return static_cast<uint64_t>(0);
    }
    return static_cast<uint64_t>(fileInfo.size());
}
QString OtaPacketApp::packetGetPartitionName(PARTITION_TYPE_E partitionType)
{
    switch (partitionType)
    {
    case PT_BOOT:
        return QString("boot");
    case PT_TAG:
        return QString("tag");
    case PT_KERNEL:
        return QString("kernrl");
    case PT_ROOTFS:
        return QString("rootfs");
    case PT_RECOVERY:
        return QString("recovery");
    case PT_SYSTEM:
        return QString("system");
    case PT_CONFIG:
        return QString("config");
    case PT_MODEL:
        return QString("model");
    case PT_DATA:
        return QString("data");
    default:
        return QString("unknown");
    }
}
int OtaPacketApp::packetCheckParam(OTA_PACKET_PARAM_S *param)
{
    for (int i = 0; i < 10; ++i)
        if (param->isCheck[i] && !param->packetPath[i].isEmpty())
            return 0;
    return -1;
}
int OtaPacketApp::packetFormatPacketHeader(OTA_PACKET_PARAM_S* param, struct packet_header* header)
{
    int ret;
    header->fw_version[0] = static_cast<uint8_t>(param->major);
    header->fw_version[1] = static_cast<uint8_t>(param->minor);
    header->fw_version[2] = static_cast<uint8_t>(param->patch);
    for(int i = 0; i < 10; ++i)
    {
        if(param->isCheck[i])
        {
            header->entry_count = TO_LE32(header->entry_count + 1);
            ret = packetGetFileSize(param->packetPath[i]);
            if (ret == 0)
            {
                BSD_LOG(LOG_ERRO, QString("file %1 not exist\n").arg(param->packetPath[i]));
                return -1;
            }
            header->total_size += TO_LE32(PARTITION_ENTRY_SIZE);
            header->total_size += TO_LE32(ret);
        }
    }
    header->entry_offset = TO_LE32(PACKET_HEADER_SIZE);
    return 0;
}



int OtaPacketApp::packetProcess(OTA_PACKET_PARAM_S* param)
{
    int ret = 0;
    QString packetName;
    QFile packetFile;
    QFile partitionFile;
    struct packet_header header;
    struct partition_entry entry;
    QByteArray tmpArray(64, '\0');  // 用0填充

    ret = packetCheckParam(param);
    if (ret < 0)
    {
        BSD_LOG(LOG_ERRO, "Please select at least one partition and specify the file path !!!\n");
        return -1;
    }
       
    packetName = QString("OTA-%1-%2-%3.%4.%5.bin").arg(param->projectName).arg(param->customerName).arg(param->major).arg(param->minor).arg(param->patch);
    BSD_LOG_INFO(QString("%1\n").arg(packetName));
    packetName = outputDir + packetName;

    if (QFile::exists(packetName))
    {
        BSD_LOG(LOG_WARN, QString("File already exists, overwriting: %1\n").arg(packetName));
        if (!QFile::remove(packetName))
        {
            BSD_LOG(LOG_ERRO, QString("Failed to remove existing file: %1\n").arg(packetName));
            return -1;
        }
    }

    // 打开打包文件
    packetFile.setFileName(packetName);
    if (!packetFile.open(QIODevice::WriteOnly))
    {
        BSD_LOG(LOG_ERRO, QString("Open file %1 failed !!!\n").arg(packetName));
        return -1;
    }
    // 格式化OTA包头
    ret = packetFormatPacketHeader(param, &header);
    if (ret < 0)
    {
        BSD_LOG(LOG_ERRO, "Format Packet Header failed !!!\n");
        return -1;
    }
    memcpy(tmpArray.data(), &header, sizeof(header));
    packetFile.write(tmpArray);                         // 写入头文件


    // 写入分区头 + 分区数据
    for(int i = 0; i < 10; ++i)
    {
        if(param->isCheck[i])
        {
            partitionFile.setFileName(param->packetPath[i]);
            if (!partitionFile.open(QIODevice::ReadOnly))
            {
                BSD_LOG(LOG_ERRO, QString("Open file %1 failed !!!\n").arg(param->packetPath[i]));
                return -1;
            }
            entry.type = static_cast<PARTITION_TYPE_E>(param->partitionType[i]);
            entry.size = TO_LE32(partitionFile.size());
            entry.offset = TO_LE32(packetFile.pos() + PARTITION_ENTRY_SIZE);
            entry.target_addr = TO_LE32(0);
            tmpArray.fill('\0');
            memcpy(tmpArray.data(), &entry, sizeof(entry));
            packetFile.write(tmpArray);                         // 写入分区头
            packetFile.write(partitionFile.readAll());                         // 写入分区数据
            partitionFile.close();
            BSD_LOG_INFO(QString("Partition [%1] success >> size: %2.\n").arg(packetGetPartitionName(entry.type)).arg(entry.size));
        }
    }
    if (param->aligin > 0 && param->aligin <= 3)
    {
        int align = std::pow(2, param->aligin + 2);
        int padding = align - (packetFile.pos() % align);
        BSD_LOG_INFO(QString("align: %1, padding: %2\n").arg(align).arg(padding));
        if (padding < align)
        {
            tmpArray.fill('\0');
            packetFile.write(tmpArray, padding);
        }
    }
    packetFile.close();
    BSD_LOG_INFO(QString("Packet successfully.\n"));
 
    return 0;
}

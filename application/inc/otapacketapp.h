#ifndef OTAPACKETAPP_H
#define OTAPACKETAPP_H


#include <QString>
#include <QDir>
#include <cmath>
#include "alltoolfun.h"


#define CONFIG_SECTION_PACKET_APP           "PacketApp"
#define CONFIG_KEY_PACKET_OUTPUT_PATH       "OutputPath"


#define PACKET_HEADER_SIZE              (64)
#define PARTITION_ENTRY_SIZE            (64)

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


#pragma pack(push, 1)
typedef enum partition_type {
    PT_BOOT,        
    PT_TAG,         
    PT_KERNEL,      
    PT_ROOTFS,      
    PT_RECOVERY,    
    PT_SYSTEM,      
    PT_CONFIG,      
    PT_MODEL,       
    PT_DATA,            
} PARTITION_TYPE_E; 

struct packet_header {
    uint8_t fw_version[3];          // 固件版本号
    uint32_t total_size;            // 固件包总大小
    uint8_t entry_count;            // 分区条目数量
    uint32_t entry_offset;          // 分区表起始偏移
    uint8_t  reserved[8];           // 保留字段
};

struct partition_entry {
    PARTITION_TYPE_E type;          // 分区名
    uint32_t size;                  // 分区数据实际大小
    uint32_t offset;                // 分区数据在包内的偏移
    uint64_t target_addr;           // flash起始地址
    uint8_t reserved[8];            // 保留字段
};
#pragma pack(pop)

#ifdef __cplusplus
}
#endif

typedef struct 
{
    QString             packetPath[10];             // 固件包路径
    int                 partitionType[10];          // 分区类型
    bool                isCheck[10];               // 是否选中
    int                 aligin;                    // 是否对齐 -1 不对齐
    QString             projectName;
    QString             customerName;
    int                 major;
    int                 minor;
    int                 patch;
}OTA_PACKET_PARAM_S;

class OtaPacketApp 
{
public:
    OtaPacketApp();
    ~OtaPacketApp();
    QString packetGetPartitionName(PARTITION_TYPE_E partitionType);
    uint64_t packetGetFileSize(QString& filePath);
    int packetCheckParam(OTA_PACKET_PARAM_S *param);
    int packetFormatPacketHeader(OTA_PACKET_PARAM_S* param, struct packet_header* header);
    int packetProcess(OTA_PACKET_PARAM_S* param);

private:
    QString outputDir;
};







#endif

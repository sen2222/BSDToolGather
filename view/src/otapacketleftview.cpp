#include "otapacketleftview.h"
#include "ui_otapacketleftview.h"

OtaPacketLeftView::OtaPacketLeftView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OtaPacketLeftView)
{
    ui->setupUi(this);

   initPartitionComponent();
   connectPartitionSignals();
}

OtaPacketLeftView::~OtaPacketLeftView()
{
    delete ui;
}

void OtaPacketLeftView::initPartitionComponent()
{
    // 使用宏简化初始化
    #define INIT_PARTITION(n) \
        m_partitions[n].lineEdit = ui->lineEditPartition##n; \
        m_partitions[n].button = ui->toolButtonPartition##n; \
        m_partitions[n].comboBox = ui->comboBoxPartition##n; \
        m_partitions[n].checkBox = ui->checkBoxPartition##n; \
        m_partitions[n].label = ui->labelPartition##n;
    
    INIT_PARTITION(0)
    INIT_PARTITION(1)
    INIT_PARTITION(2)
    INIT_PARTITION(3)
    INIT_PARTITION(4)
    INIT_PARTITION(5)
    INIT_PARTITION(6)
    INIT_PARTITION(7)
    INIT_PARTITION(8)
    INIT_PARTITION(9)
    
    #undef INIT_PARTITION
}

void OtaPacketLeftView::connectPartitionSignals()
{
    for (int i = 0; i < 10; ++i) {
        connect(m_partitions[i].button, &QToolButton::clicked, this, [=]() {
            QString fileName = QFileDialog::getOpenFileName(
                this,
                QString("选择分区%1文件").arg(i),
                m_partitions[i].lineEdit->text(),
                "所有文件 (*.*)"
            );
            
            if (!fileName.isEmpty()) {
                m_partitions[i].lineEdit->setText(fileName);
            }
        });
    }
}

void OtaPacketLeftView::clearAllLineEdit()
{
    for (int i = 0; i < 10; ++i) {
        m_partitions[i].lineEdit->clear();
    }
}
void OtaPacketLeftView::clearAllComboBox()
{
    for (int i = 0; i < 10; ++i) {
        m_partitions[i].comboBox->setCurrentIndex(0);
    }
}
void OtaPacketLeftView::clearAllCheckBox()
{
    for (int i = 0; i < 10; ++i) {
        m_partitions[i].checkBox->setChecked(false);
    }
}
void OtaPacketLeftView::getAllLineEditText(QString *lineEditText)
{
    for (int i = 0; i < 10; ++i) {
        lineEditText[i] = m_partitions[i].lineEdit->text();
    }
}
void OtaPacketLeftView::getAllCheckBox(bool *isCheck)
{
    for (int i = 0; i < 10; ++i) {
        isCheck[i] = m_partitions[i].checkBox->isChecked();
    }
}
void OtaPacketLeftView::getAllAlign(int *aligin)
{
    for (int i = 0; i < 10; ++i) {
        aligin[i] = m_partitions[i].comboBox->currentIndex();
    }
}
void OtaPacketLeftView::getAllUsetParam(QString *lineEditText, bool *isCheck, int *aligin)
{
    for (int i = 0; i < 10; ++i) {
        lineEditText[i] = m_partitions[i].lineEdit->text();
        isCheck[i] = m_partitions[i].checkBox->isChecked();
        aligin[i] = m_partitions[i].comboBox->currentIndex();
    }
}

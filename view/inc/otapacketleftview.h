#ifndef OTAPACKETLEFTVIEW_H
#define OTAPACKETLEFTVIEW_H

#include <QWidget>
#include <QFileDialog>
#include <QToolButton>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QDir>


namespace Ui {
class OtaPacketLeftView;
}

struct PartitionComponent {
    QLineEdit*      lineEdit;    
    QToolButton*    button;      
    QComboBox*      comboBox;    
    QCheckBox*      checkBox;    
    QLabel*         label;       
};

class OtaPacketLeftView : public QWidget
{
    Q_OBJECT

public:
    explicit OtaPacketLeftView(QWidget *parent = nullptr);
    ~OtaPacketLeftView();
    void clearAllLineEdit();
    void clearAllComboBox();
    void clearAllCheckBox();
    void getAllLineEditText(QString *lineEditText);
    void getAllCheckBox(bool *isCheck);
    void getAllAlign(int *aligin);
    void getAllUsetParam(QString *lineEditText, bool *isCheck, int *aligin);

private:
    void initPartitionComponent();
    void connectPartitionSignals();




private:
    Ui::OtaPacketLeftView *ui;
    PartitionComponent m_partitions[10];
    
};

#endif // OTAPACKETLEFTVIEW_H

#ifndef LoadFileDialog_H
#define LoadFileDialog_H

#include <QDialog>
#include <QStringList>

#include "dataparsermodel.h"
#include "guimodel.h"
//#include "presetparser.h" TODO

namespace Ui {
class LoadFileDialog;
}

class LoadFileDialog : public QDialog
{
    Q_OBJECT

public:

    explicit LoadFileDialog(GuiModel * pGuiModel, DataParserModel * pParserModel, QWidget *parent = nullptr);
    ~LoadFileDialog();

    void open();
    void open(QTextStream *pDataStream, qint32 sampleLineLength);

private slots:

    void updatePath();
    void updateFieldSeparator();
    void updategroupSeparator();
    void updateDecimalSeparator();
    void updateCommentSequence();
    void updateDataRow();
    void updateColumn();
    void updateLabelRow();
    void updateTimeInMilliSeconds();
    void updateStmStudioCorrection();

    void fieldSeparatorSelected(int index);
    void customFieldSeparatorUpdated();
    void groupSeparatorSelected(int  index);
    void decimalSeparatorSelected(int  index);
    void commentUpdated();
    void dataRowUpdated();
    void columnUpdated();
    void labelRowUpdated();
    void timeInMilliSecondsUpdated(bool bTimeInMilliSeconds);
    void stmStudioCorrectionUpdated(bool bCorrectData);

    void presetSelected(int index);

    void done(int r);
    void setPresetToManual();

private:

    typedef struct _ComboListItem
    {
        _ComboListItem(QString _name, QString _userData)
        {
            name = _name;
            userData = _userData;
        }

        QString name;
        QString userData;
    } ComboListItem;

    Ui::LoadFileDialog * _pUi;

    DataParserModel * _pParserModel;
    GuiModel *_pGuiModel;
    // PresetParser _presetParser; TODO

    QStringList _dataFileSample;

    bool validateSettingsData();
    qint32 findIndexInCombo(QList<ComboListItem> comboItemList, QString userDataKey);
    void loadPreset(void);
    void setPresetAccordingKeyword(QString filename);
    void updatePreview();
    void updatePreviewData(QList<QStringList> & previewData);
    void updatePreviewLayout();

    static const QList<ComboListItem> _fieldSeparatorList;
    static const QList<ComboListItem> _decimalSeparatorList;
    static const QList<ComboListItem> _groupSeparatorList;
    static const QColor _cColorLabel;
    static const QColor _cColorData;
    static const QColor _cColorIgnored;
    static const quint32 _cPresetManualIndex = 0;
    static const quint32 _cPresetAutoIndex = 1;
    static const quint32 _cPresetListOffset = 2;
};

#endif // LoadFileDialog_H
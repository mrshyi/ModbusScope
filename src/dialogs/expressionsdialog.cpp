#include "expressionsdialog.h"
#include "ui_expressionsdialog.h"

#include "graphdatamodel.h"
#include "expressionhighlighting.h"

using State = ResultState::State;

ExpressionsDialog::ExpressionsDialog(GraphDataModel *pGraphDataModel, qint32 idx, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::ExpressionsDialog),
    _pGraphDataModel(pGraphDataModel),
    _bUpdating(false)
{
    _pUi->setupUi(this);

    _graphIdx = idx;

    _pHighlighter = new ExpressionHighlighting(_pUi->lineExpression->document());

    connect(&_graphDataHandler, &GraphDataHandler::graphDataReady, this, &ExpressionsDialog::handleDataReady);

    _pUi->tblExpressionInput->setRowCount(0);
    _pUi->tblExpressionInput->setColumnCount(2);
    _pUi->tblExpressionInput->setHorizontalHeaderLabels(QStringList() << "Register" << "Value");
    _pUi->tblExpressionInput->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    _pUi->tblExpressionInput->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    connect(_pUi->tblExpressionInput, &QTableWidget::cellChanged, this, &ExpressionsDialog::handleInputChange);

    _pUi->widgetInfo->setVisible(false);
    connect(_pUi->btnInfo, &QAbstractButton::toggled, _pUi->widgetInfo, &QWidget::setVisible);
    connect(_pUi->btnCancel, &QPushButton::clicked, this, &ExpressionsDialog::handleCancel);
    connect(_pUi->btnAccept, &QPushButton::clicked, this, &ExpressionsDialog::handleAccept);

    connect(_pUi->lineExpression, &QPlainTextEdit::textChanged, this, &ExpressionsDialog::handleExpressionChange);
    _pUi->lineExpression->setPlainText(_pGraphDataModel->expression(_graphIdx));
}

ExpressionsDialog::~ExpressionsDialog()
{
    delete _pUi;
}

void ExpressionsDialog::handleExpressionChange()
{
    /* Avoid endless signal loop, because formatting also emit textChanged */
    if (
            (_localGraphDataModel.size() == 0)
            || (_localGraphDataModel.expression(0) != _pUi->lineExpression->toPlainText()))
    {
        _localGraphDataModel.clear();
        _localGraphDataModel.add();
        _localGraphDataModel.setExpression(0, _pUi->lineExpression->toPlainText());

        _graphDataHandler.processActiveRegisters(&_localGraphDataModel);

        QList<ModbusRegister> registerList;
        _graphDataHandler.modbusRegisterList(registerList);

        /* Save current test values */
        QMap<QString, QString> _testValueMap;
        for(qint32 idx = 0; idx < _pUi->tblExpressionInput->rowCount(); idx++)
        {
            QString descr = _pUi->tblExpressionInput->item(idx, 0)->text();
            QString value = _pUi->tblExpressionInput->item(idx, 1)->text();

            _testValueMap.insert(descr, value);
        }

        _bUpdating = true;
        _pUi->tblExpressionInput->setRowCount(registerList.size());
        _pHighlighter->setExpressionErrorPosition(-1);

        for(qint32 idx = 0; idx < registerList.size(); idx++)
        {
            ModbusRegister reg = registerList[idx];
            QString regDescr = reg.description();
            QTableWidgetItem *newRegisterDescr = new QTableWidgetItem(regDescr);
            newRegisterDescr->setFlags(newRegisterDescr->flags() & ~Qt::ItemIsEditable);
            _pUi->tblExpressionInput->setItem(idx, 0, newRegisterDescr);

            QString testVal = _testValueMap.contains(regDescr) ? _testValueMap[regDescr]: "0";
            QTableWidgetItem *newRegisterValue = new QTableWidgetItem(testVal);
            _pUi->tblExpressionInput->setItem(idx, 1, newRegisterValue);
        }

        _bUpdating = false;

        handleInputChange();
    }
}

void ExpressionsDialog::handleInputChange()
{
    if (!_bUpdating)
    {
        const auto lightRed = QColor(255, 0, 0, 127);
        const auto white = QColorConstants::White;

        ResultDoubleList results;
        for(qint32 idx = 0; idx < _pUi->tblExpressionInput->rowCount(); idx++)
        {
            QTableWidgetItem* pValueItem = _pUi->tblExpressionInput->item(idx, 1);
            QString valueStr = pValueItem->text();
            bool bOk = false;
            double value = valueStr.toDouble(&bOk);
            results.append(ResultDouble(value, bOk ? State::SUCCESS: State::INVALID));

            /* Avoid recursive signal/slots calling */
            _pUi->tblExpressionInput->blockSignals(true);

            pValueItem->setBackground(bOk ? white: lightRed);
            pValueItem->setToolTip(bOk ? QString(): QStringLiteral("Not a valid number"));

            _pUi->tblExpressionInput->blockSignals(false);
        }

        _graphDataHandler.handleRegisterData(results);
    }
}

void ExpressionsDialog::handleCancel()
{
    done(QDialog::Rejected);
}

void ExpressionsDialog::handleAccept()
{
    _pGraphDataModel->setExpression(_graphIdx, _pUi->lineExpression->toPlainText().trimmed());

    done(QDialog::Accepted);
}

void ExpressionsDialog::handleDataReady(ResultDoubleList resultList)
{
    QString numOutput;
    QString strError;

    const bool bOk = !resultList.isEmpty() && resultList.first().isValid();

    if (bOk)
    {
        numOutput = QString("%0").arg(resultList.first().value());
        strError = QString();
    }
    else
    {
        numOutput = QStringLiteral("-");
        strError = _graphDataHandler.expressionParseMsg(0);
    }
    _pHighlighter->setExpressionErrorPosition(_graphDataHandler.expressionErrorPos(0));
    _pHighlighter->rehighlight();

    _pUi->lblOut->setText(numOutput);
    _pUi->lblOut->setToolTip(strError);

    _pUi->lblError->setText(strError);
    QString backgroundStyle;
    if (bOk)
    {
        backgroundStyle = "background-color: rgba(0,0,0,0%);";
    }
    else
    {
        backgroundStyle = "background-color: rgba(255,0,0,50%);";
    }

    _pUi->lblOut->setStyleSheet(QString("border-style: outset;border-width: 1px; border-color: black; %1").arg(backgroundStyle));

    _pUi->lblError->setStyleSheet(backgroundStyle);
}

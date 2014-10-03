#ifndef SCOPEGUI_H
#define SCOPEGUI_H

#include <QObject>

// Foward declaration
class QCustomPlot;

class ScopeGui : public QObject
{
    Q_OBJECT
public:
    explicit ScopeGui(QCustomPlot * pGraph, QObject *parent);

    void resetGraph(quint32 variableCount);

signals:

public slots:
    void plotResults(bool bSuccess, QList<quint16> values);
    void setYAxisAutoScale(int state);
    void setXAxisAutoScale(int state);

private slots:
    void selectionChanged();
    void mousePress();
    void mouseWheel();

private:
    typedef struct
    {
        bool bXAxisAutoScale;
        bool bYAxisAutoScale;
    } GuiSettings;

    QCustomPlot * _pGraph;

    GuiSettings _settings;
};

#endif // SCOPEGUI_H

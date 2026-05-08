#ifndef ADVANCEDGAUGEPLUGIN_H
#define ADVANCEDGAUGEPLUGIN_H

#include <QDesignerCustomWidgetInterface>

class AdvancedGaugePlugin : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetInterface")

public:
    void initialize(QDesignerFormEditorInterface *core) override;
    bool isInitialized() const override;
    QWidget *createWidget(QWidget *parent) override;
    QString name() const override;
    QString group() const override;
    QString toolTip() const override;
    QString whatsThis() const override;
    QString includeFile() const override;
    QIcon icon() const override;
    bool isContainer() const override;

private:
    bool initialized = false;
};

#endif

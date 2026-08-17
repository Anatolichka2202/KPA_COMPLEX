#include "advancedgaugeplugin.h"
#include "advancedgauge.h"
#include <QtDesigner/QDesignerFormEditorInterface>

void AdvancedGaugePlugin::initialize(QDesignerFormEditorInterface * /*core*/)
{
    initialized = true;
}

bool AdvancedGaugePlugin::isInitialized() const
{
    return initialized;
}

QWidget *AdvancedGaugePlugin::createWidget(QWidget *parent)
{
    return new AdvancedGauge(parent);
}

QString AdvancedGaugePlugin::name() const
{
    return QStringLiteral("AdvancedGauge");
}

QString AdvancedGaugePlugin::group() const
{
    return QStringLiteral("Custom Widgets");
}

QString AdvancedGaugePlugin::toolTip() const
{
    return QStringLiteral("Спидометр с установкой угла (setpoint) и шаговым слайдером");
}

QString AdvancedGaugePlugin::whatsThis() const
{
    return toolTip();
}

QString AdvancedGaugePlugin::includeFile() const
{
    return QStringLiteral("advancedgauge.h");
}

QIcon AdvancedGaugePlugin::icon() const
{
    // Можно вернуть QIcon(":/icon.png"), если добавите ресурс
    return QIcon();
}

bool AdvancedGaugePlugin::isContainer() const
{
    return false;
}

#include "InspectorAtemAutoWidget.h"

#include "Global.h"

#include "AtemDeviceManager.h"
#include "DatabaseManager.h"
#include "EventManager.h"
#include "Models/Atem/AtemStepModel.h"
#include "Models/Atem/AtemMixerStepModel.h"
#include "Models/Atem/AtemAutoSpeedModel.h"
#include "Models/Atem/AtemAutoTransitionModel.h"

#include <QtWidgets/QApplication>

InspectorAtemAutoWidget::InspectorAtemAutoWidget(QWidget* parent)
    : QWidget(parent),
      model(NULL), command(NULL)
{
    setupUi(this);

    QObject::connect(&EventManager::getInstance(), SIGNAL(rundownItemSelected(const RundownItemSelectedEvent&)), this, SLOT(rundownItemSelected(const RundownItemSelectedEvent&)));
    QObject::connect(&EventManager::getInstance(), SIGNAL(atemDeviceChanged(const AtemDeviceChangedEvent&)), this, SLOT(atemDeviceChanged(const AtemDeviceChangedEvent&)));

    loadAtemStep();
    loadAtemAutoTransition();
}

void InspectorAtemAutoWidget::rundownItemSelected(const RundownItemSelectedEvent& event)
{
    this->command = nullptr;
    this->model = event.getLibraryModel();

    blockAllSignals(true);

    if (dynamic_cast<AtemAutoCommand*>(event.getCommand()))
    {
        this->command = dynamic_cast<AtemAutoCommand*>(event.getCommand());

        this->comboBoxMixerStep->clear();
        const QSharedPointer<AtemDevice> device = AtemDeviceManager::getInstance().getDeviceByName(this->model->getDeviceName());
        if (device != NULL)
        {
            this->mixerEffects = device->mixerEffects();

            loadAtemMixerStep();
        }

        this->comboBoxMixerStep->setCurrentIndex(this->comboBoxMixerStep->findData(this->command->getMixerStep()));
        this->comboBoxStep->setCurrentIndex(this->comboBoxStep->findData(this->command->getStep()));
        this->spinBoxSpeed->setValue(this->command->getSpeed());
        this->checkBoxTriggerOnNext->setChecked(this->command->getTriggerOnNext());
        this->comboBoxTransition->setCurrentIndex(this->comboBoxTransition->findData(this->command->getTransition()));
    }

    checkEmptyStep();
    checkEmptyMixerStep();

    blockAllSignals(false);
}

void InspectorAtemAutoWidget::atemDeviceChanged(const AtemDeviceChangedEvent& event)
{
    if (this->model != NULL)
    {
        // Should we update the device name?
        if (!event.getDeviceName().isEmpty() && event.getDeviceName() != this->model->getDeviceName())
        {
            const QSharedPointer<AtemDevice> device = AtemDeviceManager::getInstance().getDeviceByName(event.getDeviceName());
            if (device != NULL)
            {
                this->mixerEffects = device->mixerEffects();

                loadAtemMixerStep();
                checkEmptyMixerStep();
            }
        }
    }
}

void InspectorAtemAutoWidget::blockAllSignals(bool block)
{
    this->comboBoxMixerStep->blockSignals(block);
    this->comboBoxStep->blockSignals(block);
    this->spinBoxSpeed->blockSignals(block);
    this->checkBoxTriggerOnNext->blockSignals(block);
    this->comboBoxTransition->blockSignals(block);
}

void InspectorAtemAutoWidget::loadAtemMixerStep()
{
    // We do not have a command object, block the signals.
    // Events will not be triggered while we update the values.
    this->comboBoxMixerStep->blockSignals(true);

    this->comboBoxMixerStep->clear();
    for (int i = 0; i < this->mixerEffects; i++)
        this->comboBoxMixerStep->addItem(QString("%1").arg(i + 1), i);

    this->comboBoxMixerStep->blockSignals(false);
}

void InspectorAtemAutoWidget::loadAtemStep()
{
    // We do not have a command object, block the signals.
    // Events will not be triggered while we update the values.
    this->comboBoxStep->blockSignals(true);

    QList<AtemStepModel> models = DatabaseManager::getInstance().getAtemStep();
    foreach (AtemStepModel model, models)
        this->comboBoxStep->addItem(model.getName(), model.getValue());

    this->comboBoxStep->blockSignals(false);
}

void InspectorAtemAutoWidget::loadAtemAutoTransition()
{
    // We do not have a command object, block the signals.
    // Events will not be triggered while we update the values.
    this->comboBoxTransition->blockSignals(true);

    QList<AtemAutoTransitionModel> models = DatabaseManager::getInstance().getAtemAutoTransition();
    foreach (AtemAutoTransitionModel model, models)
        this->comboBoxTransition->addItem(model.getName(), model.getValue());

    this->comboBoxTransition->blockSignals(false);
}

void InspectorAtemAutoWidget::checkEmptyStep()
{
    if (this->comboBoxStep->isEnabled() && this->comboBoxStep->currentText() == "")
        this->comboBoxStep->setStyleSheet("border-color: firebrick;");
    else
        this->comboBoxStep->setStyleSheet("");
}

void InspectorAtemAutoWidget::checkEmptyMixerStep()
{
    if (this->comboBoxMixerStep->isEnabled() && this->comboBoxMixerStep->currentText() == "")
        this->comboBoxMixerStep->setStyleSheet("border-color: firebrick;");
    else
        this->comboBoxMixerStep->setStyleSheet("");
}

void InspectorAtemAutoWidget::stepChanged(int index)
{
    this->command->setStep(this->comboBoxStep->itemData(index).toString());

    checkEmptyStep();
}

void InspectorAtemAutoWidget::speedChanged(int value)
{
    Q_UNUSED(value);

    this->command->setSpeed(this->spinBoxSpeed->value());
}

void InspectorAtemAutoWidget::transitionChanged(int index)
{
    this->command->setTransition(this->comboBoxTransition->itemData(index).toString());
}

void InspectorAtemAutoWidget::triggerOnNextChanged(int state)
{
    this->command->setTriggerOnNext((state == Qt::Checked) ? true : false);
}

void InspectorAtemAutoWidget::mixerStepChanged(int index)
{
    this->command->setMixerStep(this->comboBoxMixerStep->itemData(index).toString());

    checkEmptyMixerStep();
}

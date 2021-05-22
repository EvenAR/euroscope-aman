#include "stdafx.h"

#include "AmanController.h"
#include "AmanPlugIn.h"
#include "AmanWindow.h"

AmanController::AmanController(AmanPlugIn* plugin) {
    this->amanModel = plugin;
    this->amanWindow = std::make_shared<AmanWindow>(this);
}

AmanController::~AmanController() {}

void AmanController::modelUpdated() {
    if (this->amanWindow != nullptr) {
        auto timelines = this->amanModel->getTimelines(activeTimelines);
        auto loadedDefinitions = this->amanModel->getAvailableIds();
        this->amanWindow->update(timelines);
        this->amanWindow->setAvailableTimelines(loadedDefinitions);
    }
}

void AmanController::toggleTimeline(const std::string& id) {
    bool isActive = std::find(activeTimelines.begin(), activeTimelines.end(), id) != activeTimelines.end();

    if (isActive) {
        activeTimelines.erase(
            std::remove(activeTimelines.begin(), activeTimelines.end(), id),
            activeTimelines.end()
        );
    } else {
        activeTimelines.push_back(id);
    }
    modelUpdated();
}

void AmanController::reloadProfiles() {
    this->amanModel->requestReload();
}

void AmanController::setTimelineHorizon(const std::string& id, uint32_t minutes) {
    this->amanWindow->setTimelineHorizon(id, minutes);
}

bool AmanController::openWindow() {
    if (this->amanWindow == nullptr) {
        this->amanWindow = std::make_shared<AmanWindow>(this);
        return true;
    }
}

bool AmanController::closeWindow() {
    if (this->amanWindow == nullptr) {
        return false;
    } else {
        this->amanWindow.reset();
        return true;
    }    
}


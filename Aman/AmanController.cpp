#include "stdafx.h"

#include "AmanController.h"
#include "AmanPlugIn.h"
#include "AmanWindow.h"

AmanController::AmanController(AmanPlugIn* plugin) {
    this->amanModel = plugin;
    this->amanWindow = NULL;
}

AmanController::~AmanController() {}

void AmanController::modelLoaded() {
    if (this->amanWindow == nullptr) {
        this->amanWindow = std::make_shared<AmanWindow>(this, amanModel->getAvailableIds());
    }
}

void AmanController::modelUpdated() {
    if (this->amanWindow != nullptr) {
        auto timelines = this->amanModel->getTimelines(activeTimelines);
        this->amanWindow->update(timelines);
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

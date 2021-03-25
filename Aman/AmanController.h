#pragma once

#include <vector>
#include <string>
#include <memory>

class AmanWindow;
class AmanPlugIn;

class AmanController {
public:
    AmanController(AmanPlugIn* model);
    void modelUpdated();
    void toggleTimeline(const std::string& id);
    void reloadProfiles();
    void setTimelineHorizon(const std::string& id, uint32_t minutes);

    ~AmanController();

private:
    AmanPlugIn* amanModel;
    std::shared_ptr<AmanWindow> amanWindow;

    std::vector<std::string> activeTimelines;
};

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
    bool isTimelineActive(const std::string& id);
    void reloadProfiles();
    bool openWindow();
    bool closeWindow();

    ~AmanController();

private:
    AmanPlugIn* amanModel;
    std::shared_ptr<AmanWindow> amanWindow;

    std::vector<std::string> activeTimelines;
};

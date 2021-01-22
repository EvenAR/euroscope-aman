#pragma once

#include <vector>
#include <string>
#include <memory>

class AmanWindow;
class AmanPlugIn;

class AmanController {
public:
    AmanController(AmanPlugIn* model);
    void modelLoaded();
    void modelUpdated();
    void toggleTimeline(const std::string& id);
    void reloadProfiles();

    ~AmanController();

private:
    AmanPlugIn* amanModel;
    std::shared_ptr<AmanWindow> amanWindow;

    std::vector<std::string> activeTimelines;
};

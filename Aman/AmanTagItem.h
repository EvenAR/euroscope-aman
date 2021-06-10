#pragma once
#include <string>

class TagItem {
public:
    TagItem(const std::string& source, const std::string& defaultValue, uint32_t width, bool rightAligned, bool isViaFixIndicator) {
        this->source = source;
        this->width = width;
        this->rightAligned = rightAligned;
        this->defaultValue = defaultValue;
        this->isViaFixIndicator = isViaFixIndicator;
    }
    std::string getSource() { return source; };
    std::string getDefaultValue() { return defaultValue; };
    uint32_t getWidth() { return width; };
    bool isRightAligned() { return rightAligned; };
    bool getIsViaFixIndicator() { return isViaFixIndicator; };
private:
    std::string source;
    std::string defaultValue;
    bool isViaFixIndicator;
    uint32_t width;
    bool rightAligned;
};
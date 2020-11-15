#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

typedef std::function<void()> Callback;

class EventEmitter {
public:
    void on(std::string eventName, Callback&& callback);

protected:
    void emit(std::string eventName);

private:
    std::unordered_map<std::string, std::vector<Callback>> callbacks;
};

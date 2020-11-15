#include "stdafx.h"

#include "EventEmitter.h"

void EventEmitter::on(std::string eventName, Callback &&callback) {
    if (callbacks.find(eventName) == callbacks.end()) {
        callbacks[eventName] = std::vector<Callback>();
    }

    callbacks[eventName].push_back(callback);
}

void EventEmitter::emit(std::string eventName) {
    if (callbacks.find(eventName) == callbacks.end()) {
        // No callbacks registered for this event name
        return;
    }
    for (auto &callback : callbacks[eventName]) {
        callback();
    }
}

#ifndef EVENTBUS_H
#define EVENTBUS_H

#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <memory>
#include "Event.h"


class EventBus {
public:
    using HandlerFunc = std::function<void(const EventBase&)>;

    template<typename EventT>
    void subscribe(std::function<void(const EventT&)> handler) {
        auto typeIdx = std::type_index(typeid(EventT));
        HandlerFunc wrapper = [handler](const EventBase& base) {
            handler(static_cast<const EventT&>(base));
        };
        handlers[typeIdx].push_back(wrapper);
    }

    // Публикация события (распространяет всем подписчикам)
    template<typename EventT>
    void publish(const EventT& event) {
        auto typeIdx = std::type_index(typeid(EventT));
        auto it = handlers.find(typeIdx);
        if (it != handlers.end()) {
            for (auto& handler : it->second) {
                handler(event);
            }
        }
    }
private:
    // тип события → массив подписчиков
    std::unordered_map<std::type_index, std::vector<HandlerFunc>> handlers;
};



#endif // EVENTBUS_H

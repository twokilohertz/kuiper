#pragma once

#include <any>
#include <cstdint>
#include <functional>
#include <queue>
#include <typeindex>
#include <unordered_map>

namespace kuiper {

class event_bus {
  private:
    event_bus() = default;

  public:
    ~event_bus() = default;

  public:
    static inline event_bus& get() {
        static event_bus s_instance;
        return s_instance;
    }

  public:
    using subscription_id = std::uint64_t;
    using handler_fn      = std::function<void(const std::any&)>;

  public:
    template<typename EventType, typename Callback>
    subscription_id subscribe(Callback&& callback) {
        using DecayedType = std::decay_t<EventType>;

        auto wrapper_fn = [cb = std::forward<Callback>(callback)](const std::any& generic_event) {
            cb(std::any_cast<DecayedType>(generic_event));
        };

        subscription_id id                                  = ++next_id;
        listeners[std::type_index(typeid(DecayedType))][id] = std::move(wrapper_fn);

        return id;
    }

    template<typename EventType>
    void unsubscribe(subscription_id id) {
        using DecayedType = std::decay_t<EventType>;
        auto type_idx     = std::type_index(typeid(DecayedType));

        if (auto it = listeners.find(type_idx); it != listeners.end()) {

            it->second.erase(id);

            if (it->second.empty()) {
                listeners.erase(it);
            }
        }
    }

    template<typename EventType>
    void publish(EventType&& event) {
        using DecayedType = std::decay_t<EventType>;
        event_queue.push(std::make_any<DecayedType>(std::forward<EventType>(event)));
    }

    void poll() {
        std::queue<std::any> current_queue;
        std::swap(current_queue, event_queue);

        while (!current_queue.empty()) {
            const auto& event    = current_queue.front();
            auto        type_idx = std::type_index(event.type());

            if (auto it = listeners.find(type_idx); it != listeners.end()) {
                for (const auto& [id, handler] : it->second) {
                    handler(event);
                }
            }

            current_queue.pop();
        }
    }

  private:
    std::unordered_map<std::type_index, std::unordered_map<subscription_id, handler_fn>> listeners {};
    std::queue<std::any>                                                                 event_queue {};
    subscription_id                                                                      next_id {0};
};

} // namespace kuiper

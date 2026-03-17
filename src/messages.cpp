#include "messages.h"
#include <cstdlib>
#include "esp_timer.h"

Message getRandomMessage(const Customer& customer)
{
    if (customer.messages.empty()) {
        return {"Inget meddelande", "text", ""};
    }

    // Räkna ut minuter sedan start
    int64_t microseconds = esp_timer_get_time();
    int minutes = (int)(microseconds / 1000000 / 60);

    // Filtrera bort meddelanden som inte passar aktuell minut
    std::vector<Message> available;
    for (const Message& msg : customer.messages) {
        if (msg.rule == "") {
            available.push_back(msg);
        } else if (msg.rule == "even" && minutes % 2 == 0) {
            available.push_back(msg);
        } else if (msg.rule == "odd" && minutes % 2 != 0) {
            available.push_back(msg);
        }
    }

    if (available.empty()) {
        return customer.messages[0];  // fallback om ingen passar
    }

    int index = rand() % available.size();
    return available[index];
}
#include <stdio.h>
#include <vector>
#include <cstdlib>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "customers.h"
#include "messages.h"
#include "display.h"
#include "sortCustomers.h"

extern "C" void app_main(void)
{
    srand(42);

    displayInit();

    std::vector<Customer> customers = loadCustomers();
    int lastCustomerIndex = -1;

    while (true)
    {
        int currentIndex = chooseNextCustomerIndex(customers, lastCustomerIndex);

        if (currentIndex == -1)
        {
            printf("Kunde inte välja kund.\n");
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        Customer currentCustomer = customers[currentIndex];
        Message msg = getRandomMessage(currentCustomer);

        printf("\n--- %s ---\n", currentCustomer.name.c_str());
        displayMessage(msg);

        lastCustomerIndex = currentIndex;

        vTaskDelay(pdMS_TO_TICKS(20000));
    }
}
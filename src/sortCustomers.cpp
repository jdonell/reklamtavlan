#include "sortCustomers.h"
#include <algorithm>
#include <cstdlib>

std::vector<Customer> sortCustomers(const std::vector<Customer>& customers)
{
    std::vector<Customer> sorted = customers;

    std::sort(sorted.begin(), sorted.end(),
        [](const Customer& a, const Customer& b) {
            return a.payment > b.payment;
        }
    );

    return sorted;
}

int chooseNextCustomerIndex(const std::vector<Customer>& customers, int lastIndex)
{
    if (customers.empty()) {
        return -1;
    }

    int totalWeight = 0;

    for (int i = 0; i < static_cast<int>(customers.size()); i++) {
        if (i != lastIndex) {
            totalWeight += customers[i].payment;
        }
    }

    if (totalWeight <= 0) {
        return -1;
    }

    int randomValue = rand() % totalWeight;
    int currentSum = 0;

    for (int i = 0; i < static_cast<int>(customers.size()); i++) {
        if (i == lastIndex) {
            continue;
        }

        currentSum += customers[i].payment;

        if (randomValue < currentSum) {
            return i;
        }
    }

    return -1;
}
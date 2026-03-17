#pragma once
#include <vector>
#include "customers.h"

std::vector<Customer> sortCustomers(const std::vector<Customer>& customers);
int chooseNextCustomerIndex(const std::vector<Customer>& customers, int lastIndex);
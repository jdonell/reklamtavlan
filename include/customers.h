#pragma once
#include <string>
#include <vector>

//representerar ett reklammedelande
struct Message{
    std::string text;  //själva reklamen
    std::string type;   // text / scroll / blink
    std::string rule;   // even / odd / ""
}; 

//representerar en kund

struct Customer{
    std::string name; 
    int payment; 
    std::vector <Message> messages; 
}; 

//laddar alla kunder och deras medelande
std::vector <Customer> loadCustomers(); 
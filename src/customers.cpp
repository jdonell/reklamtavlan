#include "customers.h"

std::vector<Customer> loadCustomers() {
    std::vector<Customer> customers;

    Customer harry;
    harry.name = "Hederlige Harrys Bilar";
    harry.payment = 5000;
    harry.messages.push_back({"Kop bil hos Harry", "scroll", ""});
    harry.messages.push_back({"En god bilaffar (for Harry!)", "text", ""});
    harry.messages.push_back({"Hederlige Harrys Bilar", "blink", ""});
    customers.push_back(harry);

    Customer langben;
    langben.name = "Langbens detektivbyra";
    langben.payment = 4000;
    langben.messages.push_back({"Mysterier? Ring Langben", "text", ""});
    langben.messages.push_back({"Langben fixar biffen", "text", ""});
    customers.push_back(langben);

    Customer anka;
    anka.name = "Farmor Ankas Pajer AB";
    anka.payment = 3000;
    anka.messages.push_back({"Kop paj hos Farmor Anka", "scroll", ""});
    anka.messages.push_back({"Skynda innan Marten atit alla pajer", "text", ""});
    customers.push_back(anka);

    Customer petter;
    petter.name = "Svarte Petters Svartbyggen";
    petter.payment = 1500;
    petter.messages.push_back({"Lat Petter bygga at dig", "scroll", "even"});
    petter.messages.push_back({"Bygga svart? Ring Petter", "text", "odd"});
    customers.push_back(petter);

    Customer iot;
    iot.name = "IOT:s Reklambyrå";
    iot.payment = 1000;
    iot.messages.push_back({"Synas har? IOT:s Reklambyr a", "text", ""});
    customers.push_back(iot);

    return customers;
}
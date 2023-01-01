#include <inery/inery.hpp>
#include <inery/time.hpp>
#include <inery/system.hpp>
#include <inery/print.hpp>

#include <string.h>
#include <vector>

using namespace inery;
using namespace std;

using inery::current_time_point;
using inery::indexed_by;
using inery::time_point_sec;

class [[inery::contract("document")]] document : public contract {
    private:
  
    struct warehouse {
         uint64_t warehouse_id;
         std::string warehouse_name;
         std::string location;
         uint64_t capacity;
         uint64_t load;
      };

      struct product {
         uint64_t product_id;
         std::string product_name;
         std::string product_description;
         std::string unit_of_measurement;

      };

      struct inventory {
         uint64_t inventory_id;
         uint64_t warehouse_id;
         uint64_t product_id;
         uint64_t quantity;
         inery::name owner;
      };

      struct  order {
         uint64_t order_id;
         uint64_t warehouse_id;
         uint64_t product_id;
         uint64_t quantity;
         time_point order_date;
         time_point expected_delivery_date;
      };

      struct delivery {
         uint64_t delivery_id;
         uint64_t order_id;
         time_point delivery_date;
         uint64_t quantity;
      };

     struct [[inery::table]] warehouses {
         uint64_t id;
         warehouse warehouse;
         vector<product> products;
         vector<inventory> inventory;
         vector<order> orders;
         vector<delivery> delivery;
         checksum256 tx_hash;
         uint64_t primary_key() const { return id; }
      };

      typedef multi_index<"warehouses"_n, warehouses> warehouses_t;

      warehouses_t _warehouses;   

   public:

      using contract::contract;

    document(name receiver, name code, datastream<const char*> ds)
    : contract(receiver, code, ds)
    , _warehouses(get_self(), get_self().value){}

      // Add a new warehouse
      [[inery::action]]
      void addwarehouse(uint64_t warehouse_id, const std::string& warehouse_name, const std::string& location, uint64_t capacity)
      {
         require_auth(get_self());
         check(capacity >= 0, "Invalid capacity");
         auto warehouse_itr = _warehouses.find(warehouse_id);
         check(warehouse_itr == _warehouses.end(), "Warehouse already exists");

         warehouse warehouse_i = {warehouse_id, warehouse_name, location, capacity, capacity};

         _warehouses.emplace(get_self(), [&](auto& w) {
               w.id = warehouse_id;
               w.warehouse = warehouse_i;
         });
      };
      [[inery::action]]
      void addproduct(uint64_t warehouse_id, uint64_t product_id, const std::string& product_name, const std::string& product_description, const std::string& unit_of_measurement) 
      {
         require_auth(get_self());
         check(product_id > 0, "Invalid product ID");
         check(product_name.size() > 0, "Invalid product name");
         check(unit_of_measurement.size() > 0, "Invalid unit of measurement");

         auto warehouse_itr = _warehouses.find(warehouse_id);
         check(warehouse_itr == _warehouses.end(), "Warehouse doesn't exists");


         for (const auto& prod : warehouse_itr->products) {
            check(prod.product_id == product_id, "Product with that Id already exists");
         }

         product product= {product_id, product_name, product_description, unit_of_measurement};

         _warehouses.modify(warehouse_itr, same_payer, [&](auto& w) {
            w.products.push_back(product);
         });

      };
      // Add a new entry to the inventory table
      [[inery::action]]
      void addinventory(uint64_t inventory_id, inery::name owner, uint64_t warehouse_id, uint64_t product_id, uint64_t quantity)
      {
         require_auth(owner);
         auto warehouse_itr = _warehouses.find(warehouse_id);
         check(warehouse_itr != _warehouses.end(), "Warehouse with given ID doesn't exist");

         // Check if product with given ID exists in warehouse's products vector
         auto product_itr = std::find_if(warehouse_itr->products.cbegin(), warehouse_itr->products.cend(),
                                       [&](const product& p) { return p.product_id == product_id; });
         check(product_itr != warehouse_itr->products.cend(), "Product with given ID doesn't exist in warehouse");

         auto inventory_itr = std::find_if(warehouse_itr->inventory.cbegin(), warehouse_itr->inventory.cend(),
                                          [&](const inventory& i) { return i.inventory_id == inventory_id; });
         check(inventory_itr == warehouse_itr->inventory.cend(), "Inventory with given ID already exists");

         check(warehouse_itr->warehouse.load >= quantity, "There is not enought space for that quantity");

         inventory inventory{inventory_id, warehouse_id, product_id, quantity, owner};

         _warehouses.modify(warehouse_itr, same_payer, [&](auto& w){
            w.inventory.push_back(inventory);
            w.warehouse.load -= quantity;
         });

      };
      // Add a new order
      [[inery::action]]
      void placeorder(uint64_t order_id, uint64_t inventory_id, uint64_t quantity, int expected_delivery_date_days)
      {
         require_auth(get_self());
      
      };
      // Update the delivery information for an order
      [[inery::action]]
      void updelivery(uint64_t delivery_id, uint64_t order_id, uint64_t quantity)
      {
         require_auth(get_self());
      };
};      

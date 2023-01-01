#include <inery/inery.hpp>
#include <inery/time.hpp>
#include <inery/system.hpp>
#include <inery/print.hpp>

#include <string.h>

using namespace inery;
using namespace std;

using inery::current_time_point;
using inery::indexed_by;
using inery::time_point_sec;

class [[inery::contract("database")]] database : public contract {
   public:
      using contract::contract;

      database(name receiver, name code, datastream<const char*> ds);

     struct [[inery::table]] warehouse {
         uint64_t warehouse_id;
         std::string warehouse_name;
         std::string location;
         uint64_t capacity;
         uint64_t load;

         uint64_t primary_key() const { return warehouse_id; }
      };
      typedef multi_index<"warehouses"_n, warehouse> warehouses_t;

      struct [[inery::table]] product {
         uint64_t product_id;
         std::string product_name;
         std::string product_description;
         std::string unit_of_measurement;

         uint64_t primary_key() const { return product_id; }
      };
      typedef multi_index<"products"_n, product> product_t;

      struct [[inery::table]] inventory {
         uint64_t inventory_id;
         uint64_t warehouse_id;
         uint64_t product_id;
         uint64_t quantity;
         inery::name owner;

         uint64_t primary_key() const { return inventory_id; }
         uint64_t warehouse_index() const { return warehouse_id; }
         uint64_t product_index() const { return product_id; }
      };
      typedef multi_index<"inventory"_n, inventory,
         indexed_by<"bywarehouse"_n, const_mem_fun<inventory, uint64_t, &inventory::warehouse_index>>,
         indexed_by<"byproduct"_n, const_mem_fun<inventory, uint64_t, &inventory::product_index>>
      > inventory_t;

      struct [[inery::table]] order {
         uint64_t order_id;
         uint64_t warehouse_id;
         uint64_t product_id;
         uint64_t quantity;
         time_point order_date;
         time_point expected_delivery_date;

         uint64_t primary_key() const { return order_id; }
         uint64_t warehouse_index() const { return warehouse_id; }
         uint64_t product_index() const { return product_id; }
      };
      typedef multi_index<"orders"_n, order,
         indexed_by<"bywarehouse"_n, const_mem_fun<order, uint64_t, &order::warehouse_index>>,
         indexed_by<"byproduct"_n, const_mem_fun<order, uint64_t, &order::product_index>>
      > order_t;

      struct [[inery::table]] delivery {
         uint64_t delivery_id;
         uint64_t order_id;
         time_point delivery_date;
         uint64_t quantity;

         uint64_t primary_key() const { return delivery_id; }
         uint64_t order_index() const { return order_id; }
      };

      typedef multi_index<"deliveries"_n, delivery,
        indexed_by<"byorderid"_n, const_mem_fun<delivery, uint64_t, &delivery::order_index >>
      > delivery_t;

      // Add a new warehouse
      [[inery::action]]
      void addwarehouse(uint64_t warehouse_id, const std::string& warehouse_name, const std::string& location, uint64_t capacity);

      // Add a new product
      [[inery::action]]
      void addproduct(uint64_t product_id, const std::string& product_name, const std::string& product_description, const std::string& unit_of_measurement);

      // Add a new entry to the inventory table
      [[inery::action]]
      void addinventory(uint64_t inventory_id, inery::name owner, uint64_t warehouse_id, uint64_t product_id, uint64_t quantity);

      // Add a new order
      [[inery::action]]
      void placeorder(uint64_t order_id, uint64_t inventory_id, uint64_t quantity, int expected_delivery_date_days);

      // Update the delivery information for an order
      [[inery::action]]
      void updelivery(uint64_t delivery_id, uint64_t order_id, uint64_t quantity);


    private:
      warehouses_t _warehouses;
      inventory_t  _inventory; 
      order_t      _orders;
      delivery_t   _deliveries;
      product_t    _products;
      
};
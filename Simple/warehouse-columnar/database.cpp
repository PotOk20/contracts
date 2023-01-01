#include "database.hpp"

using namespace inery;
using namespace std;

database::database(name receiver, name code, datastream<const char*> ds)
    : contract(receiver, code, ds)
    , _warehouses(get_self(), get_self().value) // instantiate the warehouses table
    , _inventory(get_self(), get_self().value) 
    , _orders(get_self(), get_self().value) 
    , _products(get_self(), get_self().value)
    , _deliveries(get_self(), get_self().value){}

void database::addwarehouse(uint64_t warehouse_id, const std::string& warehouse_name, const std::string& location, uint64_t capacity) {
   // Check if the warehouse already exists
   require_auth(get_self());

   auto warehouse_itr = _warehouses.find(warehouse_id);
   check(warehouse_itr == _warehouses.end(), "Warehouse already exists");

   // Add the warehouse to the _
   _warehouses.emplace(get_self(), [&](auto& w) {
      w.warehouse_id = warehouse_id;
      w.warehouse_name = warehouse_name;
      w.location = location;
      w.capacity = capacity;
      w.load     = capacity;
   });
}

void database::addproduct(uint64_t product_id, const std::string& product_name, const std::string& product_description, const std::string& unit_of_measurement) {
   require_auth(get_self());
   // Check if the product already exists
   auto product_itr = _products.find(product_id);
   check(product_itr == _products.end(), "Product already exists");

   // Add the product to the _products table
   _products.emplace(get_self(), [&](auto& p) {
      p.product_id = product_id;
      p.product_name = product_name;
      p.product_description = product_description;
      p.unit_of_measurement = unit_of_measurement;
   });
   print("Created product ", product_name, " with product id : ", product_id);

}

void database::addinventory(uint64_t inventory_id, inery::name owner, uint64_t warehouse_id, uint64_t product_id, uint64_t quantity) {
   require_auth(get_self());
   // Check if the warehouse exists
   auto warehouse_itr = _warehouses.find(warehouse_id);
   check(warehouse_itr != _warehouses.end(), "Warehouse does not exist");

   check(warehouse_itr->load - quantity >= 0, "Warehouse does not have space for that quantity!");
   // update load left in warehouse
   _warehouses.modify(warehouse_itr, same_payer, [&](auto& i) {
      i.load -= quantity;
   });

   // Check if the product exists
   auto product_itr = _products.find(product_id);
   check(product_itr != _products.end(), "Product does not exist");

   // Add the inventory entry to the inventory table
   _inventory.emplace(get_self(), [&](auto& i) {
      i.inventory_id = inventory_id;
      i.warehouse_id = warehouse_id;
      i.product_id = product_id;
      i.quantity = quantity;
      i.owner = owner;
   });
   print(owner, " Inventory ", inventory_id, " in warehouse ", warehouse_id, " with ", quantity ," of product ", product_id );
}

void database::placeorder(uint64_t order_id, uint64_t inventory_id, uint64_t quantity, int expected_delivery_date_days) {
   require_auth(get_self());

   auto inventory_itr = _inventory.find(inventory_id);
   check(inventory_itr != _inventory.end(), "Inventory does not exist");

   auto warehouse_id = inventory_itr->warehouse_id;
   auto product_id = inventory_itr->product_id;

   check(inventory_itr != _inventory.end() && inventory_itr->quantity >= quantity, "Warehouse does not have enough inventory of the product");

   // Decrease the inventory of the product in the warehouse
   _inventory.modify(inventory_itr, same_payer, [&](auto& i) {
      i.quantity -= quantity;
   });

   auto ct = current_time_point();

   time_point delivery_date = current_time_point(); 
   delivery_date +=inery::microseconds(expected_delivery_date_days * 24 * 60 * 60 * 1000000); 

   // Add the order to the orders table
   _orders.emplace(get_self(), [&](auto& o) {
      o.order_id = order_id;
      o.warehouse_id = warehouse_id;
      o.product_id = product_id;
      o.quantity = quantity;
      o.order_date = ct;
      o.expected_delivery_date = delivery_date;
   });
   print("Successfully places order with id: ", order_id, " . Expected delivery ",expected_delivery_date_days ,"days from now");
}
/*First checks if the warehouse and product specified in the order exist.
/ It then checks if the warehouse has enough inventory of the product to fulfill the order.
/ If these checks pass, it decreases the inventory of the product in the warehouse and adds the order to the orders table.
*/ 

void database::updelivery(uint64_t delivery_id, uint64_t order_id, uint64_t quantity) {
   require_auth(get_self());

   auto order_itr = _orders.find(order_id);
   check(order_itr != _orders.end(), "Order does not exist");

   // Check if the delivery already exists
   auto delivery_itr = _deliveries.find(delivery_id);
   check(delivery_itr == _deliveries.end(), "Delivery already exists");

   auto delivery_date = order_itr->expected_delivery_date;

   // Add the delivery information to the deliveries table
   _deliveries.emplace(get_self(), [&](auto& d) {
      d.delivery_id = delivery_id;
      d.order_id = order_id;
      d.delivery_date = delivery_date;
      d.quantity = quantity;
   });
   print("Set delivery ", delivery_id, " status for order : ", order_id );
}
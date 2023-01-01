#include <inery/inery.hpp>
#include <inery/time.hpp>
#include <inery/system.hpp>
#include <inery/print.hpp>
#include<inery/asset.hpp>

#include <string.h>

using namespace inery;
using namespace std;

using inery::current_time_point;
using inery::indexed_by;
using inery::time_point_sec;

class [[inery::contract("library")]] library : public contract {
   public:
      using contract::contract;

      library(name receiver, name code, datastream<const char*> ds);

    struct book {
        uint64_t book_id;
        string title;
        string author;
        string isbn;
        string location;
        asset borrow_price;
        int    quantity;
        vector<string> tags;
        vector<string> categories;
    };

    struct Patron {
        string name;
        std::vector<std::pair<book, time_point>> borrowedItems;
        double overdueFines;
    };

    struct [[inery::table]] libraries {
        uint64_t  id;
        string    shelf_name;
        std::vector<book> items;
        std::vector<Patron> patrons;

    uint64_t primary_key() const { return id; }
    };

    typedef multi_index<"libraries"_n, libraries > library_t;

    ACTION addlibrary(uint64_t library_id, name owner);

    ACTION addbook(uint64_t library_id, 
                        uint64_t book_id,
                            string title,
                                string author,
                                    string isbn,
                                        string location,
                                            asset borrow_price,
                                                int quantity);

    ACTION borrowbook(uint64_t library_id, uint64_t book_id, name owner, asset payment);
    ACTION returnbook(uint64_t library_id, uint64_t book_id, name owner );

    private :

    library_t _library;

};
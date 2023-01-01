#include "library.hpp"

using namespace inery;
using namespace std;

library::library(name receiver, name code, datastream<const char*> ds)
    : contract(receiver, code, ds),
    _library(get_self(), get_self().value) {}

void library::addlibrary(uint64_t library_id, name owner){
    require_auth(get_self());

    auto library_itr = _library.find(library_id);
    check(library_itr != _library.end(), "Library already exist");

   _library.emplace(get_self(), [&](auto& l){
        l.id = library_id;
        l.owner = owner;
    });
}

void library::addbook(uint64_t library_id, uint64_t book_id,string title, string author, string isbn, string location, asset borrow_price, int quantity){
    require_auth(get_self());

    auto library_itr = _library.find(library_id);
    check(library_itr == _library.end(), "Library does not exist");

    book book={book_id,title, author,isbn,  location, borrow_price, quantity };
    _library.modify(library_itr, same_payer, [&](auto& l){
        l.items.push_back(book);
    });
}


void library::borrowbook(uint64_t library_id, uint64_t book_id, name owner, asset payment){
    require_auth(get_self());

    auto library_itr = _library.find(library_id);
    check(library_itr != _library.end(), "Library does not exist");

    // Check if book exists and is available for borrowing
    bool book_exists = false;
    const book* book_ptr = nullptr;
    for (auto& book : library_itr->items) {
        if (book.book_id == book_id && book.quantity > 0) {
            book_exists = true;
            book_ptr = &book;
            break;
        }
    }
    check(book_exists, "Book does not exist or is not available for borrowing");

    // Check if patron has reached maximum number of borrowed books
    bool patron_exists = false;
    Patron* patron_ptr = nullptr;
    for (auto& patron : library_itr->patrons) {
        if (patron.name == owner) {
            patron_exists = true;
            patron_ptr = &patron;
            if (patron.borrowedItems.size() >= MAX_BORROWED_BOOKS) {
                check(false, "Patron has reached maximum number of borrowed books");
            }
            break;
        }
    }
    check(patron_exists, "Patron does not exist");

    // Check if payment is sufficient
    check(payment >= book_ptr->borrow_price, "Payment is insufficient");

    // Deduct payment from patron's account
    action(
        permission_level{owner, "active"_n},
        "inery.token"_n,
        "transfer"_n,
        std::make_tuple(owner, get_self(), payment, std::string("Borrowing book"))
    ).send();

    // Decrement quantity of book and add to patron's borrowed items
    _library.modify(library_itr, same_payer, [&](auto& l){
        book_ptr->quantity--;
        patron_ptr->borrowedItems.emplace_back(*book_ptr, current_time_point());
    });
}

void library::returnbook(uint64_t library_id, uint64_t book_id, name owner ){
    require_auth(get_self());

    auto library_itr = _library.find(library_id);
    check(library_itr == _library.end(), "Library does not exist");
}
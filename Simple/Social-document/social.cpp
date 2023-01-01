#include <inery/inery.hpp>
#include <inery/time.hpp>
#include <inery/system.hpp>
#include <inery/print.hpp>
#include <inery/crypto.hpp>

#include <string>
#include <vector>

using namespace inery;
using namespace std;

using inery::current_time_point;
using inery::indexed_by;
using inery::time_point_sec;


class [[inery::contract("social")]] social : public contract {
   public:
      using contract::contract;

      social(name receiver, name code, datastream<const char*> ds) : contract(receiver, code, ds),
       _users(get_self(), get_self().value) {}

    ACTION message(string& name){

        checksum256 hash = sha256(name.c_str(), name.size());;

        auto bytes = hash.extract_as_byte_array();

        uint64_t value;
        std::memcpy(&value, bytes.data(), sizeof(value));

        print("checksum : ", hash);
        print(" hashed checksum : ", value);
    }

    uint64_t string_to_uint64(string s){
        checksum256 hash = sha256(s.c_str(), s.size());;   
        auto bytes = hash.extract_as_byte_array();
        uint64_t value;
        std::memcpy(&value, bytes.data(), sizeof(value));    
        return value;     
    }


    ACTION adduser(name owner, string username, string email){
        
        require_auth(owner);

        uint64_t id = string_to_uint64(username); 
        auto itr = _users.find(id);
        check(itr == _users.end(), "User with that username already exists");

        _users.emplace(itr, [&](auto& u){
            u.id = id;
            u.username = username;
            u.email = email;
        });
            


    }

    private: 

    struct education_institute {
        uint64_t    id;
        string      name;
        Location    Location;
    }

    struct Location {
        string State;
        string City;
        string Street;
    }

    struct Personal_Info {
        string first_name;
        string last_name;
        int age;
        Location location;
        vector<education_institute>education;
    }

    TABLE user {
        uint64_t  id;
        string username;
        string email;

        Personal_Info info;

        vector<string> following;
        vector<string> followers;

        vector<Post> posts;

        uint64_t primary_key() const { return id; }
    };

    typedef multi_index<"user"_n, user > user_t;

    user_t  _users;

    struct Post {
        uint64_t id;
        string author;
        string content;
        string time;
        vector<Comment> comments;
        vector<string> likes;
    };

    struct Comment {
        string id;
        string author;
        string content;
        string time;
    };
    
    struct Group {
        string id;
        string name;
        string description;
        vector<string> members;
        vector<string> posts;
    };

    struct Event {
        string id;
        string name;
        string description;
        string location;
        string time;
        string host;
        vector<string> attendees;
    };

};
#include <inery/inery.hpp>
#include <inery/print.hpp>

using namespace inery;

CONTRACT musicstore : public contract {
  public:
    using contract::contract;

    ACTION addalbum(uint64_t id, std::string title, std::string artist, std::string label, std::string genre, std::string release_date) {
      require_auth(_self);

      album_index albums(_self, _self.value);
      albums.emplace(_self, [&](auto& album) {
        album.id = id;
        album.title = title;
        album.artist = artist;
        album.label = label;
        album.genre = genre;
        album.release_date = release_date;
      });
    }

    ACTION addsong(uint64_t id, std::string title, std::string artist, uint64_t album_id, uint32_t duration) {
      require_auth(_self);

      song_index songs(_self, _self.value);
      songs.emplace(_self, [&](auto& song) {
        song.id = id;
        song.title = title;
        song.artist = artist;
        song.album_id = album_id;
        song.duration = duration;
      });
    }

    ACTION adduser(uint64_t id, std::string name, std::string email) {
      require_auth(_self);

      user_index users(_self, _self.value);
      users.emplace(_self, [&](auto& user) {
        user.id = id;
        user.name = name;
        user.email = email;
      });
    }

    ACTION addorder(uint64_t id, uint64_t user_id, std::vector<uint64_t> song_ids) {
      require_auth(_self);

      order_index orders(_self, _self.value);
      orders.emplace(_self, [&](auto& order) {
        order.id = id;
        order.user_id = user_id;
        order.song_ids = song_ids;
      });
    }

  private:

    TABLE Album {
        uint64_t id;
        std::string title;
        std::string artist;
        std::string label;
        std::string genre;
        std::string release_date;

        uint64_t primary_key() const { return id; }
    };

    TABLE Song {
        uint64_t id;
        std::string title;
        std::string artist;
        uint64_t album_id;
        uint32_t duration;

        uint64_t primary_key() const { return id; }
    };

    TABLE User {
        uint64_t id;
        name        owner;
        uint64_t    balance;
        std::string name;
        std::string email;

        uint64_t primary_key() const { return id; }
    };

    TABLE Order {
        uint64_t id;
        uint64_t user_id;
        std::vector<uint64_t> song_ids;

        uint64_t primary_key() const { return id; }
    };
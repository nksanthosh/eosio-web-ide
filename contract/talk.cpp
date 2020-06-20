#include <eosio/eosio.hpp>

// Message table
struct [[eosio::table("message"), eosio::contract("talk")]] message {
    uint64_t    id       = {}; // Non-0
    uint64_t    reply_to = {}; // Non-0 if this is a reply
    eosio::name user     = {};
    std::string content  = {};
    uint64_t    postlikes    = {};

    uint64_t primary_key() const { return id; }
    uint64_t get_reply_to() const { return reply_to; }
};

using message_table = eosio::multi_index<
    "message"_n, message, eosio::indexed_by<"by.reply.to"_n, eosio::const_mem_fun<message, uint64_t, &message::get_reply_to>>>;

// The contract
class talk : eosio::contract {
  public:
    // Use contract's constructor
    using contract::contract;

    // Post a message
    [[eosio::action]] void post(uint64_t id, uint64_t reply_to, eosio::name user, const std::string& content) {
        message_table table{get_self(), 0};

        // Check user
        require_auth(user);

        // Check reply_to exists
        if (reply_to)
            table.get(reply_to);

        // Create an ID if user didn't specify one
        eosio::check(id < 1'000'000'000ull, "user-specified id is too big");
        if (!id)
            id = std::max(table.available_primary_key(), 1'000'000'000ull);

        // Record the message
        table.emplace(get_self(), [&](auto& message) {
            message.id       = id;
            message.reply_to = reply_to;
            message.user     = user;
            message.content  = content;
        });
    }

 // Like a message and increment the count of likes
    [[eosio::action]] void like(uint64_t id, eosio::name user) {
        message_table table{get_self(), 0};

        // Check user
        require_auth(user);

        //find the row with the post
        auto itr = table.find(id);

       // if the iterator is not at the end we found the post, so we update the likes count 
        if (itr != table.end()) {
            auto postlikes = itr->postlikes;
            postlikes++;

            // Update the message with the updated count of likes
            table.modify(itr, get_self(), [&](auto& message) {
                message.postlikes = postlikes;
            });
        }
    }

    // Similar to the above likes action one can also implement an unlike action

};

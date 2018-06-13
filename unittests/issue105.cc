//
// Test case for the issue #105 of original Christoph Rupp ubscale db
// This is the issue #8 in billon-pl/upscale repository
//


#include <iostream>
#include <ups/upscaledb.hpp>
#include <cstdlib>


int main()
{
    ups_env_t* env;
    ups_env_create(&env, "test.db", UPS_ENABLE_TRANSACTIONS, 0664, 0);
    //ups_env_create(&env, "test.db", 0, 0664, 0);

    ups_parameter_t params[] = {
    {UPS_PARAM_KEY_TYPE, UPS_TYPE_UINT32},
    {0, }
    };

    ups_db_t* db;
    ups_env_create_db(env, &db, 1, 0, &params[0]);

    const unsigned int item_count = 50;

    for (unsigned int i = 0; i < item_count; i++)
    {
        ups_key_t key = ups_make_key(&i, sizeof(i));
        ups_record_t record = {0};

        ups_db_insert(db, 0, &key, &record, 0);
    }

    for(unsigned int i = 0; i < item_count / 2; i++)
    {
        ups_key_t key = ups_make_key(&i, sizeof(i));

        ups_db_erase(db, 0, &key, 0);
    }

    uint64_t count = 0;
    ups_db_count(db,0, 0, &count);

    if(count != item_count / 2){
        std::cout << "Item count after delete: " << count << std::endl;
    }

    for(unsigned int i = 0; i < item_count / 2; i++)
    {
        ups_key_t key = ups_make_key(&i, sizeof(i));
        ups_record_t record = {0};

        ups_cursor_t* cursor;
        ups_cursor_create(&cursor, db, 0, 0);
        ups_status_t st = ups_cursor_find(cursor, &key, &record, UPS_FIND_GEQ_MATCH);
        //ups_status_t st = ups_db_find(db, 0, &key, &record, UPS_FIND_GEQ_MATCH);

        if(st == UPS_SUCCESS && *reinterpret_cast<int*>(key.data) == i){
            std::cout << "Found deleted item: " << i << std::endl;
        }

        ups_cursor_close(cursor);
    }

    ups_db_close(db, 0);

    return 0;
}

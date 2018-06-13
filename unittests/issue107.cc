//
// Test case for the issue #107 of original Christoph Rupp ubscale db
// This is the issue #4 in billon-pl/upscale repository
//


#include <iostream>
#include <ups/upscaledb.hpp>


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

    const int item_count = 57; // required

    for (int i = 0; i < item_count; i++)
    {
        ups_key_t key = ups_make_key(&i, sizeof(i));
        ups_record_t record = {0};

        ups_db_insert(db, 0, &key, &record, 0);
    }

    ups_cursor_t* cur;

    ups_cursor_create(&cur, db, 0, 0);

    int key_data = 0;
    ups_key_t key = {0}; key.data = &key_data; key.size = sizeof(key_data);
    ups_record_t rec = {0};

    ups_cursor_move(cur, &key, &rec, UPS_CURSOR_FIRST);
    if((UPS_SUCCESS == ups_cursor_move(cur, &key, &rec, UPS_CURSOR_PREVIOUS)))
    {
        std::cout << "Cursor moved from FIRST to PREVIOUS without UPS_KEY_NOT_FOUND" << std::endl;
    }

    ups_cursor_close(cur);

    ups_db_close(db, 0);

    return 0;
}


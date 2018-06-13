//
// Test case for the issue #107 of original Christoph Rupp ubscale db
// This is the issue #4 in billon-pl/upscale repository
//


#include <iostream>
#include <ups/upscaledb.hpp>
#include <cstdlib>

#define EXPECT_TRUE( st, txt ) \
if( !st ) \
{ \
	std::cout << txt << std::endl; \
	exit(1); \
}


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

    const unsigned int item_count = 57; // required

    for (unsigned int i = 0; i < item_count; i++)
    {
        ups_key_t key = ups_make_key(&i, sizeof(i));
        ups_record_t record = {0};

        ups_db_insert(db, 0, &key, &record, 0);
    }

    ups_cursor_t* cur;
    ups_status_t st;

    ups_cursor_create(&cur, db, 0, 0);

    int key_data = 0;
    ups_key_t key = {0}; key.data = &key_data; key.size = sizeof(key_data);
    ups_record_t rec = {0};

    st = ups_cursor_move(cur, &key, &rec, UPS_CURSOR_FIRST);
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_cursor_move, UPS_CURSOR_FIRST" );
    
    st = ups_cursor_move(cur, &key, &rec, UPS_CURSOR_PREVIOUS);
    EXPECT_TRUE( st != UPS_SUCCESS, "Cursor moved from FIRST to PREVIOUS without UPS_KEY_NOT_FOUND" );

    ups_cursor_close(cur);

    ups_db_close(db, 0);

    return 0;
}


//
// Test case for the issue #105 of original Christoph Rupp ubscale db
// This is the issue #8 in billon-pl/upscale repository
//


#include <iostream>
#include <ups/upscaledb.hpp>
#include <cstdlib>


#define EXPECT_TRUE( st, txt )     \
if( !( st ) )                      \
{                                  \
    std::cout << txt << std::endl; \
    exit( 1 );                     \
}


int main()
{
    ups_env_t* env;
    ups_status_t st = ups_env_create(&env, "test.db", UPS_ENABLE_TRANSACTIONS, 0664, 0);
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_env_create" );

    //ups_env_create(&env, "test.db", 0, 0664, 0);

    ups_parameter_t params[] = {
    {UPS_PARAM_KEY_TYPE, UPS_TYPE_UINT32},
    {0, }
    };

    ups_db_t* db;
    st = ups_env_create_db(env, &db, 1, 0, &params[0]);
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_env_create_db" );

    ups_txn_t* txn = nullptr;
    st = ups_txn_begin( &txn, env, nullptr, nullptr, 0 );
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_txn_begin" );

    const uint32_t item_count = 50;

    for (uint32_t i = 0; i < item_count; i++)
    {
        ups_key_t key = ups_make_key(&i, sizeof(i));
        ups_record_t record = {0};

        st = ups_db_insert(db, txn, &key, &record, 0);
        EXPECT_TRUE( st == UPS_SUCCESS, "ups_db_insert" );
    }

    uint64_t count = 0;
    st = ups_db_count( db, txn, 0, &count );
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_db_count" );
    EXPECT_TRUE( count == item_count, "ups_db_count returns wrong value after insert" );

    for(uint32_t i = 0; i < item_count / 2; i++)
    {
        ups_key_t key = ups_make_key(&i, sizeof(i));

        st = ups_db_erase(db, txn, &key, 0);
        EXPECT_TRUE( st == UPS_SUCCESS, "ups_db_erase" );
    }
    if( txn )
    {
        ups_txn_commit( txn, 0 );
    }

    //
    //    The function ups_db_count after delete does not work!!!
    //
    //    count = 0;
    //    st = ups_db_count(db, txn, 0, &count);
    //    EXPECT_TRUE( st == UPS_SUCCESS, "ups_db_count" );
    //    EXPECT_TRUE( count == item_count / 2, "ups_db_count returns wrong value after delete" );

    for(uint32_t i = 0; i < item_count / 2; i++)
    {
        ups_key_t key = ups_make_key(&i, sizeof(i));
        ups_record_t record = {0};

        ups_cursor_t* cursor;
        st = ups_cursor_create(&cursor, db, txn, 0);
        EXPECT_TRUE( st == UPS_SUCCESS, "ups_cursor_create" );

        st = ups_cursor_find(cursor, &key, &record, UPS_FIND_GEQ_MATCH);
        //ups_status_t st = ups_db_find(db, 0, &key, &record, UPS_FIND_GEQ_MATCH);

        if(st == UPS_SUCCESS && *reinterpret_cast< uint32_t* >(key.data) == i){
            std::cout << "Found deleted item: " << i << std::endl;
        }

        st = ups_cursor_close(cursor);
        EXPECT_TRUE( st == UPS_SUCCESS, "ups_cursor_close" );
    }

    if( txn )
    {
        ups_txn_commit( txn, 0 );
    }

    st = ups_db_close(db, 0);
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_db_close" );

    return 0;
}

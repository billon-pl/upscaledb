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

ups_db_t* create_env( ups_env_t** env, bool enable_transactions );
void fill_db( ups_db_t* db, unsigned int item_count );
void check_cursor( ups_db_t* db );
void close_env( ups_env_t *env, ups_db_t* db );


int main()
{
    const unsigned int item_count = 57; // required
    for( unsigned int cnt = 1; cnt <= item_count; cnt++ )
    {

        std::cout << "cnt = " << cnt << std::endl;

        ups_env_t* env = nullptr;
        ups_db_t* db = create_env( &env, true );
        fill_db( db, cnt );
        check_cursor( db );
        close_env( env, db );
    }

    return 0;
}

//
//
//
ups_db_t* create_env( ups_env_t **env, bool enable_transactions )
{
    const std::string db_name( "test.db" );
    const uint32_t mode = 0664;
    ups_status_t st = UPS_INV_PARAMETER;

    if( enable_transactions )
    {
        st = ups_env_create( env, db_name.c_str(), UPS_ENABLE_TRANSACTIONS, mode, 0);
        EXPECT_TRUE( st == UPS_SUCCESS, "ups_env_create, UPS_ENABLE_TRANSACTIONS" );
    }
    else
    {
        st = ups_env_create( env, db_name.c_str(), 0, mode, 0);
        EXPECT_TRUE( st == UPS_SUCCESS, "ups_env_create, UPS_DISABLE_TRANSACTIONS" );
    }

    ups_parameter_t params[] =
    {
        {UPS_PARAM_KEY_TYPE, UPS_TYPE_UINT32},
        {0, }
    };

    ups_db_t* db = nullptr;
    st = ups_env_create_db( *env, &db, 1, 0, &params[0]);
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_env_create_db" );

    return db;

}

//
//
//
void fill_db(ups_db_t* db, unsigned int item_count)
{
    for (unsigned int i = 0; i < item_count; i++)
    {
        ups_key_t key = ups_make_key(&i, sizeof(i));

        int record_value = 98;
        ups_record_t record =  ups_make_record( &record_value, sizeof( record_value ) );

        const ups_status_t st = ups_db_insert(db, 0, &key, &record, 0);
        EXPECT_TRUE( st == UPS_SUCCESS, "ups_db_insert" );
    }

}

//
//
//
void check_cursor(ups_db_t* db)
{
    ups_status_t st = UPS_INV_PARAMETER;
    ups_cursor_t* cur = nullptr;

    st = ups_cursor_create( &cur, db, 0, 0 );
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_cursor_create" );

    unsigned int query = 0;
    ups_key_t key = ups_make_key( &query, sizeof( query ) );
    ups_record_t rec = {0};

    st = ups_cursor_find( cur, &key, &rec, UPS_FIND_GEQ_MATCH );
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_cursor_find" );

    EXPECT_TRUE( rec.data, "rec.data is nullptr" );
    int *p = reinterpret_cast<int*>(rec.data);
    std::cout << (*p) << std::endl;

    // st = ups_cursor_move( cur, &key, &rec, UPS_CURSOR_FIRST );
    // EXPECT_TRUE( st == UPS_SUCCESS, "ups_cursor_move, UPS_CURSOR_FIRST" );

    st = ups_cursor_move( cur, &key, &rec, UPS_CURSOR_PREVIOUS );
    EXPECT_TRUE( st != UPS_SUCCESS, "Cursor moved from FIRST to PREVIOUS without UPS_KEY_NOT_FOUND" );

    st = ups_cursor_close( cur );
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_cursor_close" );
}

//
//
//
void close_env(ups_env_t* env, ups_db_t* db)
{
    ups_status_t st = ups_db_close(db, 0);
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_db_close" );

    st = ups_env_close( env, UPS_AUTO_CLEANUP );
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_env_close" );
}

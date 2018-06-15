//
// Test case for the issue #106 of original Christoph Rupp ubscale db
// This is the issue #6 in billon-pl/upscale repository
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
    ups_status_t st = ups_env_create( &env, "test.db", UPS_ENABLE_TRANSACTIONS, 0664, 0 );
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_env_create" );

    ups_parameter_t params[] =
    {
        { UPS_PARAM_KEY_TYPE, UPS_TYPE_UINT32 },
        { 0, }
    };

    ups_db_t* db;
    st = ups_env_create_db( env, &db, 1, 0, &params[ 0 ] );
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_env_create" );

    const unsigned int item_count = 57;

    for( unsigned  int i = 0; i < item_count; i++ )
    {
        ups_key_t key = ups_make_key( &i, sizeof( i ) );
        ups_record_t record = ups_make_record( nullptr, 0 );

        st = ups_db_insert( db, 0, &key, &record, 0 );
        EXPECT_TRUE( st == UPS_SUCCESS, "ups_db_insert" );
    }

    ups_cursor_t* cur;
    st = ups_cursor_create( &cur, db, 0, 0 );
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_cursor_create" );

    unsigned int key_data = 0;
    ups_key_t key = ups_make_key( &key_data, sizeof( key_data ) );
    ups_record_t rec = ups_make_record( nullptr, 0 );

    st = ups_cursor_find( cur, &key, &rec, UPS_FIND_GEQ_MATCH );
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_cursor_find" );

    unsigned int counter = 0;
    do
    {
        //
        // This statement causes the problem
        //
        // ups_cursor_move( cur, &key, &rec, UPS_CURSOR_FIRST );

        EXPECT_TRUE( counter == *reinterpret_cast< unsigned int* >( key.data ), "Invalid data" );
        counter++;
    }
    while( UPS_SUCCESS == ups_cursor_move( cur, &key, &rec, UPS_CURSOR_NEXT ) );
    EXPECT_TRUE( counter == item_count, "Cursor does not contain all data" );

    st = ups_cursor_close( cur );
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_cursor_close" );

    st = ups_db_close( db, 0 );
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_db_close" );

    return 0;
}

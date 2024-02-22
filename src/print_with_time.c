#include "../headers/timer.h"
#include <stdio.h>
#include <time.h>


static time_t t;

void printnl_with_time( char *text )
{

    t = time( NULL );
    if( t != (time_t)-1 )
    {

        struct tm time_s = *localtime( &t );
        printf( "%d:%d:%d ", time_s.tm_hour, time_s.tm_min, time_s.tm_sec );
        printf( "%s\n",text );

    }
    else
    {

        printf( "PRINTING_WITH_TIME_ERROR!!!\n");

    }

}


void print_with_time( char *text )
{

    t = time( NULL );
    if( t != (time_t)-1 )
    {

        struct tm time_s = *localtime( &t );
        printf( "%d:%d:%d ", time_s.tm_hour, time_s.tm_min, time_s.tm_sec );
        printf( "%s",text );

    }
    else
    {

        printf( "PRINTING_WITH_TIME_ERROR!!!\n");

    }

}
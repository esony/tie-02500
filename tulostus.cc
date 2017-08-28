#include "tulostus.hh"

namespace tulostus
{
  namespace {
    // sisäiseen käyttöön mutex, jota kaikki lukitusoliot käyttävät
    pthread_mutex_t suojaus = PTHREAD_MUTEX_INITIALIZER;
  }

  // kun lukko-olio luodaan, niin se varaa mutex:n
  lukko::lukko()
  {
    if( pthread_mutex_lock( &suojaus ) != 0 )
      throw LukkoError( "lukko, mutex_lock failed" );
  }

  // lukko-olion tuhoutuminen vapauttaa mutex:n
  lukko::~lukko()
  {
    if( pthread_mutex_unlock( &suojaus ) != 0 )
      throw LukkoError( "lukko, mutex unlock failed" );
  }
}


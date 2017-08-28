// 218100 Finta Albert
// 218605 Eriksson Mikael

#include <iostream>
#include <vector>
#include <pthread.h>
#include <time.h>
#include <fstream>

// kurssikirjasto:
#include "rinn2015.h"
#include "client.h"
// c++11 kirjastoja:
#include <memory>
#include <cstdint>
#include <string>
#include <sstream>
#include <chrono>

pthread_mutex_t count_mutex     = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t tallenna_mutex     = PTHREAD_MUTEX_INITIALIZER;


// Saikeen koodi
// Funktio hakee palvelimelta uuden yhteyden, 
// rakentaa vastaustiedot ja lahettaa vastauksen palvelimelle
void* suorita(void* arg)
{
	// 2. Haetaan yksi http-yhteys käsittelyyn
	
	// get_connection() ei ole kirjaston puolesta saieturvallinen, joten
	// lukitaan toiminto, ettei palvelimelta kysella yhteyksia paallekkain
	pthread_mutex_lock( &count_mutex );
	std::unique_ptr<rinn2015::Client> client( rinn2015::get_connection() );
	pthread_mutex_unlock( &count_mutex );
		
	
	// 3. Rakennetaan vastaustiedot
	// Ei saieturvallisuuden kannalta mitaan oleellista
	
	// http Status Code 200 = "OK"
	client->reply_status = "200";
	  
	std::stringstream vastaus;
    vastaus << "<html> <body>"
			<< "<h1> Hello World! </h1>"
			<< "</body> </html>";

    client->reply_data = vastaus.str();

    // 4. lähetetään vastaus kirjaston avulla:
	
	// send_reply on kirjaston puolesta saieturvallinen, 
	// rinnakkaisten saikeiden suoritusvaiheista ei tarvitse murehtia
	uint32_t id = rinn2015::send_reply( std::move(client) );
	  
	// Palautetaan yhteyden id
    return reinterpret_cast<void*>( id );
}

//Tiedostoon tulostamisen toteutus omana saikeenaan

//Pros: Uudet saikeet voi lahtea hakemaan uusia yhteyksia palvelimelta,
//		eika tallennuksesta synny mainittavaa viivetta.
//
//Cons: Se ei toimi

/*
void* tallenna(void* arg){

		auto saikeet = reinterpret_cast<std::vector<pthread_t> >( arg );
		pthread_mutex_lock( &tallenna_mutex );

		for( auto& thr : saikeet ){
			void* retval = 0;
			auto status = pthread_join( thr, &retval  );
			
			if( status != 0 ) {
				perror( "pthread_join" );
				exit(2);
			}
			  
			  // 5. haetaan ja tulostetaan yhteyden tiedot:
			  auto id = reinterpret_cast<long int>( retval );
			  
			  std::cout << "Palveltiin yhteys: " << rinn2015::peer_info( id )
						<< std::endl;

		}
		
			  auto stop = std::chrono::high_resolution_clock::now();
			  auto diff = stop - start;
			  std::clog.precision( 12 );
			  std::clog << "Duration: " << std::fixed
						<< std::chrono::duration<double, std::milli>(diff).count() 
						<< " ms" << std::endl;
    		
			pthread_mutex_unlock( &tallenna_mutex );
			
}
*/

int main(){
  try {
    // 1. alustetaan kirjasto omalla opiskelijanumerolla
    rinn2015::init_server( 218605 );

    while( true ) {
	
		// Vektori saikeille
		std::vector<pthread_t> saikeet;
		
		// "Kello kayntiin"
		auto start = std::chrono::high_resolution_clock::now();
		
		// 100 saietta kayntiin ja tunniste talteen vektoriin	
		for( auto i = 1; i <= 100; i++ ){
			
			auto num = reinterpret_cast<void*>( i );
			
			// Luodaan uusi pthread_t-tyyppinen saie
			pthread_t saie;
			
			// pthread_create kaynnistaa saikeessa "saie", 
			// funktion "suorita", funktion argumentilla "num"
			
			auto status = pthread_create( &saie, NULL, &suorita, num );
			
			if( status != 0 ) {
				perror( "pthread_create" );
				exit(1);
			}
			
			// Luodun saikeen tunnus vektorin peralle
			saikeet.push_back( saie );
		}

		// Tallennetaan suoritetut saikeet lokitiedostoon
		std::ofstream tulostus("HTTP.LOKI", std::ios_base::app);
		for( auto& thr : saikeet ){
			void* retval = 0;
			auto status = pthread_join( thr, &retval  );
			
			if( status != 0 ) {
				perror( "pthread_join" );
				exit(2);
			}
			  
			// 5. haetaan, tulostetaan ja tallennetaan yhteyden tiedot:
			auto id = reinterpret_cast<long int>( retval );
			  
			// Tulostetaan naytolle
			std::string tiedot = rinn2015::peer_info( id );
			std::cout << "Palveltiin yhteys: " 
					  << /*rinn2015::peer_info( id )*/ tiedot
					  << std::endl;
			
			// Tulostetaan sama lokitiedostoon
			tulostus << "Palveltiin yhteys: " 
					 << /*rinn2015::peer_info( id )*/ tiedot
					 << std::endl;
		}
		// Suljetaan stream
		tulostus.close();
		
		// Tulostetaan naytolle viimeisen 100 yhteyteen kulunut aika
		auto stop = std::chrono::high_resolution_clock::now();
		auto diff = stop - start;
		std::cout << std::endl;
		std::clog.precision( 12 );
		std::clog << "100 connections, duration: " << std::fixed
				  << std::chrono::duration<double, std::milli>(diff).count() 
				  << " ms" << std::endl;  
		std::cout << std::endl;
 
	
	/*
	// Tiedostoon tallentamisen toteutus omana saikeenaan
	
		auto loki_ptr = static_cast<void*>( &saikeet );
		//auto loki_ptr = reinterpret_cast<void*>( &saikeet );

		pthread_t tallennus_saie;
		auto status = pthread_create( &tallennus_saie, NULL, &tallenna, loki_ptr );
		
		if( status != 0 ) {
			perror( "pthread_create" );
			exit(1);
		}
	*/
	}

  } catch( std::exception &e ) {
    std::cerr << "exception in main(): " << e.what() << std::endl;
  }
}

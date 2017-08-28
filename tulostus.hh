#ifndef TULOSTUS_HH
#define TULOSTUS_HH
#include <pthread.h>
#include <exception>
#include <string>

namespace tulostus
{
  // virhepoikkeus
  class LukkoError : public std::exception {
  public:
    LukkoError(std::string message) : msg_(message) {};
    ~LukkoError() throw() {};
    const char* what() const throw() { return msg_.c_str(); }
  private:
    std::string msg_;
  };

  // tulostusten sarjallistamista varten lukko
  // olion luonti tekee lukituksen ja olion tuhoaminen vapauttaa
  class lukko {
  public:
    lukko();
    ~lukko();
  };
}

#endif
#include "CDI.hpp"

CDI::CDI() : disk()
{

}

CDI::~CDI()
{
    disk.Close();
}
